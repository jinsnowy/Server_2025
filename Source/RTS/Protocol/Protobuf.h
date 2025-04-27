#pragma once

#include "Core/Network/ProtocolController.h"

namespace RTS {
	class Protobuf : public Network::ProtocolController<google::protobuf::Message, > {
	};
}

