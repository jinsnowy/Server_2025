#pragma once

#include "Core/Network/Session.h"

namespace Protobuf {
	class ProtobufSession : public Network::Session {
	public:
		ProtobufSession(const System::Channel& channel);
		ProtobufSession();

		void OnConnected(const Network::IPAddress& address) override;

		void Send(const google::protobuf::Message& message);
		void Send(const std::shared_ptr<const google::protobuf::Message>& message);
	};
}
