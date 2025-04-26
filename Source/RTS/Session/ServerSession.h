#pragma once

#include "Core/Network/Session.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

namespace RTS {
	class ServerSession : public Network::Session {
	public:
		ServerSession(std::shared_ptr<Network::Connection> conn);
		~ServerSession();

		void Send(const std::shared_ptr<const google::protobuf::Message>& message);
		void OnMessage(const std::string& message) override;
	};
}

