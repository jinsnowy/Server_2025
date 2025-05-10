#include "stdafx.h"
#include "Buffer.h"

namespace Network {
	Buffer::Buffer()
		:
		buffer_(nullptr),
		start_pos_(0),
		end_pos_(0),
		size_(0)
	{
	}

	Buffer::Buffer(int32_t alloc_size)
		:
		buffer_(new char[alloc_size]),
		start_pos_(0),
		end_pos_(0),
		size_(alloc_size)
	{
	}

	void Buffer::Allocate(int32_t alloc_size) {
		buffer_.reset(new char[alloc_size]());
		start_pos_= 0;
		end_pos_ = 0;
		size_ = alloc_size;
	}

}
