#pragma once

#include "Core/ThirdParty/Protobuf.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"

namespace Network {
	class Buffer;
	class OutputStream : public google::protobuf::io::ZeroCopyOutputStream {
	public:
		OutputStream();

		bool Next(void** data, int* size) override;
		void BackUp(int count) override;
		int64_t ByteCount() const override;

		bool WriteAliasedRaw(const void* Data, int32_t Size) override;
		bool AllowsAliasing() const override;

		int32_t RemainingByteCount() const;

		bool WriteRaw(const void* Data, int64_t Size);

		Buffer& GetBuffer() {
			return buffer_;
		}

		const Buffer& GetBuffer() const {
			return buffer_;
		}
		
		void SetBuffer(Buffer&& buffer) {
			buffer_ = std::move(buffer);
		}

		std::optional<BufferView> Flush();

		size_t GetPendingBufferCount() const {
			return pending_buffers_.size();
		}

	private:
		Buffer buffer_;
		std::list<Buffer> pending_buffers_;
	};

	class StreamWriter {
	public:
		StreamWriter(OutputStream& output_stream)
			: 
			output_stream_(output_stream) {
		}

		bool WriteMessage(const size_t message_id, const void* data, const size_t size);
		bool WriteMessage(const size_t message_id, const google::protobuf::Message& message);

		std::optional<BufferView> Flush() {
			return output_stream_.Flush();
		}

	private:
		OutputStream& output_stream_;

		void AssureWriteCapcity(const PacketHeader& header);
	};
}

