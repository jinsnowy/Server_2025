#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace Server {
	template<typename THandlerMap>
	class HandlerMap : public Protobuf::ProtobufHandlerMap, public System::Singleton<THandlerMap> {
	};

}
