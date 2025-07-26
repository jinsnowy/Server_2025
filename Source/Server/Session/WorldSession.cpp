#include "stdafx.h"
#include "WorldSession.h"
#include "Core/Sql/Database.h"
#include "Service/WorldServiceDef.h"
#include "Protobuf/Public/World.h"

#include "Server/Database/DB.h"
#include "Server/Model/Player.h"
#include "Server/Model/Movable.h"
#include "Server/Model/Combat.h"
#include "Server/InterServer/LobbyGrpcService.h"
#include "Server/InterServer/LobbyGrpcClient.h"

#include "Server/Service/WorldService.h"
#include "Server/World/PlayerMovableTick.h"
#include "Server/Authenticator/Authenticator.h"
#include "Server/World/PlayerRepository.h"
#include "Server/Section/Section.h"
#include "Server/Section/SectionRepository.h"

namespace Server {
	WorldSession::WorldSession()
	{
	}

	WorldSession::~WorldSession() {
	}

	std::unique_ptr<Network::Protocol> WorldSession::CreateProtocol() {
		return std::make_unique<WorldProtocol>();
	}

	void WorldSession::OnConnected() {
		ProtobufSession::OnConnected();

		LOG_INFO("WorldSession::OnConnected session_id:{}, address:{}", session_id(), GetConnectionString());
	}

	void WorldSession::OnDisconnected() {
		ProtobufSession::OnDisconnected();

		LOG_INFO("WorldSession::OnDisconnected session_id:{}, address:{}", session_id(), GetConnectionString());

		auto this_session = System::Actor::GetShared(this);
		PlayerMovableTick::EndTick(this_session);

		if (section_ != nullptr) {
			SectionRepository::LeaveSection(section_->map_uid(), this_session);
		}

		
	}

	void WorldSession::OnMovableTick(const System::Tick& ) {
		if (player() == nullptr) {
			return;
		}

		if (section_ == nullptr) {
			return;
		}

		auto move_notify = std::make_shared<world::OtherClientMoveNotify>();
		move_notify->set_character_id(character_id());
		player()->movable().WriteTo(move_notify->mutable_character_pos());
		section()->Multicast(move_notify, session_id());
	}

	void WorldSession::set_player(std::unique_ptr<Model::Player> player) {
		player_ = std::move(player);
	}

	void WorldSession::OnSectionEntered(std::shared_ptr<Section> section) {
		DEBUG_ASSERT(IsSynchronized());

		if (section == nullptr) {
			return;
		}

		section_ = section;

		world::ClientEnterMapNotify enter_notify;
		section->WriteTo(enter_notify.mutable_section_info());
		Send(enter_notify);

		auto notify = std::make_shared<world::OtherClientEnterNotify>();
		notify->set_character_id(character_id());
		section->WriteTo(notify->mutable_section_info());
		player()->movable().WriteTo(notify->mutable_character_pos());
		section->Multicast(notify, session_id());

		section->ForEach([entered_session = GetShared(this), section](WorldSession& session) {
			if (session.player() == nullptr) {
				return;
			}

			auto notify = world::OtherClientEnterNotify();
			notify.set_character_id(session.character_id());
			session.player()->movable().WriteTo(notify.mutable_character_pos());
			section->WriteTo(notify.mutable_section_info());
		
			entered_session->Send(notify);
		}, session_id());
	}

	void WorldSession::OnSectionLeft(std::shared_ptr<Section> section) {
		DEBUG_ASSERT(IsSynchronized());

		if (section_ != section) {
			LOG_ERROR("WorldSession::OnSectionLeft section mismatch: expected {}, got {}", 
				section_ ? section->section_id() : 0, section->section_id());
			return;
		}

		section_.reset();

		world::ClientLeaveMapNotify leave_notify;
		section->WriteTo(leave_notify.mutable_section_info());
		Send(leave_notify);

		auto notify = std::make_shared<world::OtherClientLeaveNotify>();
		notify->set_character_id(character_id());
		section->WriteTo(notify->mutable_section_info());
		section->Multicast(notify, session_id());
	}

	void WorldSession::OnVerifyClientAction(const std::shared_ptr<const world::ClientActionReq>& msg, world::ClientActionRes& res)
	{
		if (player() == nullptr) {
			LOG_ERROR("WorldSession::OnClientAction player is null for session_id: {}", session_id());
			res.set_result(types::Result::kNotFound);
			return;
		}

		auto action_type = msg->client_action().ClientActionField_case();
		switch (action_type) {
		case types::ClientAction::kBaseAttackAction:
		{
			auto& combat = player()->combat();
			const auto tick = System::Tick::Current();

			float client_timestamp = msg->client_action().base_attack_action().client_timestamp();
			float required_cooldown_secs = (tick - combat.GetLastBaseAttackTick()).AsSecs();
			float diff_seconds = required_cooldown_secs - combat.GetBaseAttackCooldown();
			float expected_timestamp = client_timestamp + combat.GetBaseAttackCooldown() - std::min(diff_seconds, 0.f);
			res.mutable_client_action()->mutable_base_attack_action()->set_client_timestamp(expected_timestamp);

			if (diff_seconds < 0.f) {
				LOG_WARNING("BaseAttack cheat detected, diff seconds : {:4.3f}", diff_seconds);
				res.set_result(types::Result::kInvalidCooldown);
				return;
			}

			res.set_result(types::Result::kSuccess);
			combat.SetLastBaseAttackTick(tick);
		}break;
			default:
				res.set_result(types::Result::kSuccess);
				break;
		}
	}

	static void OnRegisterServerReq(WorldSession& session, const std::shared_ptr<const world::RegisterServerReq>& msg) {
		LOG_INFO("WorldSession::OnRegisterServerReq session_id:{}, address:{}, server_address:{}, server_type:{}",
			session.session_id(), session.GetConnectionString(), msg->server_address(), System::Enums::ToString(msg->server_type()));

		std::string server_address = msg->server_address();
		auto server_type = msg->server_type();
		std::string level_map_name = msg->level_map_name();

		auto agent = DB::GetLobbyDB().GetAgent();
		auto stmt = agent->CreateStmt();
		stmt.BindInParam(static_cast<uint8_t>(server_type));
		stmt.BindInParam(Sql::WCharArray(128, server_address.c_str()));
		stmt.BindInParam(Sql::WCharArray(128, level_map_name.c_str()));
		stmt.BindInParam(System::Time::UtcNow());
		if (stmt.Execute(L"usp_UpsertServer") == false) {
			auto res = std::make_shared<world::RegisterServerRes>();
			res->set_result(types::Result::kDatabaseError);
			res->set_server_id(0);
			session.Send(res);
			LOG_ERROR("Failed to register server: server_address: {}, server_type: {}, error: {}", 
				server_address, static_cast<int32_t>(server_type), stmt.GetLastErrorMessage());
			return;
		}

		stmt.Reset();
		stmt.BindInParam(Sql::WCharArray(128, server_address.c_str()));
		if (stmt.Execute(L"usp_SelectServer") == false) {
			auto res = std::make_shared<world::RegisterServerRes>();
			res->set_result(types::Result::kDatabaseError);
			res->set_server_id(0);
			session.Send(res);
			LOG_ERROR("Failed to select server: server_address: {}, error: {}", server_address, stmt.GetLastErrorMessage());
			return;
		}

		int32_t server_id = 0;
		stmt.BindColumn(&server_id);
		if (stmt.FetchResult() == false) {
			auto res = std::make_shared<world::RegisterServerRes>();
			res->set_result(types::Result::kDatabaseError);
			res->set_server_id(0);
			session.Send(res);
			LOG_ERROR("Failed to fetch server: server_address: {}, error: {}", server_address, stmt.GetLastErrorMessage());
			return;
		}

		auto res = std::make_shared<world::RegisterServerRes>();
		res->set_result(types::Result::kSuccess);
		res->set_server_id(server_id);
		session.Send(res);
	}

	static void OnRecvHelloWorldServer(WorldSession& session, const std::shared_ptr<const world::HelloWorldServer>& msg) {
		std::string user_id = msg->user_id();
		std::string access_token = msg->access_token();
		int32_t playing_server_id = msg->playing_server_id();
		int64_t playing_character_id = msg->playing_character_id();
		if (access_token.empty() || user_id.empty() || playing_server_id <= 0 || playing_character_id <= 0) {
			LOG_ERROR("Invalid HelloWorldServer message: user_id: {}, access_token: {}, playing_server_id: {}, playing_character_id: {}",
				user_id, access_token, playing_server_id, playing_character_id);
			session.Disconnect();
			return;
		}

		auto login_request = std::make_shared<lobby_service::CharacterLoginRequest>();
		login_request->set_user_id(user_id);
		login_request->set_access_token(access_token);
		login_request->set_playing_server_id(playing_server_id);
		login_request->set_playing_character_id(playing_character_id);

		WorldService& world_service = WorldSession::GetService();
		world_service.GetLobbyGrpcClient().CharacterLogin(login_request)
			.Then([playing_character_id, session = System::Actor::GetShared(&session)](GrpcCallResult<CharacterLoginResponse> result) mutable {
			if (result.ok() == false) {
				LOG_ERROR("Failed to login character: {}", result.status.error_message());
				session->Disconnect();
				return;
			}

			PlayerRepository::Pop(playing_character_id).Then([session](std::unique_ptr<Model::Player> player) {
				Ctrl(*session).Post([player = std::move(player)](WorldSession& session) mutable {
					if (player == nullptr) {
						player = std::make_unique<Model::Player>(session.character_id());
					}

					session.set_player(std::move(player));

					auto player_ptr = session.player();
					if (player_ptr->LoadFromDb() == false) {
						session.Disconnect();
						return;
					}

					auto this_session = System::Actor::GetShared(&session);
					PlayerMovableTick::BeginTick(this_session);

					world::HelloWorldClient hello_client;
					hello_client.set_map_uid(1);
					hello_client.set_server_tick_interval_ms(PlayerMovableTick::kPlayerMovableTickInterval);

					session.Send(hello_client);

					LOG_INFO("WorldSession::OnRecvHelloServer session_id:{}, address:{}, character_id:{}",
						session.session_id(), session.GetConnectionString(), session.character_id());
					});
				});
			});
	}
	
	static void OnRecvClientEnterMapReq(WorldSession& session, const std::shared_ptr<const world::ClientEnterMapReq>& msg) {
		int32_t map_uid = msg->map_uid();
		if (map_uid > 0 && session.player()) {
			session.player()->movable().ReadFrom(msg->character_pos());

			LOG_INFO("WorldSession::OnRecvClientEnterMapReq session_id:{}, address:{}, map_uid:{}",
				session.session_id(), session.GetConnectionString(), map_uid);

			auto res = std::make_shared<world::ClientEnterMapRes>();
			res->set_result(types::Result::kSuccess);
			res->set_map_uid(map_uid);
			
			session.Send(res);

			SectionRepository::EnterSection(map_uid, System::Actor::GetShared(&session));
		}
	}

	static void OnRecvClientMoveReq (WorldSession& session, const std::shared_ptr<const world::ClientMoveReq>& msg) {
		if (session.player() == nullptr) {
			LOG_ERROR("Player not found for session_id: {}", session.session_id());
			return;
		}

		if (session.section() == nullptr) {
			LOG_ERROR("Section not found for session_id: {}", session.session_id());
			return;
		}

		session.player()->movable().ReadFrom(msg->character_pos());
		
		world::ClientMoveRes res;
		res.set_client_timestamp(msg->client_timestamp());
		session.Send(res);
	}

	static void OnRecvClientActionReq (WorldSession& session, const std::shared_ptr<const world::ClientActionReq>& msg) {
		if (session.player() == nullptr) {
			LOG_ERROR("Player not found for session_id: {}", session.session_id());
			return;
		}
		if (session.section() == nullptr) {
			LOG_ERROR("Section not found for session_id: {}", session.session_id());
			return;
		}

		world::ClientActionRes res;
		session.OnVerifyClientAction(msg, res);
		res.set_client_timestamp(msg->client_timestamp());
		session.Send(res);

		if (res.result() != types::Result::kSuccess) {
			return;
		}

		auto notify = std::make_shared<world::OtherClientActionNotify>();
		notify->set_character_id(session.character_id());
		notify->mutable_client_action()->CopyFrom(msg->client_action());

		session.section()->Multicast(notify, session.session_id());
	}

	static void OnRecvChangeServerTickIntervalReq(WorldSession& session, const std::shared_ptr<const world::ChangeServerTickIntervalReq>& msg) {
		if (msg->server_tick_interval_ms() <= 0) {
			LOG_ERROR("Invalid server tick interval: {}", msg->server_tick_interval_ms());
			return;
		}

		PlayerMovableTick::SetServerTickInterVal(msg->server_tick_interval_ms());
		world::ChangeServerTickIntervalRes res;
		res.set_server_tick_interval_ms(msg->server_tick_interval_ms());
		session.Send(res);

		LOG_INFO("WorldSession::OnRecvChangeServerTickIntervalReq session_id:{}, address:{}, new_interval:{}",
			session.session_id(), session.GetConnectionString(), msg->server_tick_interval_ms());
	}

	void WorldSession::RegisterHandler(WorldHandlerMap* handler_map) {
		handler_map->Register(OnRegisterServerReq);
		handler_map->Register(OnRecvHelloWorldServer);
		handler_map->Register(OnRecvClientEnterMapReq);
		handler_map->Register(OnRecvClientMoveReq);
		handler_map->Register(OnRecvClientActionReq);
		handler_map->Register(OnRecvChangeServerTickIntervalReq);
	}

	WorldService& WorldSession::GetService() {
		return *System::DependencyInjection::Get<WorldService>();
	}
}

