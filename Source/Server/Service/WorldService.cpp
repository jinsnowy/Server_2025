#include "stdafx.h"
#include "WorldService.h"
#include "Protobuf/Public/LobbyService.h"

namespace Server {
	void WorldService::Start() {
		Service::Start();

		auto channel = grpc::CreateChannel(lobby_server_address_, grpc::InsecureChannelCredentials());
		lobby_service_stub_ = lobby_service::LobbyService::NewStub(channel);

		health_check_timer_handle_ = System::PeriodicTimer::Schedule(System::Duration::FromSeconds(1), OnHealthCheck, true);

		LOG_INFO("WorldService started.");
	}

	void WorldService::OnHealthCheck(System::PeriodicTimer::Handle&) {
		// WorldService that is implemented class of SingletonServiceInterface
		auto world_service = System::DependencyInjection::Get<WorldService>();

		grpc::ClientContext context;
		lobby_service::PingRequest request;
		lobby_service::PingResponse response;

		auto status = world_service->lobby_service_stub_->Ping(&context, request, &response);
		if (status.ok()) {
			if (world_service->is_healthy_ == false) {
				world_service->is_healthy_ = true;
				if (world_service->is_lobby_grpc_service_inited_ == false) {
					world_service->is_lobby_grpc_service_inited_ = true;
					world_service->OnLobbyGrpcServiceInited();
				}
				world_service->OnLobbyGrpcServiceConnected();
			}
		}
		else {
			if (world_service->is_healthy_ == true) {
				world_service->is_healthy_ = false;
				world_service->OnLobbyGrpcServiceDisconnected(status);
			}
		}
	}

	bool WorldService::IsHealthy() {
		return System::DependencyInjection::Get<WorldService>()->is_healthy_;
	}

	void WorldService::OnLobbyGrpcServiceInited() {
		RegisterServer();
	}

	void WorldService::OnLobbyGrpcServiceConnected() {
		LOG_INFO("WorldService is healthy.");
	}

	void WorldService::OnLobbyGrpcServiceDisconnected(const grpc::Status& status)
	{
		LOG_ERROR("WorldService disconnected from LobbyGrpcService. Status: {}, Error: {}",  static_cast<int32_t>(status.error_code()), status.error_message());
	}

	void WorldService::RegisterServer() {
		if (is_registered_) {
			LOG_INFO("WorldService is already registered with LobbyGrpcService.");
			return;
		}

		grpc::ClientContext context;
		lobby_service::RegisterServerRequest request;
		request.set_server_address(ipaddress_.ToString());
		request.set_server_type(types::kWorldServer);

		lobby_service::RegisterServerReponse response;
		auto status = lobby_service_stub_->RegisterServer(&context, request, &response);
		if (status.ok() == false || response.result() != types::kSuccess) {
			LOG_ERROR("Failed to register WorldService with LobbyGrpcService. Status: {}, Error: {}, Result: {}",
				static_cast<int32_t>(status.error_code()), status.error_message(), types::Result_Name(response.result()));
			return;
		}
		is_registered_ = true;
	}
}

