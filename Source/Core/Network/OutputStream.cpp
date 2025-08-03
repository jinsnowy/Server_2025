#include "stdafx.h"
#include "OutputStream.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/BufferPool.h"
#include "Core/Network/Buffer.h"

namespace Network {

	OutputStream::OutputStream() = default;
	OutputStream::~OutputStream() = default;

	bool OutputStream::Next(void** data, int* size) {
		if (buffers_.empty() || buffers_.back()->GetRemainingByteCount() <= 0) {
			buffers_.emplace_back(std::make_shared<Buffer>(RequestSendBuffer()));
		}

		Buffer& buffer = *buffers_.back();

		*data = buffer.GetFreePtr();
		*size = static_cast<int32_t>(buffer.GetRemainingByteCount());
		buffer.set_end_pos(buffer.end_pos() + *size);

		return true;
	}

	void OutputStream::BackUp(int count) {
		Buffer& buffer_ = *buffers_.back();
		if (buffer_.GetByteCount() < count) {
			RELEASE_ASSERT(false && "[OutputStream] BackUp: Not enough data to back up");
			return;
		}

		buffer_.set_end_pos(buffer_.end_pos() - count);
	}

	int64_t OutputStream::ByteCount() const {
		return bytes_count_;
	}

	bool OutputStream::WriteRaw(const void* data, int64_t size) {
		void* buffer_out = nullptr;
		int32_t buffer_size = 0;
		bytes_count_ = 0;

		while (size > 0) {
			if (Next(&buffer_out, &buffer_size) == false) {
				return false;
			}

			const int32_t write_size = static_cast<int32_t>(std::min(static_cast<int64_t>(buffer_size), size));
			std::memcpy(buffer_out, reinterpret_cast<const char*>(data) + bytes_count_, write_size);

			bytes_count_ += write_size;
			size -= write_size;

			int32_t remain_size = buffer_size - write_size;
			if (remain_size > 0) {
				BackUp(remain_size);
			}
		}

		return true;
	}

	std::optional<BufferView> OutputStream::NextBuffer() {
		while (!buffers_.empty()) {
			auto& front = buffers_.front();
			if (front->GetByteCount() == 0 && front->GetRemainingByteCount() == 0) {
				buffers_.pop_front();
			}
			else {
				break;
			}
		}
		if (buffers_.empty()) {
			return std::nullopt; // No data to flush
		}
		auto& front = buffers_.front();
		if (front->GetByteCount() == 0) {
			return std::nullopt; // No data to flush
		}
		return BufferView(front, front->start_pos(), front->GetByteCount());
	}

	void OutputStream::Clear() {
		buffers_.clear();
	}

};