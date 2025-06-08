#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace Server {
	class ClientHandlerMap : public Protobuf::ProtobufHandlerMap, public System::Singleton<ClientHandlerMap> {
	public:
		ClientHandlerMap();
		~ClientHandlerMap();
	};
}
