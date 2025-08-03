#pragma once

namespace Network {
	class OutputStream;
	class StreamWriter final {
	public:
		StreamWriter(OutputStream& output_stream)
			:
			output_stream_(output_stream) {
		}

		bool WriteMessage(const uint64_t message_id, const void* data, const size_t size);

	private:
		OutputStream& output_stream_;
	};
}

