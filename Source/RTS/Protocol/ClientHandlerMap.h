#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace RTS {
	class ClientHandlerMap : public Protobuf::ProtobufHandlerMap, public System::Singleton<ClientHandlerMap> {
	public:
		ClientHandlerMap();
		~ClientHandlerMap();
	};
}
