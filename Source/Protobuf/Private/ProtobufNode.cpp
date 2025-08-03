#include "stdafx.h"
#include "Protobuf/Public/ProtobufNode.h"
#include "Protobuf/Public/ProtobufOutputStream.h"
#include "Core/Network/OutputStream.h"

namespace Protobuf {
	ProtobufNode::ProtobufNode(uint64_t message_id, const std::shared_ptr<const google::protobuf::Message>& message)
		:
		message_id_(message_id),
		message_(message)
	{
	}

	ProtobufNode::~ProtobufNode() = default;

	bool ProtobufNode::SerializeTo(Network::OutputStream& output_stream) {
		ProtobufOutputStream protobuf_output_stream(output_stream);

		Network::PacketHeader header(message_id_, message_->ByteSizeLong());
		if (protobuf_output_stream.WriteAliasedRaw(&header, sizeof(header)) == false) {
			return false;
		}

		return header.size > 0 && message_->SerializeToZeroCopyStream(&protobuf_output_stream);
	}

	size_t ProtobufNode::GetSize() const {
		return message_->ByteSizeLong();
	}
}