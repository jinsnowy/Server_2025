#pragma once

#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/OutputStream.h"

namespace Network {

	struct PendingStream {
		std::optional<PacketHeader> header;
		std::vector<char> packetBuffer;
		uint32_t remainSegmentLength;
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
