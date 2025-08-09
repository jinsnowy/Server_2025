#pragma once

#include "Server/Service/HandlerMap.h"
#include "Server/Service/Protocol.h"

namespace Server {

	class LobbyHandlerMap : public HandlerMap<LobbyHandlerMap> {
	public:
		LobbyHandlerMap(Protection) {}
	};
	using LobbyProtocol = Protocol<LobbyHandlerMap>;

}
