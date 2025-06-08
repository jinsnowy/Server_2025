#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace Server {
	class ClientProtocol : public Protobuf::ProtobufProtocol {
	public:
		bool ProcessMessage(Network::Session& session);
	};

}

