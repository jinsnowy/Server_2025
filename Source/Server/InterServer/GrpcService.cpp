#include "stdafx.h"
#include "GrpcService.h"

namespace Server {
	void StopGrpcService() {
        for (auto& server : sevices) {
            if (server) {
                server->Shutdown(std::chrono::system_clock::now());
            }
        }
        sevices.clear();
    }
}