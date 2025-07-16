#pragma once

#include "Service.h"
#include "../InterServer/LobbyGrpcService.h"
#include "Core/System/PeriodicTimer.h"

namespace grpc {
	class Status;
} // namespace grpc

namespace Server {
	class WorldService : public Service {
	public:
		void Start() override;

		static void OnHealthCheck(System::PeriodicTimer::Handle& handle);

		static bool IsHealthy();

		void set_lobby_server_address(const std::string& address) {
			lobby_server_address_ = address;
		}

		lobby_service::LobbyService::Stub& GetLobbyServiceStub() {
			return *lobby_service_stub_.get();
		}

		void OnLobbyGrpcServiceInited();
		void OnLobbyGrpcServiceConnected();
		void OnLobbyGrpcServiceDisconnected(const grpc::Status& status);

	private:
		bool is_lobby_grpc_service_inited_ = false;
		bool is_registered_ = false;
		bool is_healthy_ = false;
		std::string lobby_server_address_;
		System::PeriodicTimer::Handle health_check_timer_handle_;
		std::unique_ptr<lobby_service::LobbyService::Stub> lobby_service_stub_;

		void RegisterServer();
	};
} // namespace Server
