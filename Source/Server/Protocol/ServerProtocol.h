#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace Server {
	class ServerProtocol : public Protobuf::ProtobufProtocol {
	public:
		bool ProcessMessage(Network::Session& session) override;
	};

}

