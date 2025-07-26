#pragma once

#include "Protobuf/Public/GrpcHeader.h"
#include "Protobuf/Public/LobbyService.h"

namespace Server {
    class LobbyGrpcServiceCallback;
    class LobbyGrpcService : public lobby_service::LobbyService::AsyncService {
    public:
        static constexpr const char* kListenAddress = "0.0.0.0:21213";
        static constexpr const char* kConnectAddress = "127.0.0.1:21213";

        static void StartAsyncService();

    private:
      
		static void RegisterHandlers(LobbyGrpcServiceCallback&);
    };
}