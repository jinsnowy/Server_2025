#pragma 

#include "Core/Network/Packet/Packet.h"

namespace Network {
	enum InternalPacketId : uint32_t
	{
		kHello = 0,
		kMessage,
		kPing,
		kPong,
		kCount,
	}

	struct InternalMessage {
		std::string_view message;

		PacketHeader header() const {
			return PacketHeader{
				.id = InternalPacketId::kMessage,
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
				.id = InternalPacketId::kPing,
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
				.id = InternalPacketId::kPing,
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
}


