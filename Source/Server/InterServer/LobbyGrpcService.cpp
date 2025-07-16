#include "stdafx.h"
#include "LobbyGrpcService.h"
#include "../Database/DB.h"

namespace Server {
	::grpc::Status LobbyGrpcService::RegisterServer(::grpc::ServerContext*, const::lobby_service::RegisterServerRequest* req, ::lobby_service::RegisterServerReponse* res) {
		auto agent = DB::GetLobbyDB().GetAgent();
		auto stmt = agent->CreateStmt();
		agent;
		stmt;
		req;
		res;
		return ::grpc::Status::OK;
	}

	::grpc::Status LobbyGrpcService::Ping(::grpc::ServerContext*, const::lobby_service::PingRequest*, ::lobby_service::PingResponse*) {
		return ::grpc::Status::OK;
	}
}

