#pragma once

#include "Core/Network/ProtocolController.h"
#include "ProtoSerializer.h"

namespace RTS {
	using ProtobufProtocol = Network::ProtocolController<google::protobuf::Message, ProtoSerializer>;
}