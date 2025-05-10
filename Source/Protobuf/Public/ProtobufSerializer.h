#pragma once

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

namespace Protobuf {
	struct ProtobufSerializer {
		static std::vector<char> Serialize(const google::protobuf::Message& packet);
		static std::shared_ptr<google::protobuf::Message> Deserialize(const size_t& packetId, const Network::PacketSegment& segment);
		static uint32_t Resolve(const google::protobuf::Message& packet);
		static bool IsValid(const size_t& packetId);
	};
}