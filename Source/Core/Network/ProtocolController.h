#pragma once

#include "Core/Network/Protocol.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Packet/PacketHandlerMap.h"

namespace Network {
	template<typename TPacket, typename TSerializer>
	class ProtocolController : public Protocol {
	public:
		bool ProcessMessage(Session& session, const size_t& packetId, const PacketSegment& segment) override;

		virtual bool HandleMessage(Session& session, const size_t& packetId, const std::shared_ptr<const TPacket>& message) = 0;

	protected:
		bool IsValid(const uint32_t& packetId) const {
			return TSerializer::IsValid(packetId);
		}

		PacketSegment Serialize(const TPacket& packet) const {
			return TSerializer::Serialize(packet);
		}

		PacketSegment Serialize(const std::shared_ptr<TPacket>& packet) const {
			return TSerializer::Serialize(*packet);
		}

		std::shared_ptr<TPacket> Parse(const int32_t& packetId, const PacketSegment& segment) const {
			return TSerializer::Deserialize(packetId, segment);
		}

		size_t GetPacketId(const TPacket& packet) const {
			return TSerializer::Resolve(packet);
		}
	};

	template<typename TPacket, typename TSerializer>
	inline bool ProtocolController<TPacket, TSerializer>::ProcessMessage(Session& session, const size_t& packetId, const PacketSegment& segment) {
		if (IsValid(packetId) == false) {
			return false;
		}

		auto packet = Parse(packetId, segment);
		if (packet == nullptr) {
			LOG_ERROR("ProtocolController cannnot parse segment packet_id:{}", packetId);
			return true;
		}

		return HandleMessage(session, packetId, packet);
	}
}