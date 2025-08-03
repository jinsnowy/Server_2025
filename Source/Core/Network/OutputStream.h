#pragma once

#include "Core/ThirdParty/Protobuf.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/Container/MPSC.h"

namespace Network {
	class Buffer;
	class BufferView;
	class OutputStream {
	public:
		OutputStream();
		~OutputStream();

		bool Next(void** data, int* size);
		void BackUp(int count);
		int64_t ByteCount() const;
		bool WriteRaw(const void* data, int64_t size);
		std::optional<BufferView> NextBuffer();
		void Clear();
		size_t GetBufferCount() const {
			return buffers_.size();
		}

	private:
		int64_t bytes_count_ = 0;
		std::deque<std::shared_ptr<Buffer>> buffers_;
	};
}

