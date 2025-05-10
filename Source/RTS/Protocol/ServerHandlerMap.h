#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace RTS {
	class ServerHandlerMap : public Protobuf::ProtobufHandlerMap, public System::Singleton<ServerHandlerMap> {
	public:
		ServerHandlerMap();
		~ServerHandlerMap();
	};
}
