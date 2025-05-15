#pragma once

#include "Core/Network/Session.h"

namespace Protobuf {
	class ProtobufSession : public Network::Session {
	public:
		ProtobufSession(const std::shared_ptr<System::Context>& context);
		ProtobufSession();

		void OnConnected() override;

		void Send(const google::protobuf::Message& message);
		void Send(const std::shared_ptr<const google::protobuf::Message>& message);
	};


}
