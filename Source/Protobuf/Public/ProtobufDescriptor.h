#pragma once

#include "Core/Network/Packet/Packet.h"

namespace Protobuf {
	class ProtobufDescriptor final {
	public:
		template<typename TPacket>
		static const google::protobuf::Descriptor* GetDescriptor() {
			return TPacket::default_instance().descriptor();
		}

		static size_t GetMessageId(const google::protobuf::Descriptor* descriptor);
		static void RegisterDescriptor(size_t messageId, const google::protobuf::Descriptor* descriptor);

		static size_t GetMessageId(const google::protobuf::Message& message);
		static std::shared_ptr<google::protobuf::Message> CreateMessage(const size_t packet_id);
		static const google::protobuf::Descriptor* GetDescriptor(size_t messageId);

	private:
		template<typename TPacket>
		struct DescriptorRegistry {
			size_t messageId;
			DescriptorRegistry() {
				const auto& descriptor = TPacket::default_instance().descriptor();
				messageId = GetMessageId(descriptor);
				RegisterDescriptor(messageId, descriptor);
			}
		};
	};

	//using ProtocolController = Tcp::Packet::ProtocolController<google::protobuf::Message>;

	//google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint);
	//System::Time FromTimestamp(const google::protobuf::Timestamp& timestamp);
}