#pragma once

#include "Server/Service/HandlerMap.h"
#include "Server/Service/Protocol.h"

namespace Server {

	class WorldHandlerMap : public HandlerMap<WorldHandlerMap> {
	public:
		WorldHandlerMap(Protection){}
	};
	using WorldProtocol = Protocol<WorldHandlerMap>;
}
