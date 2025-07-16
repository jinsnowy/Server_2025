#pragma once

#include "Service.h"
#include "../InterServer/LobbyGrpcService.h"
#include "Core/System/PeriodicTimer.h"

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

		static void OnHealthCheck(System::PeriodicTimer::Handle& handle);

		static bool IsHealthy();

	private:
		bool is_healthy_ = false;
		std::string lobby_server_address_;
		System::PeriodicTimer::Handle health_check_timer_handle_;
		std::unique_ptr<lobby_service::LobbyService::Stub> lobby_service_stub_;
	};
} // namespace Server
