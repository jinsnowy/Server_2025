#include "stdafx.h"
#include "ServerProtocol.h"
#include "ServerHandlerMap.h"

namespace Server {

	bool ServerProtocol::ProcessMessage(Network::Session& session)
	{
		if (Protobuf::ProtobufProtocol::ProcessMessage(session) == false)
		{
			return false;
		}

		for (auto pair = packet_messages_.Dequeue(); pair.has_value(); pair = packet_messages_.Dequeue()) {
			if (ServerHandlerMap::GetInstance().HandleMessage(session, pair.value().first, pair.value().second) == false) {
				LOG_ERROR("ServerProtocol::HandleMessage: Failed to handle message with packetId: {}", pair.value().first);
				return false;
			}
		}

		return true;
	}
}

