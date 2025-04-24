#pragma once

#include "Core/Network/Session.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

namespace RTS {
	class ServerSession : public Network::Session {
	public:
		ServerSession();
		~ServerSession();

		void Send(const std::shared_ptr<const google::protobuf::Message>& message);
		bool OnReceive(std::string message) override;
	};
}

