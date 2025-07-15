#pragma once

#include "Protobuf/Public/GrpcHeader.h"
#include "Protobuf/Public/LobbyService.h"

namespace Server {
    class LobbyGrpcService : public lobby_service::LobbyService::Service {
    public:
        static constexpr const char* kAddress = "0.0.0.0:21213";

        ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::lobby_service::RegisterServerRequest* req, ::lobby_service::RegisterServerReponse* res) override;
    };
}