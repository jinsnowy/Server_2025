#pragma once

#include "Core/Network/BufferMemory.h"

namespace Network {
	class BufferMemory;
	class BufferView;
	class Buffer {
	public:
		Buffer(std::shared_ptr<BufferMemory> source);
		~Buffer();
		
		std::shared_ptr<BufferMemory> source() const { return source_; }

		char* GetBufferPtr() { return source_->GetBufferPtr(); }
		const char* GetBufferPtr() const { return source_->GetBufferPtr(); }
		const char* GetDataPtr() const { return source_->GetBufferPtr() + start_pos_; }
		char* GetFreePtr() { return source_->GetBufferPtr() + end_pos_; }

		size_t GetBufferSize() const { return source_->GetBufferSize(); }

		size_t start_pos() const { return start_pos_; }
		void set_start_pos(size_t offset) { start_pos_ = offset; }

		size_t end_pos() const { return end_pos_; }
		void set_end_pos(size_t offset) { end_pos_ = offset; }

		size_t GetByteCount() const { return end_pos_ - start_pos_; }
		size_t GetRemainingByteCount() const { return GetBufferSize() - end_pos_; }

		bool IsEmpty() const { return end_pos_ == start_pos_; }
		void Clear() {
			start_pos_ = 0;
			end_pos_ = 0;
		}

		std::string ToString() const;

	private:
		std::shared_ptr<BufferMemory> source_;
		size_t start_pos_;
		size_t end_pos_;
	};

	class BufferView final {
	public:
		BufferView(const std::shared_ptr<Buffer>& buffer, size_t data_offset, size_t data_length)
			:
			buffer_(buffer),
			data_(buffer->GetBufferPtr() + data_offset),
			data_length_(data_length) {
		}

		const char* data() const {
			return data_;
		}

		size_t length() const {
			return data_length_;
		}

		const std::shared_ptr<Buffer>& buffer() const {
			return buffer_;
		}
	
	private:
		std::shared_ptr<Buffer> buffer_;
		const char* data_;
		const size_t data_length_;
	};

	class BufferWriter {
	public:
		BufferWriter(Network::Buffer& buffer)
			:
			buffer_(buffer) {
		}

		void Write(const void* data, size_t size) {
			::memcpy_s(buffer_.GetBufferPtr() + buffer_.end_pos(), buffer_.GetBufferSize() - buffer_.end_pos(), data, size);
			buffer_.set_end_pos(buffer_.end_pos() + size);
		}

		const void* GetFreePtr() const {
			return buffer_.GetBufferPtr() + buffer_.end_pos();
		}

		const void* GetDataPtr() const {
			return buffer_.GetBufferPtr() + buffer_.start_pos();
		}

		size_t GetRemainingSize() const {
			return buffer_.GetRemainingByteCount();
		}

	private:
		Buffer& buffer_;
	};
}
