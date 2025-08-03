#include "stdafx.h"
#include "SendNode.h"
#include "Core/Network/OutputStream.h"
#include "Core/Network/StreamWriter.h"

namespace Network {
	bool RawNode::SerializeTo(OutputStream& output_stream) {
		StreamWriter writer(output_stream);
		return writer.WriteMessage(message_id_, data_.data(), data_.size());
	}
} // namespace Network

