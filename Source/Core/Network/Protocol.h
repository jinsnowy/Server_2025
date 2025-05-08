#pragma once

namespace Network {
	class Session;
	class Protocol {
	public:
		virtual ~Protocol() = default;
		virtual bool ProcessMessage(Session& session, const size_t& packetId, const PacketSegment& segment) = 0;
	};
}
