#include "stdafx.h"
#include "WorldSession.h"
#include "Core/Sql/Database.h"
#include "Service/WorldServiceDef.h"
#include "Protobuf/Public/World.h"

#include "Server/Database/DB.h"
#include "Server/Model/Player.h"
#include "Server/Model/Combat.h"
#include "Server/InterServer/LobbyGrpcService.h"
#include "Server/InterServer/LobbyGrpcClient.h"

#include "Server/Section/SpawnSystem.h"
#include "Server/Service/WorldService.h"
#include "Server/DataTable/SpawnerDataRecord.h"
#include "Server/World/PlayerTick.h"
#include "Server/Authenticator/Authenticator.h"
#include "Server/World/PlayerRepository.h"
#include "Server/Section/Section.h"
#include "Server/Section/SectionRepository.h"
#include "Server/Model/UniqueId.h"
#include "Server/GameObject/Projectile.h"
#include "Server/GameObject/Npc.h"
#include "Server/GameObject/Pc.h"
#include "Server/Utilites/Protobuf.h"


namespace Server {
	WorldSession::WorldSession()
		:
		player_(std::make_unique<Model::Player>()),
		server_id_(GetService().server_id()) 
	{
		pc_ = std::make_shared<Pc>();
	}

	WorldSession::~WorldSession() {
	}

	std::unique_ptr<Network::Protocol> WorldSession::CreateProtocol() {
		return std::make_unique<WorldProtocol>();
	}

	int64_t WorldSession::GenerateId() const {
		return UniqueId::Issue(GetService().server_id());
	}

	void WorldSession::OnConnected(const Network::IPAddress& address) {
		ProtobufSession::OnConnected(address);

		LOG_INFO("WorldSession::OnConnected session_id:{}, address:{}", session_id(), GetConnectionString());
	}

	void WorldSession::OnDisconnected() {
		ProtobufSession::OnDisconnected();

		LOG_INFO("WorldSession::OnDisconnected session_id:{}, address:{}", session_id(), GetConnectionString());

		auto this_session = SharedFrom(this);
		PlayerTick::EndTick(this_session);

		if (section_ != nullptr) {
			SectionRepository::LeaveSection(section_->map_uid(), this_session);
		}
	}

	void WorldSession::OnSectionTick(float) {
		DEBUG_ASSERT(IsSynchronized());

		const auto tick = System::Tick::Current();
		for (auto iter = action_queue_.begin(); iter != action_queue_.end();) {
			if (iter->expire_time <= tick) {
				iter = action_queue_.erase(iter);
			} else {
				++iter;
			}
		}
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
		pc().set_client(SharedFrom(this));

		world::ClientEnterMapNotify enter_notify;
		section->WriteTo(enter_notify.mutable_section_info());
		Send(enter_notify);
	}

	void WorldSession::OnSectionLeft(std::shared_ptr<Section> section) {
		DEBUG_ASSERT(IsSynchronized());

		if (section_ != section) {
			LOG_ERROR("WorldSession::OnSectionLeft section mismatch: expected {}, got {}", 
				section_ ? section->section_id() : 0, section->section_id());
			return;
		}

		section_.reset();
		pc().set_client(nullptr);

		auto notify = std::make_shared<world::OtherClientLeaveNotify>();
		notify->set_character_id(character_id_);
		section->Multicast(notify, session_id());
	}

	void WorldSession::OnVerifyClientAction(const std::shared_ptr<const world::ClientActionReq>& msg, world::ClientActionRes& res) {
		auto action_type = msg->client_action().ClientActionField_case();
		switch (action_type) {
		case types::ClientAction::kBaseAttackAction:
		{
			auto& combat = player().combat();
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

			IssueAction(types::ClientAction::kBaseAttackAction, 50000);

			res.set_result(types::Result::kSuccess);
			combat.SetLastBaseAttackTick(tick);
		}break;
			default:
				res.set_result(types::Result::kSuccess);
				break;
		}
	}

	int64_t WorldSession::IssueAction(const types::ClientAction::ClientActionFieldCase& type, int32_t expire_ms) {
		Model::ClientAction action;
		action.action_id = GenerateId();
		action.action_type = type;
		action.action_time = System::Tick::Current();
		action.expire_time = action.action_time.AddMilliseconds(expire_ms);		
		action_queue_.emplace_back(action);

		return action.action_id;
	}

	bool WorldSession::ConsumeAction(const types::ClientAction::ClientActionFieldCase& type, Model::ClientAction* out_action) {
		for (auto iter = action_queue_.begin(); iter != action_queue_.end(); ++iter) {
			if (iter->action_type == type) {
				*out_action = (*iter);
				action_queue_.erase(iter);
				return true;
			}
		}
		return false;
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
			.THEN([playing_character_id, session = SharedFrom(&session)](GrpcCallResult<CharacterLoginResponse> result) mutable {
			if (result.ok() == false) {
				LOG_ERROR("Failed to login character: {}", result.status.error_message());
				session->Disconnect();
				return;
			}
			
			int64_t account_id = result.response.account_id();

			Ctrl(*session).Post([playing_character_id, account_id](WorldSession& session) mutable {

				session.player().set_account_id(account_id);
				session.set_character_id(playing_character_id);
				if (session.player().LoadFromDb() == false) {
					session.Disconnect();
					return;
				}

				auto this_session = SharedFrom(&session);
				PlayerTick::BeginTick(this_session);

				world::HelloWorldClient hello_client;
				hello_client.set_map_uid(1);
				hello_client.set_server_tick_interval_ms(PlayerTick::kPlayerTickInterval);

				session.Send(hello_client);

				LOG_INFO("WorldSession::OnRecvHelloServer session_id:{}, address:{}, character_id:{}",
					session.session_id(), session.GetConnectionString(), session.character_id());
				});
			});
	}
	
	static void OnRecvClientEnterMapReq(WorldSession& session, const std::shared_ptr<const world::ClientEnterMapReq>& msg) {
		if (session.section() != nullptr) {
			LOG_ERROR("WorldSession::OnRecvClientEnterMapReq session_id:{} already in section: {}",
				session.session_id(), session.section()->section_id());
			return;
		}

		int32_t map_uid = msg->map_uid();
		if (map_uid > 0) {
			session.pc().ReadFrom(msg->character_pos());

			LOG_INFO("WorldSession::OnRecvClientEnterMapReq session_id:{}, address:{}, map_uid:{}",
				session.session_id(), session.GetConnectionString(), map_uid);

			auto session_ptr = SharedFrom(&session);

			SectionRepository::EnterSection(map_uid, SharedFrom(&session))
			.THEN_POST([session_ptr](Section& section) {
				auto res = std::make_shared<world::ClientEnterMapRes>();
				res->set_result(types::Result::kSuccess);
				section.WriteTo(res->mutable_section_info());
				session_ptr->Send(res);
			}).Catch([session_ptr](const std::exception&) {
				auto res = std::make_shared<world::ClientEnterMapRes>();
				res->set_result(types::Result::kInternalError);
				session_ptr->Send(res);
			});
		}
	}

	static void OnRecvClientMoveReq (WorldSession& session, const std::shared_ptr<const world::ClientMoveReq>& msg) {
		if (session.section() == nullptr) {
			LOG_ERROR("Section not found for session_id: {}", session.session_id());
			return;
		}

		auto session_ptr = SharedFrom(&session);
		Ctrl(*session.section()).Post([session_ptr, msg](Section&) {
			session_ptr->pc().ReadFrom((msg->character_pos()));
		});
	
		auto move_notify = std::make_shared<world::OtherClientMoveNotify>();
		move_notify->set_character_id(session.character_id());
		*move_notify->mutable_character_pos() = msg->character_pos();
		session.section()->Multicast(move_notify, session.session_id());

		world::ClientMoveRes res;
		res.set_client_timestamp(msg->client_timestamp());
		session.Send(res);
	}

	static void OnRecvClientActionReq (WorldSession& session, const std::shared_ptr<const world::ClientActionReq>& msg) {
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

		PlayerTick::SetServerTickInterVal(msg->server_tick_interval_ms());
		world::ChangeServerTickIntervalRes res;
		res.set_server_tick_interval_ms(msg->server_tick_interval_ms());
		session.Send(res);

		LOG_INFO("WorldSession::OnRecvChangeServerTickIntervalReq session_id:{}, address:{}, new_interval:{}",
			session.session_id(), session.GetConnectionString(), msg->server_tick_interval_ms());
	}

	static void OnRecvSpawnNpcOnSectionReq(WorldSession& session, const std::shared_ptr<const world::SpawnNpcOnSectionReq>& msg) {
		auto res = std::make_shared<world::SpawnNpcOnSectionRes>();
		if (session.section() == nullptr) {
			res->set_result(types::Result::kNotFound);
			session.Send(res);
			LOG_ERROR("Section not found for session_id: {}", session.session_id());
			return;
		}

		if (session.section()->GetOwnerCharacterId() != session.character_id()) {
			res->set_result(types::Result::kUnauthorized);
			session.Send(res);
			LOG_ERROR("Session {} is not the owner of section {}", session.session_id(), session.section()->section_id());
			return;
		}

		auto session_ptr = SharedFrom(&session);
		Ctrl(*session.section()).Post([msg, session_ptr, res](Section& section) {

			auto& spawn_system = section.spawn_system();

			const auto& spawner_data_record = spawn_system.spawner_data_record();
			if (spawner_data_record == nullptr) {
				res->set_result(types::Result::kInternalError);
				session_ptr->Send(res);
				LOG_ERROR("Spawner data record not found for section {}", section.section_id());
				return;
			}

			const auto& iter = spawner_data_record->items.find(msg->spawner_id());
			if (iter == spawner_data_record->items.end()) {
				res->set_result(types::Result::kNotFound);
				session_ptr->Send(res);
				LOG_ERROR("Spawner ID {} not found in section {}", msg->spawner_id(), section.section_id());
				return;
			}

			float next_spawn_tick = msg->client_timestamp() + static_cast<float>(iter->second.spawnDurationSeconds);
			res->set_next_client_timestamp(next_spawn_tick);

			const auto current_tick = System::Tick::Current();
			if ((current_tick - spawn_system.last_spawn_tick()).AsSecs() < static_cast<float>(iter->second.spawnDurationSeconds)) {
				res->set_result(types::Result::kInvalidCooldown);
				session_ptr->Send(res);
				return;
			}

			std::vector<types::NpcSpawnInfo> spawn_infos;
			for (const auto& spawn_info : msg->npc_spawn_infos()) {
				spawn_infos.push_back(spawn_info);
			}

			bool is_valid = spawn_system.IsValidSpawnInfo(msg->spawner_id(), spawn_infos);
			if (is_valid == false) {
				res->set_result(types::Result::kInvalidRequest);
				session_ptr->Send(res);
				return;
			}

			auto npcs = spawn_system.MakeNpcsFromSpawner(msg->spawner_id(), std::move(spawn_infos));
			section.SpawnMany(std::move(npcs));

			res->set_spawner_id(msg->spawner_id());
			res->set_result(types::Result::kSuccess);
			session_ptr->Send(res);
		});
	}

	static void OnRecvSpawnProjectileOnSectionReq(WorldSession& session, const std::shared_ptr<const world::SpawnProjectileOnSectionReq>& msg) {
		auto res = std::make_shared<world::SpawnProjectileOnSectionRes>();
		if (session.section() == nullptr) {
			res->set_result(types::Result::kNotFound);
			session.Send(res);
			LOG_ERROR("Section not found for session_id: {}", session.session_id());
			return;
		}

		const auto current_tick = System::Tick::Current();
		const Math::Vec3& expected_position = session.pc().GetExpectedPosition(current_tick);
		if (session.pc().IsValidProjectile(expected_position, msg->character_pose()) == false) {
			res->set_result(types::Result::kInvalidRequest);
			session.Send(res);
			LOG_ERROR("Invalid projectile pose for session_id: {}", session.session_id());
			return;
		}

		// TODO : Validate between character_pose and projectile_pose

		float projectile_speed = 5000.f;
		if (msg->projectile_speed() - projectile_speed > 0.01f || msg->projectile_speed() < 0.f) {
			LOG_ERROR("Invalid projectile speed: {} for session_id: {}", msg->projectile_speed(), session.session_id());
			res->set_result(types::Result::kInvalidRequest);
			session.Send(res);
			return;
		}

		Model::ClientAction action;
		if (session.ConsumeAction(types::ClientAction::kBaseAttackAction, &action) == false) {
			res->set_result(types::Result::kInvalidRequest);
			session.Send(res);
			LOG_ERROR("Failed to consume action for session_id: {}", session.session_id());
			return;
		}

		Math::Vec3 char_position = Utilites::ReadFrom(msg->character_pose().location());
		Math::Vec3 adjusted_vector = expected_position - char_position;

		Math::Vec3 projectile_position = Utilites::ReadFrom(msg->pose().location());
		projectile_position += adjusted_vector;

		Math::Vec3 projectile_rotator = Utilites::ReadFrom(msg->pose().rotation());

		// roll pitch yaw to forward vector
		Math::Vec3 forward_vector = Math::ForwardVectorFromEuler(projectile_rotator);

		auto projectile = std::make_shared<Projectile>();
		projectile->set_position(projectile_position);
		projectile->set_rotation(projectile_rotator);
		projectile->set_object_id(action.action_id);
		projectile->set_trigger_id(session.player().character_id());
		projectile->set_initial_speed(projectile_speed);
		projectile->set_direction(forward_vector);
		projectile->SetLifetimeSeconds(30);
		projectile->set_damage(20);

		res->set_result(types::Result::kSuccess);
		auto trajectory = projectile->simulate_trajectory(0.05f, 100);
		for (const auto& point : trajectory) {
			auto position = res->add_debug_trajectory();
			Utilites::WriteTo(point, position);;
		}
		res->set_object_id(projectile->object_id());

		Ctrl(*session.section()).Post([projectile, projectile_speed, session_ptr = SharedFrom(&session), res](Section& section) {
			section.SpawnObject(projectile);
			session_ptr->Send(res);
		});
	}

	static void OnRecvHitObjectByProjectileReq(WorldSession& session, const std::shared_ptr<const world::HitObjectByProjectileReq>& req) {
		auto res = std::make_shared<world::HitObjectByProjectileRes>();
		if (session.section() == nullptr) {
			res->set_result(types::Result::kNotFound);
			session.Send(res);
			LOG_ERROR("Section not found for session_id: {}", session.session_id());
			return;
		}

		Ctrl(*session.section()).Post([req, res, session_ptr = SharedFrom(&session)](Section& section) {
			auto projectile = section.spawn_system().FindProjectile(req->projectile_object_id());
			if (projectile == nullptr) {
				res->set_result(types::Result::kNotFound);
				session_ptr->Send(res);
				return;
			}

			auto hit_object = section.spawn_system().FindNpc(req->hit_object_id());
			if (hit_object == nullptr) {
				res->set_result(types::Result::kNotFound);
				session_ptr->Send(res);
				LOG_ERROR("Hit object not found for object_id: {}", req->hit_object_id());
				return;
			}

			Math::Vec3 hit_location = Utilites::ReadFrom(req->on_hit_location());
			Math::Vec3 current_location = projectile->GetExpectedPosition(System::Tick::Current());
			float distance = Math::DistanceTo(hit_location, current_location);
			if (distance > 50.f) {
				res->set_result(types::Result::kInvalidRequest);
				session_ptr->Send(res);
				LOG_ERROR("Hit location is too far {} from projectile's current location for object_id: {}", distance, req->hit_object_id());
				return;
			}

			projectile->HitObject(hit_object);

			res->set_result(types::Result::kSuccess);
			session_ptr->Send(res);
		});
	}

	void WorldSession::RegisterHandler(WorldHandlerMap* handler_map) {
		handler_map->Register(OnRegisterServerReq);
		handler_map->Register(OnRecvHelloWorldServer);
		handler_map->Register(OnRecvClientEnterMapReq);
		handler_map->Register(OnRecvClientMoveReq);
		handler_map->Register(OnRecvClientActionReq);
		handler_map->Register(OnRecvChangeServerTickIntervalReq);
		handler_map->Register(OnRecvSpawnNpcOnSectionReq);
		handler_map->Register(OnRecvSpawnProjectileOnSectionReq);
		handler_map->Register(OnRecvHitObjectByProjectileReq);
	}

	WorldService& WorldSession::GetService() {
		return *System::DependencyInjection::Get<WorldService>();
	}
}

