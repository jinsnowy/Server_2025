#pragma once

#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/OutputStream.h"
#include "Core/Container/MPSC.h"

namespace Network {
	class SendNode;
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

		RecvNetworkStream();
	};

	struct SendNetworkStream {
		Container::MPSCQueue<std::unique_ptr<SendNode>> pending_buffers;
		~SendNetworkStream();
	};
}
