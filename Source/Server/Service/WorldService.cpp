#include "stdafx.h"
#include "WorldService.h"

namespace Server {
	void WorldService::Start() {
		Service::Start();
		auto channel = grpc::CreateChannel(lobby_server_address_, grpc::InsecureChannelCredentials());
		lobby_service_stub_ = lobby_service::LobbyService::NewStub(channel);
		LOG_INFO("WorldService started.");
	}
}

