#include "stdafx.h"
#include "Protobuf/Public/ProtobufSerializer.h"
#include "Protobuf/Public/ProtobufDescriptor.h"
#include "Protobuf/Public/Types.h"

namespace Protobuf {
	bool ProtobufSerializer::Serialize(const google::protobuf::Message& packet, void** out_buffer, int32_t buffer_size) {
		if (packet.SerializeToArray(out_buffer, buffer_size) == false) {
			LOG_ERROR("serialize protobuf failed: {}", packet.GetTypeName());
			return false;
		}
		return true;
	}

	std::shared_ptr<google::protobuf::Message> ProtobufSerializer::Deserialize(const size_t& packetId, const void* data, const size_t& data_size) {
		auto* descriptor = ProtobufDescriptor::GetDescriptor(packetId);
		if (descriptor == nullptr) {
			LOG_ERROR("cannot get descriptor packet_id: {}", packetId);
			return nullptr;
		}

		if (data_size > std::numeric_limits<int32_t>::max()) {
			return nullptr;
		}

		const auto* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		std::shared_ptr<google::protobuf::Message> message(prototype->New());
		if (message->ParseFromArray(data, static_cast<int32_t>(data_size)) == false) {
			LOG_ERROR("parse failed packet_id: {}, name: {}", packetId, prototype->GetTypeName());
			return nullptr;
		}
		return message;
	}

	uint32_t ProtobufSerializer::Resolve(const google::protobuf::Message& packet) {
		return packet.GetDescriptor()->options().GetExtension(types::message_id);
	}

	bool ProtobufSerializer::IsValid(const size_t& packetId) {
		return types::protocol_IsValid(static_cast<int32_t>(packetId));
	}
}