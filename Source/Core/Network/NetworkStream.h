#pragma once

#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"

namespace Network {
	struct SendNetworkStream {
		bool is_sending = false;
		std::list<Buffer> buffers;
	};

	struct PendingStream {
		PacketHeader header;
		std::vector<char> packetBuffer;
		uint32_t remainSegmentLength;
	};

	struct RecvNetworkStream {
		Buffer buffer;
		std::optional<PendingStream> pending;
	};
}
