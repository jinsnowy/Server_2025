#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace Server {
	class ServerHandlerMap : public Protobuf::ProtobufHandlerMap, public System::Singleton<ServerHandlerMap> {
	public:
		ServerHandlerMap();
		~ServerHandlerMap();
	};
}
