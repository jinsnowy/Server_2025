#pragma once

namespace Network {
	class Buffer {
	public:
		static constexpr int32_t kDefault = 4096;

		Buffer();
		Buffer(int32_t alloc_size);
		Buffer(std::shared_ptr<char[]> buffer, int32_t offset, int32_t size);
		
		void Allocate(int32_t alloc_size);

		std::shared_ptr<char[]> buffer_shared() const { return buffer_; }

		char* data() { return buffer_.get(); }
		const char* data() const { return buffer_.get(); }

		int32_t size() const { return size_; }
		int32_t offset() const { return offset_; }
		void set_offset(int32_t offset) { offset_ = offset; }

	private:
		std::shared_ptr<char[]> buffer_;
		int32_t offset_;
		int32_t size_;
	};
}
