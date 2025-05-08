#include "stdafx.h"
#include "ProtobufDescriptor.h"

namespace RTS {
	class MessageDescriptorStorage : public System::Singleton<MessageDescriptorStorage> {
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

	uint32_t ProtobufDescriptor::GetMessageId(const google::protobuf::Message& message) {
		return GetMessageId(message.GetDescriptor());
	}

	std::shared_ptr<google::protobuf::Message> ProtobufDescriptor::CreateMessage(const uint32_t packet_id) {
		auto* descriptor = GetDescriptor(packet_id);
		if (descriptor == nullptr) {
			LOG_ERROR("cannot get descriptor packet_id: {}", packet_id);
			return nullptr;
		}
		const auto* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		return std::shared_ptr<google::protobuf::Message>(prototype->New());
	}

	const google::protobuf::Descriptor* ProtobufDescriptor::GetDescriptor(int32_t messageId) {
		auto& instance = MessageDescriptorStorage::GetInstance();
		return instance.GetDescriptor(messageId);
	}

	uint32_t ProtobufDescriptor::GetMessageId(const google::protobuf::Descriptor* descriptor) {
		return static_cast<uint32_t>(descriptor->options().GetExtension(type::message_id));
	}

	void ProtobufDescriptor::RegisterDescriptor(int32_t messageId, const google::protobuf::Descriptor* descriptor) {
		MessageDescriptorStorage::GetInstance().AddDescriptor(messageId, descriptor);
	}

	//google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint) {
	//	return google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(timepoint.ToMicroseconds());
	//}

	//System::Time FromTimestamp(const google::protobuf::Timestamp& timestamp) {
	//	return System::Time::FromMicroseconds(google::protobuf::util::TimeUtil::TimestampToMicroseconds(timestamp));
	//}
}