#pragma once

#include "Service.h"
#include "../InterServer/LobbyGrpcService.h"

namespace Server {
	class WorldService : public Service {
	public:
		void Start() override;

		void set_lobby_server_address(const std::string& address) {
			lobby_server_address_ = address;
		}

		lobby_service::LobbyService::Stub& GetLobbyServiceStub() {
			return *lobby_service_stub_.get();
		}

	private:
		std::string lobby_server_address_;
		std::unique_ptr<lobby_service::LobbyService::Stub> lobby_service_stub_;
	};
} // namespace Server
