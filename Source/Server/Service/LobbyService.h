#pragma once

#include "Service.h"
#include "../InterServer/LobbyGrpcService.h"
#include "Core/System/PeriodicTimer.h"

namespace Server {
	class LobbyService : public Service {
	public:
		LobbyService();

		void Start() override;

	private:
		std::shared_ptr<grpc::Server> lobby_grpc_server_;
	};
}

