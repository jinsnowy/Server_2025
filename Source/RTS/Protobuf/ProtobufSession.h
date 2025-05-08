#pragma once

#include "Core/Network/Session.h"

namespace RTS {
	class ProtobufSession : public Network::Session {
	public:
		void Send(const google::protobuf::Message& message);
		void Send(const std::shared_ptr<const google::protobuf::Message>& message);
	};


}
