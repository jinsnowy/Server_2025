#include "stdafx.h"
#include "WorldSession.h"
#include "Core/Sql/Database.h"
#include "Service/WorldServiceDef.h"
#include "Protobuf/Public/World.h"

#include "Server/Database/DB.h"
#include "Server/Model/Player.h"
#include "Server/Model/Movable.h"

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

	void WorldSession::OnMovableTick(const System::Tick& tick) {
		if (player() == nullptr) {
			return;
		}

		if (section_ == nullptr) {
			return;
		}

		auto move_notify = std::make_shared<world::OtherClientMoveNotify>();
		move_notify->set_character_id(character_id());
		player()->movable().WriteTo(move_notify->mutable_character_pos());
		*move_notify->mutable_server_timestamp() = Protobuf::ToTimestamp(tick);

		section_->Multicast(move_notify, session_id());
	}

	void WorldSession::set_player(std::unique_ptr<Model::Player> player)
	{
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
		int64_t character_id = msg->character_id();
		session.set_character_id(character_id);

		PlayerRepository::Pop(character_id).Then([session = System::Actor::GetShared(&session)](std::unique_ptr<Model::Player> player) {
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

				session.Send(hello_client);

				LOG_INFO("WorldSession::OnRecvHelloServer session_id:{}, address:{}, character_id:{}",
					session.session_id(), session.GetConnectionString(), session.character_id());
			});
		});
	}
	
	static void OnRecvClientEnterMapReq(WorldSession& session, const std::shared_ptr<const world::ClientEnterMapReq>& msg) {
		int32_t map_uid = msg->map_uid();
		if (map_uid > 0) {

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
		session.player()->movable().ReadFrom(msg->character_pos());
		
		world::ClientMoveRes res;
		session.Send(res);
	}

	void WorldSession::RegisterHandler(WorldHandlerMap* handler_map) {
		handler_map->Register(OnRegisterServerReq);
		handler_map->Register(OnRecvHelloWorldServer);
		handler_map->Register(OnRecvClientEnterMapReq);
		handler_map->Register(OnRecvClientMoveReq);
	}
}

