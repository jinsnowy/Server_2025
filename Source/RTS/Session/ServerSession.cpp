#include "stdafx.h"
#include "ServerSession.h"

namespace RTS {
	ServerSession::ServerSession(std::shared_ptr<Network::Connection> conn)
		:
		Network::Session(std::move(conn)) {
	}

	ServerSession::~ServerSession() {
	}

	void ServerSession::Send(const std::shared_ptr<const google::protobuf::Message>& message) {
		std::string data;
		if (!message->SerializeToString(&data)) {
			return;
		}

		SendMessage(data);
	}

	void ServerSession::OnMessage(const std::string& message) {
		LOG_INFO("ServerSession::OnMessage: {}", message.c_str());
	}
}

