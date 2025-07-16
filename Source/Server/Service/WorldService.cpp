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
				LOG_INFO("WorldService is healthy.");
			}
		}
		else {
			if (world_service->is_healthy_ == true) {
				world_service->is_healthy_ = false;
				LOG_INFO("WorldService is unhealthy. Error: {}", status.error_message());
			}
		}
	}

	bool WorldService::IsHealthy() {
		return System::DependencyInjection::Get<WorldService>()->is_healthy_;
	}
}

