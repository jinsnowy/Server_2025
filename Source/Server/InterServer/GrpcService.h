#pragma once

#include "Protobuf/Public/GrpcHeader.h"
#include "Core/System/Scheduler.h"

namespace Server 
{
	template<typename TService>
	static void RunGrpcService(std::string server_address) {
        System::Scheduler::CreateThread([server_address]() {
            TService service;
            grpc::ServerBuilder builder;
            builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);

            auto server = builder.BuildAndStart();
            server->Wait();
        });
	}
}