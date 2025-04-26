#pragma once

#include "Core/Network/Packet/Packet.h"

namespace Network {
	class StreamWriter;

	struct PendingSendStream {
		bool headerWritten = false;
		PacketHeader header;
		PacketSegment segment;
		uint32_t remainSegmentLength;
	};

	struct PendingRecvStream {
		PacketHeader header;
		std::vector<char> packetBuffer;
		uint32_t remainSegmentLength;
	};
}
