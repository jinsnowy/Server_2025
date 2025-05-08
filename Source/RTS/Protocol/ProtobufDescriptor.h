#pragma once

#include "Core/Network/Packet/Packet.h"

namespace RTS {
	struct ProtoSerializer {
		std::vector<char> Serialize(const google::protobuf::Message& packet);
		std::shared_ptr<google::protobuf::Message> Deserialize(const size_t& packetId, const Network::PacketSegment& segment);
		uint32_t Resolve(const google::protobuf::Message& packet);
		bool IsValid(const uint32_t& packetId);
	};

	class ProtocolHelper final {
	public:
		template<typename THandler>
		using PacketHandlerMapType = Tcp::Packet::PacketHandlerMap<THandler, Message, ProtoSerializer>;

		template<typename TPacketHandlerMap, typename THandler, typename TPacket>
		static void RegisterHandler(TPacketHandlerMap& handlerMap, void (*handlerFunc)(THandler&, const std::shared_ptr<const TPacket>&)) {
			DescriptorRegistry<TPacket> registry;
			handlerMap.RegisterHandler(registry.messageId, std::forward<decltype(handlerFunc)>(handlerFunc));
		}

		template<typename TPacket>
		static uint32_t GetMessageId() {
			const auto& descriptor = TPacket::default_instance().descriptor();
			return GetMessageId(descriptor);
		}

		static uint32_t GetMessageId(const google::protobuf::Message& message);
		static std::shared_ptr<google::protobuf::Message> CreateMessage(const uint32_t packet_id);
		static const google::protobuf::Descriptor* GetDescriptor(int32_t messageId);

	private:
		template<typename TPacket>
		struct DescriptorRegistry {
			uint32_t messageId;
			DescriptorRegistry() {
				const auto& descriptor = TPacket::default_instance().descriptor();
				messageId = GetMessageId(descriptor);
				RegisterDescriptor(messageId, descriptor);
			}
		};

		static uint32_t GetMessageId(const google::protobuf::Descriptor* descriptor);
		static void RegisterDescriptor(int32_t messageId, const google::protobuf::Descriptor* descriptor);
	};

	//using ProtocolController = Tcp::Packet::ProtocolController<google::protobuf::Message>;

	//google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint);
	//System::Time FromTimestamp(const google::protobuf::Timestamp& timestamp);
}