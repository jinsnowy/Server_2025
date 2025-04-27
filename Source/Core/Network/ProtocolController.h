#pragma once

#include "Core/Network/Protocol.h"

namespace Network {

	template<typename TPacket, typename TSerializer>
	class ProtocolController : public Protocol {
	public:
		virtual ~ProtocolController() = default;

		virtual void Send(const TPacket& packet) = 0;
		virtual void Send(const std::shared_ptr<const TPacket>& packet) = 0;

		bool ProcessMessage(const size_t& packetId, const PacketSegment& segment) override;

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

		virtual void HandleMessage(const size_t& packetId, const std::shared_ptr<const TPacket>& packet) = 0;
	};

	template<typename TPacket, typename TSerializer>
	inline bool ProtocolController<TPacket, TSerializer>::ProcessMessage(const size_t& packetId, const PacketSegment& segment) {
		if (IsValid(packetId) == false) {
			return false;
		}

		auto packet = Parse(packetId, segment);
		if (packet == nullptr) {
			LOG_ERROR("ProtocolController cannnot parse segment packet_id:{}", packetId);
			return true;
		}

		HandleMessage(packetId, packet);
		return true;
	}

	template<typename TPacket, typename TSerializer>
	inline void ProtocolController<TPacket, TSerializer>::HandleMessage(const size_t& packetId, const std::shared_ptr<const TPacket>& packet)
	{
	}
}