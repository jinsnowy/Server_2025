#include "stdafx.h"
#include "ClientProtocol.h"
#include "ClientHandlerMap.h"

namespace Server {
	bool ClientProtocol::ProcessMessage(Network::Session& session)
	{
		if (Protobuf::ProtobufProtocol::ProcessMessage(session) == false)
		{
			return false;
		}

		for (auto pair = packet_messages_.Dequeue(); pair.has_value(); pair = packet_messages_.Dequeue()) {
			if (ClientHandlerMap::GetInstance().HandleMessage(session, pair.value().first, pair.value().second) == false) {
				LOG_ERROR("ClientProtocol::HandleMessage: Failed to handle message with packetId: {}", pair.value().first);
				return false;
			}
		}

		return true;
	}
}

