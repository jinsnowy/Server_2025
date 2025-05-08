#pragma once


namespace RTS {
	struct ProtoSerializer {
		static std::vector<char> Serialize(const google::protobuf::Message& packet);
		static std::shared_ptr<google::protobuf::Message> Deserialize(const size_t& packetId, const Network::PacketSegment& segment);
		static uint32_t Resolve(const google::protobuf::Message& packet);
		static bool IsValid(const size_t& packetId);
	};
}