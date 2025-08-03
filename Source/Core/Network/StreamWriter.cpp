#include "stdafx.h"
#include "StreamWriter.h"
#include "Core/Network/OutputStream.h"

namespace Network {
	bool StreamWriter::WriteMessage(const uint64_t message_id, const void* data, const size_t size) {
		PacketHeader header = { .id = message_id, .size = size };
		if (!output_stream_.WriteRaw(&header, sizeof(PacketHeader))) {
			return false;
		}

		if (size > 0 && !output_stream_.WriteRaw(data, static_cast<int64_t>(size))) {
			return false;
		}

		return true;
	}
}