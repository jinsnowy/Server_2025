#include "stdafx.h"
#include "LobbyService.h"

#include "Core/System/DependencyInjection.h"

namespace Server {

    LobbyService::LobbyService() {
    }

    void LobbyService::Start() {
        Service::Start();

        LobbyGrpcService::StartAsyncService();
	}
}

