#include "stdafx.h"
#include "LobbyService.h"

#include "Core/System/DependencyInjection.h"

namespace Server {

    LobbyService::LobbyService() {
    }

    void LobbyService::Start() {
        Service::Start();

        System::Scheduler::CreateThread([]() {
            LobbyGrpcService service;
            grpc::ServerBuilder builder;
            builder.AddListeningPort(LobbyGrpcService::kListenAddress, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            LOG_INFO("LobbyGrpcService started on : {}", LobbyGrpcService::kListenAddress);
            auto server = builder.BuildAndStart();
            server->Wait();
        });
	}
}

