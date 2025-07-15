#pragma once

#include "Server/Service/HandlerMap.h"
#include "Server/Service/Protocol.h"

namespace Server {

	class LobbyHandlerMap : public HandlerMap<LobbyHandlerMap> {};
	using LobbyProtocol = Protocol<LobbyHandlerMap>;

}
