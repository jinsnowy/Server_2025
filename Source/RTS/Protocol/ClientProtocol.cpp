#include "stdafx.h"
#include "ClientProtocol.h"
#include "ClientHandlerMap.h"

namespace RTS {

	bool ClientProtocol::HandleMessage(Network::Session& session, const size_t& packetId, const std::shared_ptr<const google::protobuf::Message>& message) {
		if (ClientHandlerMap::GetInstance().HandleMessage(session, packetId, message) == false) {
			LOG_ERROR("ClientProtocol::HandleMessage: Failed to handle message with packetId: {}", packetId);
			return false;
		}

		return true;
	}
}

