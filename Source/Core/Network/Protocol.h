#pragma once

namespace Network {
	//template<typename TPacket>
	//class ProtocolController {
	//public:
	//	virtual ~ProtocolController() = default;

	//	virtual void Send(const TPacket& packet) = 0;
	//	virtual void Send(const std::shared_ptr<const TPacket>& packet) = 0;

	//protected:
	//	bool IsValid(const uint32_t& packetId) const {
	//		return TSerializer::IsValid(packetId);
	//	}

	//	bool ProcessMessage(const uint32_t& packetId, const Memory::Segment& segment);

	//	Memory::Segment Serialize(const TPacket& packet) const {
	//		return TSerializer::Serialize(packet);
	//	}

	//	Memory::Segment Serialize(const std::shared_ptr<TPacket>& packet) const {
	//		return TSerializer::Serialize(*packet);
	//	}

	//	std::shared_ptr<TPacket> Parse(const int32_t& packetId, const Memory::Segment& segment) const {
	//		return TSerializer::Deserialize(packetId, segment);
	//	}

	//	uint32_t GetPacketId(const TPacket& packet) const {
	//		return TSerializer::Resolve(packet);
	//	}

	//	virtual void HandleMessage(const uint32_t& packetId, const std::shared_ptr<const TPacket>& packet) = 0;
	//};

	//template<typename TPacket>
	//inline bool ProtocolController<TPacket>::ProcessMessage(const uint32_t& packetId, const Memory::Segment& segment) {
	//	if (IsValid(packetId) == false) {
	//		return false;
	//	}

	//	auto packet = Parse(packetId, segment);
	//	if (packet == nullptr) {
	//		LOG_ERROR("[{}] cannnot parse {}", typeid(ProtocolController<TPacket>).name(), packetId);
	//		return true;
	//	}

	//	HandleMessage(packetId, packet);
	//	return true;
	//}

	//template<typename TPacket>
	//inline void ProtocolController<TPacket>::HandleMessage(const uint32_t& packetId, const std::shared_ptr<const TPacket>& packet)
	//{
	//}
}