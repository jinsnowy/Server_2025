#pragma once

namespace Network {
	struct PacketHeader {
		static const PacketHeader* Peek(const char* data) {
			return reinterpret_cast<const PacketHeader*>(data);
		}
		static constexpr size_t Size() {
			return sizeof(PacketHeader);
		}
		static constexpr size_t kMaxSize = 1 << 24; // 64MB

		size_t id;
		uint32_t size;
	};

	struct PacketSegment {
		const void* data;
		const uint32_t length;

		const PacketHeader& header() const {
			return *reinterpret_cast<const PacketHeader*>(data);
		}

		const char* body() const {
			return reinterpret_cast<const char*>(data) + sizeof(PacketHeader);
		}

		uint32_t body_length() const {
			return header().size;
		}
	};

	template<typename T, typename TPacket>
	concept PacketSerializer = requires(T serializer, const TPacket & packet, const size_t & packet_id, const PacketSegment & segment) {
		{ T::Serialize(packet) } -> std::same_as<std::vector<char>>;
		{ T::Deserialize(packet_id, segment) } -> std::same_as<std::shared_ptr<TPacket>>;
		{ T::Resolve(packet) } -> std::same_as<size_t>;
		{ T::IsValid(packet_id) } -> std::same_as<bool>;
	};
}
