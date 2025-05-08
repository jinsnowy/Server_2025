#include "stdafx.h"
#include "ProtoSerializer.h"
#include "ProtobufDescriptor.h"

namespace RTS {
	std::vector<char> ProtoSerializer::Serialize(const google::protobuf::Message& packet) {
		std::vector<char> buffer(packet.ByteSizeLong());
		if (packet.SerializeToArray(buffer.data(), buffer.size()) == false) {
			LOG_ERROR("serialize protobuf failed: {}", packet.GetTypeName());
			return{};
		}

		return buffer;
	}

	std::shared_ptr<google::protobuf::Message> ProtoSerializer::Deserialize(const size_t& packetId, const Network::PacketSegment& segment) {
		auto* descriptor = ProtobufDescriptor::GetDescriptor(packetId);
		if (descriptor == nullptr) {
			LOG_ERROR("cannot get descriptor packet_id: {}", packetId);
			return nullptr;
		}

		const auto* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		std::shared_ptr<Message> message(prototype->New());
		if (message->ParseFromArray(segment.body(), segment.body_length()) == false) {
			LOG_ERROR("parse failed packet_id: {}, name: {}", packetId, prototype->GetTypeName());
			return nullptr;
		}
		return message;
	}

	uint32_t ProtoSerializer::Resolve(const google::protobuf::Message& packet) {
		return packet.GetDescriptor()->options().GetExtension(type::message_id);
	}

	bool ProtoSerializer::IsValid(const size_t& packetId) {
		return type::protocol_IsValid(packetId);
	}
}