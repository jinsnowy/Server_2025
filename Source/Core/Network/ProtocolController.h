#pragma once

#include "Core/Network/Protocol.h"
#include "Core/Network/Packet/PacketHandlerMap.h"

namespace Network {

	template<typename TPacket, typename TSerializer>
	class ProtocolController : public Protocol {
	public:
		ProtocolController(PacketHandlerMap<TPacket>* handler_map)
			:
			handler_map_(handler_map) {
		}

		virtual ~ProtocolController() = default;

		bool ProcessMessage(Session& session, const size_t& packetId, const PacketSegment& segment) override;

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

	private:
		PacketHandlerMap<TPacket>* handler_map_;
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

		return handler_map_->HandleMessage(session, packetId, packet);
	}
}