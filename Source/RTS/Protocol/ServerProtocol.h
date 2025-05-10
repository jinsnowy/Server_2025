#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace RTS {
	class ServerProtocol : public Protobuf::ProtobufProtocol {
	public:
		bool HandleMessage(Network::Session& session, const size_t& packetId, const std::shared_ptr<const google::protobuf::Message>& message) override;
	};

}

