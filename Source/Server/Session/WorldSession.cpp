#include "stdafx.h"
#include "WorldSession.h"
#include "Core/Sql/Database.h"
#include "Service/WorldServiceDef.h"
#include "Protobuf/Public/World.h"

#include "SessionId.h"
#include "Server/Database/DB.h"
#include "Server/Authenticator/Authenticator.h"

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

		session_id_ = SessionId::Issue(SessionType::kWorldSession);

		LOG_INFO("WorldSession::OnConnected session_id:{}, address:{}", session_id_, GetConnectionString());
	}

	void WorldSession::OnDisconnected() {
		ProtobufSession::OnDisconnected();

		LOG_INFO("WorldSession::OnDisconnected session_id:{}, address:{}", session_id_, GetConnectionString());
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

	void WorldSession::RegisterHandler(WorldHandlerMap* handler_map) {
		handler_map->Register(OnRegisterServerReq);
	}
}

