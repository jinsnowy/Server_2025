#pragma once

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

namespace Protobuf {
	struct ProtobufSerializer {
		static bool Serialize(const google::protobuf::Message& packet, void** out_buffer, int32_t buffer_size);
		static std::shared_ptr<google::protobuf::Message> Deserialize(const size_t& packetId, const void* data, const size_t& data_size);
		static uint32_t Resolve(const google::protobuf::Message& packet);
		static bool IsValid(const size_t& packetId);
	};
}