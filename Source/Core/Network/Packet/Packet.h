#pragma once

namespace Network {
	struct PacketHeader {
		static const PacketHeader* Peek(const char* data) {
			return reinterpret_cast<const PacketHeader*>(data);
		}

		static constexpr size_t Size() {
			return sizeof(PacketHeader);
		}

		uint64_t id;
		size_t size;
	};

	struct PacketSegment {
		const void* data;
		const size_t length;

		const PacketHeader& header() const {
			return *reinterpret_cast<const PacketHeader*>(data);
		}

		const char* body() const {
			return reinterpret_cast<const char*>(data) + sizeof(PacketHeader);
		}

		size_t body_length() const {
			return header().size;
		}
	};

	template<typename T, typename TPacket>
	concept PacketSerializer = requires(T serializer, const TPacket & packet, const size_t & packet_id, const void* data, const size_t& data_size, void* out_buffer, int32_t buffer_size) {
		{ T::Serialize(packet, &out_buffer, buffer_size) } -> std::same_as<bool>;
		{ T::Deserialize(packet_id, data, data_size) } -> std::same_as<std::shared_ptr<TPacket>>;
		{ T::Resolve(packet) } -> std::same_as<size_t>;
		{ T::IsValid(packet_id) } -> std::same_as<bool>;
	};
}
