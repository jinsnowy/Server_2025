#pragma once

#include "Service.h"
#include "Core/System/PeriodicTimer.h"

namespace grpc {
	class Status;
} // namespace grpc

namespace Server {
	class LobbyGrpcClient;
	class WorldService : public Service {
	public:
		WorldService();
		~WorldService();
		void Start() override;

		static void OnHealthCheck(System::PeriodicTimer::Handle& handle);

		static bool IsHealthy();

		void set_lobby_server_address(const std::string& address) {
			lobby_server_address_ = address;
		}

		LobbyGrpcClient& GetLobbyGrpcClient();

		void OnLobbyGrpcServiceInited();
		void OnLobbyGrpcServiceConnected();
		void OnLobbyGrpcServiceDisconnected(const grpc::Status& status);

	private:
		bool is_lobby_grpc_service_inited_ = false;
		bool is_registered_ = false;
		bool is_healthy_ = false;
		std::string lobby_server_address_;
		System::PeriodicTimer::Handle health_check_timer_handle_;
		std::unique_ptr<LobbyGrpcClient> lobby_grpc_client_;

		void RegisterServer();
	};
} // namespace Server
