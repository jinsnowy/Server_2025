#pragma once

#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/OutputStream.h"

namespace Network {

	struct PendingStream {
		PacketHeader header;
		std::vector<char> dataBuffer;

		void AppendData(const char* data, size_t size) {
			dataBuffer.resize(dataBuffer.size() + size);
			std::memcpy(dataBuffer.data() + dataBuffer.size() - size, data, size);
		}
	};

	struct RecvNetworkStream {
		std::unique_ptr<Buffer> buffer;
		std::optional<PendingStream> pending;
	};

	struct SendNetworkStream {
		std::atomic<bool> is_sending = false;
		std::list<BufferView> pending_buffers;
	};
}
