#include "stdafx.h"
#include "WorldService.h"
#include "Protobuf/Public/LobbyService.h"
#include "../InterServer/LobbyGrpcClient.h"
#include "../InterServer/GrpcService.h"

namespace Server {
	WorldService::WorldService() = default;
	WorldService::~WorldService() = default;

	void WorldService::Start() {
		Service::Start();

		lobby_grpc_client_ = std::make_unique<LobbyGrpcClient>(lobby_server_address_);
		health_check_timer_handle_ = System::PeriodicTimer::Schedule(System::Duration::FromMilliseconds(10), OnHealthCheck, true);

		LOG_INFO("WorldService started.");
	}

	void WorldService::OnHealthCheck(System::PeriodicTimer::Handle&) {
		// WorldService that is implemented class of SingletonServiceInterface
		auto world_service = System::DependencyInjection::Get<WorldService>();
		auto request = std::make_shared<PingRequest>();
	
		world_service->lobby_grpc_client_->Ping(request)
		.Then([world_service](GrpcCallResult<PingResponse> result) {
			if (result.ok()) {
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
					world_service->OnLobbyGrpcServiceDisconnected(result.status);
				}
			}
		});
	}

	bool WorldService::IsHealthy() {
		return System::DependencyInjection::Get<WorldService>()->is_healthy_;
	}

	LobbyGrpcClient& WorldService::GetLobbyGrpcClient()
	{
		return *lobby_grpc_client_;
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

		auto request = std::make_shared<lobby_service::RegisterServerRequest>();
		request->set_server_address(ipaddress_.ToString());
		request->set_server_type(types::kWorldServer);

		lobby_service::RegisterServerReponse response;
		lobby_grpc_client_->RegisterServer(request)
		.Then([this](GrpcCallResult<RegisterServerReponse> result) {
			if (result.ok() && result.response.result() == types::Result::kSuccess) {
				is_registered_ = true;
				server_id_ = static_cast<int16_t>(result.response.server_id());
			}
			result.response.result() == types::Result::kSuccess ? 
				LOG_INFO("WorldService registered successfully.") : LOG_ERROR("Failed to register WorldService");
		});
	}
}

