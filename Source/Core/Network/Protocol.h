#pragma once

#include "Core/Network/Packet/Internal.h"
#include "Core/Container/SPSC.h"

namespace Network {
	class Session;
	class Protocol {
	public:
		virtual ~Protocol() = default;
		virtual bool ProcessReceiveData(const PacketSegment& segment);
		virtual bool ProcessMessage(Session& session);

	protected:
		Container::SPSCQueue<InternalPacket> internal_messages_;
	};
}
