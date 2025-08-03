#pragma once


#include "Core/Network/SendNode.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

namespace Protobuf {
	class ProtobufNode : public Network::SendNode { 
	public:
		ProtobufNode(uint64_t message_id, const std::shared_ptr<const google::protobuf::Message>& message);
		~ProtobufNode();

		bool SerializeTo(Network::OutputStream& output_stream) override;

		uint64_t GetMessageId() const override {
			return message_id_;
		}
		
		size_t GetSize() const override;

	private:
		uint64_t message_id_;
		std::shared_ptr<const google::protobuf::Message> message_;
	};
}

