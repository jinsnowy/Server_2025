#pragma once

#include "Server/Service/HandlerMap.h"
#include "Server/Service/Protocol.h"

namespace Server {
	class ClientHandlerMap : public HandlerMap<ClientHandlerMap> {
	public:
		ClientHandlerMap(Protection) {
		}
	};
	using ClientProtocol = Protocol<ClientHandlerMap>;
}
