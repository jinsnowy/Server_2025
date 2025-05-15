#include "stdafx.h"
#include "OutputStream.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"

namespace Network {

	OutputStream::OutputStream(SendNetworkStream& send_stream)
		:
		send_stream_(&send_stream)
	{
	}

	bool OutputStream::Next(void** data, int32_t* size) {
		if (send_stream_->buffers.empty()) {
			send_stream_->buffers.emplace_back(Buffer(Buffer::kDefault));
		}

		Buffer* send_buffer = &send_stream_->buffers.back();
		int32_t remainSize = send_buffer->size() - send_buffer->end_pos();
		if (remainSize == 0) {
			auto& last = send_stream_->buffers.emplace_back(Buffer(Buffer::kDefault));
			send_buffer = &last;
			remainSize = Buffer::kDefault;
		}

		*data = (send_buffer->data() + send_buffer->end_pos());
		*size = remainSize;
		send_buffer->set_end_pos(send_buffer->end_pos() + remainSize);

		return true;
	}

	void OutputStream::BackUp(int32_t count) {
		auto& last = send_stream_->buffers.back();
		last.set_end_pos(last.end_pos() - count);
	}

	int64_t OutputStream::ByteCount() const {
		if (send_stream_->buffers.empty()) {
			return 0;
		}
		return send_stream_->buffers.back().end_pos() - send_stream_->buffers.back().start_pos();
	}

	bool OutputStream::WriteAliasedRaw(const void* data, int32_t size) {
		if (size == 0) {
			return false;
		}
		if (size >= Buffer::kDefault) {
			send_stream_->buffers.emplace_back(Buffer(size));
		}

		void* alloc_buffer = nullptr;
		int32_t alloc_size = 0;
		int32_t offset = 0;
		while (size > 0) {
			if (Next(&alloc_buffer, &alloc_size) == false) {
				return false;
			}

			const int32_t write_size = std::min(alloc_size, size);
			memcpy_s(alloc_buffer, write_size, reinterpret_cast<const char*>(data) + offset, size);

			int32_t remain_size = alloc_size - write_size;
			if (remain_size > 0) {
				BackUp(remain_size);
			}

			offset += write_size;
			size -= write_size;
		}

		return true;
	}

	bool OutputStream::AllowsAliasing() const {
		return true;
	}

};