#pragma once

namespace Network {
	class Buffer {
	public:
		static constexpr int32_t kDefault = 4096;

		Buffer();
		Buffer(int32_t alloc_size);
		
		void Allocate(int32_t alloc_size);

		std::shared_ptr<char[]> buffer_shared() const { return buffer_; }

		char* data() { return buffer_.get(); }
		const char* data() const { return buffer_.get(); }

		int32_t size() const { return size_; }

		int32_t start_pos() const { return start_pos_; }
		void set_start_pos(int32_t offset) { start_pos_ = offset; }

		int32_t end_pos() const { return end_pos_; }
		void set_end_pos(int32_t offset) { end_pos_ = offset; }

		int32_t GetByteCount() const { return end_pos_ - start_pos_; }
		int32_t GetRemainingByteCount() const { return size_ - end_pos_; }

		bool IsEmpty() const { return end_pos_ == start_pos_; }

	private:
		std::shared_ptr<char[]> buffer_;
		int32_t start_pos_;
		int32_t end_pos_;
		int32_t size_;
	};

	class BufferWriter {
	public:
		BufferWriter(Network::Buffer& buffer)
			:
			buffer_(buffer) {
		}

		void Write(const void* data, size_t size) {
			::memcpy_s(buffer_.data() + buffer_.end_pos(), buffer_.size() - buffer_.end_pos(), data, size);
			buffer_.set_end_pos(buffer_.end_pos() + size);
		}

		void* data_ptr() {
			return buffer_.data() + buffer_.end_pos();
		}

		size_t remaining_size() const {
			return buffer_.GetRemainingByteCount();
		}

	private:
		Buffer& buffer_;
	};
}
