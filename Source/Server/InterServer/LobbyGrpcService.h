#pragma once

#include "Protobuf/Public/GrpcHeader.h"
#include "Protobuf/Public/LobbyService.h"

namespace Server {
    class LobbyGrpcService : public lobby_service::LobbyService::Service {
    public:
        static constexpr const char* kListenAddress = "0.0.0.0:21213";
        static constexpr const char* kConnectAddress = "127.0.0.1:21213";

        ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::lobby_service::RegisterServerRequest* req, ::lobby_service::RegisterServerReponse* res) override;
		::grpc::Status Ping(::grpc::ServerContext* /*context*/, const ::lobby_service::PingRequest* req, ::lobby_service::PingResponse* res) override;
    };
}