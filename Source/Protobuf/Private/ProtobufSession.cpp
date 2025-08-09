#include "stdafx.h"
#include "Protobuf/Public/ProtobufSession.h"
#include "Protobuf/Public/ProtobufSerializer.h"
#include "Protobuf/Public/ProtobufProtocol.h"
#include "Protobuf/Public/ProtobufNode.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/OutputStream.h"

namespace Protobuf {
	ProtobufSession::ProtobufSession(const System::Channel& channel)
		:
		Network::Session(channel) {
	}
	
	ProtobufSession::ProtobufSession()
		:
		Network::Session() {
	}

	void ProtobufSession::OnConnected(const Network::IPAddress& address) {
		Network::Session::OnConnected(address);
	}

	void ProtobufSession::Send(const google::protobuf::Message& message) {
		uint64_t message_id = Protobuf::ProtobufSerializer::Resolve(message);
		Network::Session::Send(std::make_unique<Network::RawNode>(message_id, message.SerializeAsString()));
	}

	void ProtobufSession::Send(const std::shared_ptr<const google::protobuf::Message>& message) {
		uint64_t message_id = Protobuf::ProtobufSerializer::Resolve(*message);
		Network::Session::Send(std::make_unique<ProtobufNode>(message_id, message));
	}
}

