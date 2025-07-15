#pragma once

#include "Protobuf/Public/ProtobufProtocol.h"

namespace Server {

	template<typename THandlerMap>
	class Protocol : public Protobuf::ProtobufProtocol {
	public:
		bool ProcessMessage(Network::Session& session) {
			if (Protobuf::ProtobufProtocol::ProcessMessage(session) == false)
			{
				return false;
			}

			for (auto pair = packet_messages_.Dequeue(); pair.has_value(); pair = packet_messages_.Dequeue()) {
				if (THandlerMap::GetInstance().HandleMessage(session, pair.value().first, pair.value().second) == false) {
					LOG_ERROR("ClientProtocol::HandleMessage: Failed to handle message with packetId: {}", pair.value().first);
					return false;
				}
			}

			return true;
		}
	};
}


