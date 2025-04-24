#include "stdafx.h"
#include "ServerSession.h"

namespace RTS {

	ServerSession::ServerSession() {
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

	bool ServerSession::OnReceive(std::string message) {
		LOG_INFO("ServerSession::OnRecv: {}", message.c_str());
		return true;
	}
}

