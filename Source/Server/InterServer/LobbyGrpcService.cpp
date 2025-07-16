#include "stdafx.h"
#include "LobbyGrpcService.h"
#include "../Database/DB.h"
#include "../Model/Server.h"

namespace Server {
	::grpc::Status LobbyGrpcService::RegisterServer(::grpc::ServerContext*, const::lobby_service::RegisterServerRequest* req, ::lobby_service::RegisterServerReponse* res) {
		auto agent = DB::GetLobbyDB().GetAgent();
		Model::Server server;
		server.server_address = req->server_address();
		server.server_type = static_cast<Model::ServerType>(req->server_type());
		if (server.UpsertToDb(*agent) == false) {
			LOG_ERROR("Failed to upsert server: server_address: {}, server_type: {}", server.server_address, System::Enums::ToString(server.server_type));
			res->set_result(types::Result::kDatabaseError);
			return ::grpc::Status::OK;
		}
		return ::grpc::Status::OK;
	}

	::grpc::Status LobbyGrpcService::Ping(::grpc::ServerContext*, const::lobby_service::PingRequest*, ::lobby_service::PingResponse*) {
		return ::grpc::Status::OK;
	}
}

