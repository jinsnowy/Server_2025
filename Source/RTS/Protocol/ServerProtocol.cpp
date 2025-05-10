#include "stdafx.h"
#include "ServerProtocol.h"
#include "ServerHandlerMap.h"

namespace RTS {
	bool ServerProtocol::HandleMessage(Network::Session& session, const size_t& packetId, const std::shared_ptr<const google::protobuf::Message>& message) {
		if (ServerHandlerMap::GetInstance().HandleMessage(session, packetId, message) == false) {
			LOG_ERROR("ServerProtocol::HandleMessage: Failed to handle message with packetId: {}", packetId);
			return false;
		}

		return true;
	}
}

