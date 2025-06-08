#pragma once

#include "Core/Network/Protocol.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Packet/PacketHandlerMap.h"
#include "Core/Container/SPSC.h"

namespace Network {
	template<typename TPacket, typename TSerializer>
	class ProtocolController : public Protocol {
	public:
		bool ProcessReceiveData(const PacketSegment& segment) override;


	protected:
		bool IsValid(const size_t& packetId) const {
			return TSerializer::IsValid(packetId);
		}

		PacketSegment Serialize(const TPacket& packet) const {
			return TSerializer::Serialize(packet);
		}

		PacketSegment Serialize(const std::shared_ptr<TPacket>& packet) const {
			return TSerializer::Serialize(*packet);
		}

		std::shared_ptr<TPacket> Parse(const size_t& packetId, const void* data, const size_t& data_size) const {
			return TSerializer::Deserialize(packetId, data, data_size);
		}

		size_t GetPacketId(const TPacket& packet) const {
			return TSerializer::Resolve(packet);
		}

	protected:
		Container::SPSCQueue<std::pair<size_t, std::shared_ptr<TPacket>>> packet_messages_;
	};

	template<typename TPacket, typename TSerializer>
	inline bool ProtocolController<TPacket, TSerializer>::ProcessReceiveData(const PacketSegment& segment) {
		if (Protocol::ProcessReceiveData(segment) == true) {
			return true;
		}

		const size_t packetId = segment.header().id;
		if (IsValid(packetId) == false) {
			return false;
		}

		std::shared_ptr<TPacket> packet = Parse(packetId, segment.body(), segment.body_length());
		if (packet == nullptr) {
			LOG_ERROR("ProtocolController cannnot parse segment packet_id:{}", packetId);
			return true;
		}

		packet_messages_.Enqueue(packetId, packet);
		return true;
	}
}