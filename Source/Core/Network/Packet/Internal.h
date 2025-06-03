#pragma once

#include "Core/Network/Packet/Packet.h"

namespace Network {
	enum class InternalPacketId : uint32_t
	{
		kHello = 0,
		kMessage,
		kPing,
		kPong,
		kCount,
	};

#define INTERNAL_PACKET_ID(id) static_cast<uint32_t>(InternalPacketId::id)

	struct InternalMessage {
		std::string_view message;

		PacketHeader header() const {
			return PacketHeader{
				.id = INTERNAL_PACKET_ID(kMessage),
				.size = length()
			};
		}
		const void* data() const {
			return message.data();
		}
		const uint32_t length() const {
			return static_cast<uint32_t>(message.length());
		}
	};

	struct InternalPing {
		int32_t ping_id;
		System::DateTime send_time;

		PacketHeader header() const {
			return PacketHeader{
				.id = INTERNAL_PACKET_ID(kPing),
				.size = length()
			};
		}
		const void* data() const {
			return reinterpret_cast<const void*>(this);
		}

		const uint32_t length() const {
			return sizeof(InternalPing);
		}
	};

	struct InternalPong {
		int32_t ping_id;
		System::DateTime send_time;

		PacketHeader header() const {
			return PacketHeader{
				.id = INTERNAL_PACKET_ID(kPong),
				.size = length()
			};
		}
	
		const void* data() const {
			return reinterpret_cast<const void*>(this);
		}

		const uint32_t length() const {
			return sizeof(InternalPong);
		}
	};

	using InternalPacket = std::variant<InternalMessage, InternalPing, InternalPong>;

#undef INTERNAL_PACKET_ID
}



