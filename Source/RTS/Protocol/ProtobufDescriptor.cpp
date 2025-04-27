#include "stdafx.h"
#include "ProtobufDescriptor.h"

namespace RTS {
	using namespace google::protobuf;

	class Impl : public System::Singleton<Impl> {
	public:
		std::unordered_map<uint32_t, const google::protobuf::Descriptor*> _descriptors;

		void AddDescriptor(uint32_t message_id, const google::protobuf::Descriptor* descriptor) {
			_descriptors.try_emplace(message_id, descriptor);
		}

		const google::protobuf::Descriptor* GetDescriptor(uint32_t message_id) {
			if (auto iter = _descriptors.find(message_id); iter != _descriptors.end()) {
				return iter->second;
			}
			return nullptr;
		}
	};

	std::vector<char> ProtoSerializer::Serialize(const Message& packet) {
		std::vector<char> buffer(packet.ByteSizeLong());
		if (packet.SerializeToArray(buffer.data(), buffer.size()) == false) {
			LOG_ERROR("serialize protobuf failed: {}", packet.GetTypeName());
			return{};
		}

		return buffer;
	}

	std::shared_ptr<Message> ProtoSerializer::Deserialize(const size_t& packetId, const Network::PacketSegment& segment) {
		auto* descriptor = ProtocolHelper::GetDescriptor(packetId);
		if (descriptor == nullptr) {
			LOG_ERROR("cannot get descriptor packet_id: {}", packetId);
			return nullptr;
		}

		const auto* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
		std::shared_ptr<Message> message(prototype->New());
		if (message->ParseFromArray(segment.body(), segment.body_length()) == false) {
			LOG_ERROR("parse failed packet_id: {}, name: {}", packetId, prototype->GetTypeName());
			return nullptr;
		}
		return message;
	}

	uint32_t ProtoSerializer::Resolve(const Message& packet) {
		return packet.GetDescriptor()->options().GetExtension(type::message_id);
	}

	bool ProtoSerializer::IsValid(const uint32_t& packetId) {
		return type::protocol_IsValid(packetId);
	}

	uint32_t ProtocolHelper::GetMessageId(const google::protobuf::Message& message) {
		return GetMessageId(message.GetDescriptor());
	}

	std::shared_ptr<google::protobuf::Message> ProtocolHelper::CreateMessage(const uint32_t packet_id) {
		auto* descriptor = GetDescriptor(packet_id);
		if (descriptor == nullptr) {
			LOG_ERROR("cannot get descriptor packet_id: {}", packet_id);
			return nullptr;
		}
		const auto* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
		return std::shared_ptr<google::protobuf::Message>(prototype->New());
	}

	const google::protobuf::Descriptor* ProtocolHelper::GetDescriptor(int32_t messageId) {
		auto& instance = Impl::GetInstance();
		return instance.GetDescriptor(messageId);
	}

	uint32_t ProtocolHelper::GetMessageId(const google::protobuf::Descriptor* descriptor) {
		return static_cast<uint32_t>(descriptor->options().GetExtension(type::message_id));
	}

	void ProtocolHelper::RegisterDescriptor(int32_t messageId, const google::protobuf::Descriptor* descriptor) {
		Impl::GetInstance().AddDescriptor(messageId, descriptor);
	}

	google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint) {
		return google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(timepoint.ToMicroseconds());
	}

	System::Time FromTimestamp(const google::protobuf::Timestamp& timestamp) {
		return System::Time::FromMicroseconds(google::protobuf::util::TimeUtil::TimestampToMicroseconds(timestamp));
	}
}