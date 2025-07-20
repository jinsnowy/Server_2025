#include "stdafx.h"
#include "OutputStream.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/BufferPool.h"

namespace Network {

	OutputStream::OutputStream()
		: 
		buffer_(RequestSendBuffer()) {
	}

	bool OutputStream::Next(void** data, int* size) {
		if (buffer_.GetRemainingByteCount() <= 0) {
			RotateBuffer(RequestSendBuffer());
		}

		*data = buffer_.GetFreePtr();
		*size = static_cast<int32_t>(buffer_.GetRemainingByteCount());
		buffer_.set_end_pos(buffer_.end_pos() + *size);

		return true;
	}

	void OutputStream::BackUp(int count) {
		if (buffer_.GetByteCount() < count) {
			LOG_ERROR("[OutputStream] BackUp: Not enough data to back up");
			return;
		}

		buffer_.set_end_pos(buffer_.end_pos() - count);
	}

	int64_t OutputStream::ByteCount() const {
		return buffer_.GetByteCount();
	}

	size_t OutputStream::RemainingByteCount() const {
		return buffer_.GetRemainingByteCount();
	}

	bool OutputStream::WriteAliasedRaw(const void* Data, int32_t Size) {
		return WriteRaw(Data, Size);
	}

	bool OutputStream::WriteRaw(const void* data, int64_t size) {
		void* buffer_out = nullptr;
		int32_t buffer_size = 0;
		int32_t offset = 0;

		while (size > 0) {
			if (Next(&buffer_out, &buffer_size) == false) {
				return false;
			}

			const int32_t write_size = static_cast<int32_t>(std::min(static_cast<int64_t>(buffer_size), size));
			std::memcpy(buffer_out, reinterpret_cast<const char*>(data) + offset, write_size);

			int32_t remain_size = buffer_size - write_size;
			if (remain_size > 0) {
				BackUp(remain_size);
			}

			offset += write_size;
			size -= write_size;
		}

		return true;
	}

	std::optional<BufferView> OutputStream::Flush() {
		if (pending_buffers_.empty() == false) {
			Buffer front = pending_buffers_.front();
			pending_buffers_.pop_front();
			return front.AsView();
		}
		if (buffer_.GetByteCount() > 0) {
			auto buffer_view = buffer_.AsView();
			buffer_.set_start_pos(buffer_.end_pos());
			return buffer_view;
		}
		return std::nullopt;
	}

	void OutputStream::RotateBuffer(Buffer&& new_buffer) {
		if (buffer_.GetByteCount() > 0) {
			pending_buffers_.push_back(std::move(buffer_));
		}
		buffer_ = std::move(new_buffer);
	}

	bool OutputStream::AllowsAliasing() const {
		return true;
	}

	bool StreamWriter::WriteMessage(const size_t message_id, const void* data, const size_t size) {
		PacketHeader header = {.id = message_id, .size = size};
		AssureWriteCapcity(sizeof(PacketHeader));

		if (!output_stream_.WriteRaw(&header, sizeof(PacketHeader))) {
			return false;
		}

		if (size > 0 && !output_stream_.WriteRaw(data, static_cast<int64_t>(size))) {
			return false;
		}

		return true;
	}

	bool StreamWriter::WriteMessage(const size_t message_id, const google::protobuf::Message& message) {
		size_t size = message.ByteSizeLong();
		PacketHeader header = { .id = message_id, .size = size };
		AssureWriteCapcity(sizeof(PacketHeader));

		if (!output_stream_.WriteRaw(&header, sizeof(PacketHeader))) {
			return false;
		}

		if (size > 0 && !message.SerializeToZeroCopyStream(&output_stream_)) {
			return false;
		}

		return true;
	}

	void StreamWriter::AssureWriteCapcity(size_t size) {
		if (output_stream_.RemainingByteCount() < size) {
			output_stream_.RotateBuffer(RequestSendBuffer());
		}
	}
};