#pragma once

namespace Network {
	class Protocol {
	public:
		virtual ~Protocol() = default;
		virtual bool ProcessMessage(const size_t& packetId, const PacketSegment& segment) = 0;
	};
}
