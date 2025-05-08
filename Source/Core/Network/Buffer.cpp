#include "stdafx.h"
#include "Buffer.h"

namespace Network {
	Buffer::Buffer()
		:
		buffer_(nullptr),
		offset_(0),
		size_(0)
	{
	}

	Buffer::Buffer(int32_t alloc_size)
		:
		buffer_(new char[alloc_size]),
		offset_(0),
		size_(alloc_size)
	{
	}

	Buffer::Buffer(std::shared_ptr<char[]> buffer, int32_t offset, int32_t size)
		:
		buffer_(buffer),
		offset_(offset),
		size_(size)
	{
	}

	void Buffer::Allocate(int32_t alloc_size) {
		buffer_.reset(new char[alloc_size]());
		offset_ = 0;
		size_ = alloc_size;
	}

}
