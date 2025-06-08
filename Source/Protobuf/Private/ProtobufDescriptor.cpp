#include "stdafx.h"
#include "Protobuf/Public/ProtobufDescriptor.h"
#include "Protobuf/Public/Types.h"

namespace Protobuf {
	class MessageDescriptorStorage : public System::Singleton<MessageDescriptorStorage> {
	public:
		std::unordered_map<size_t, const google::protobuf::Descriptor*> _descriptors;

		void AddDescriptor(size_t message_id, const google::protobuf::Descriptor* descriptor) {
			_descriptors.try_emplace(message_id, descriptor);
		}

		const google::protobuf::Descriptor* GetDescriptor(size_t message_id) {
			if (auto iter = _descriptors.find(message_id); iter != _descriptors.end()) {
				return iter->second;
			}
			return nullptr;
		}
	};

	size_t ProtobufDescriptor::GetMessageId(const google::protobuf::Message& message) {
		return GetMessageId(message.GetDescriptor());
	}

	std::shared_ptr<google::protobuf::Message> ProtobufDescriptor::CreateMessage(const size_t packet_id) {
		auto* descriptor = GetDescriptor(packet_id);
		if (descriptor == nullptr) {
			LOG_ERROR("cannot get descriptor packet_id: {}", packet_id);
			return nullptr;
		}
		const auto* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		return std::shared_ptr<google::protobuf::Message>(prototype->New());
	}

	const google::protobuf::Descriptor* ProtobufDescriptor::GetDescriptor(size_t messageId) {
		auto& instance = MessageDescriptorStorage::GetInstance();
		return instance.GetDescriptor(messageId);
	}

	size_t ProtobufDescriptor::GetMessageId(const google::protobuf::Descriptor* descriptor) {
		return descriptor->options().GetExtension(types::message_id);
	}

	void ProtobufDescriptor::RegisterDescriptor(size_t messageId, const google::protobuf::Descriptor* descriptor) {
		MessageDescriptorStorage::GetInstance().AddDescriptor(messageId, descriptor);
	}

	//google::protobuf::Timestamp ToTimestamp(const System::Time& timepoint) {
	//	return google::protobuf::util::TimeUtil::MicrosecondsToTimestamp(timepoint.ToMicroseconds());
	//}

	//System::Time FromTimestamp(const google::protobuf::Timestamp& timestamp) {
	//	return System::Time::FromMicroseconds(google::protobuf::util::TimeUtil::TimestampToMicroseconds(timestamp));
	//}
}