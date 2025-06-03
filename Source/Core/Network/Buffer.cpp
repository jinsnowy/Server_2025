#include "stdafx.h"
#include "Buffer.h"

namespace Network {

	Buffer::Buffer(std::shared_ptr<BufferMemory> source)
		:
		source_(source),
		start_pos_(0),
		end_pos_(0)
	{
	}

	Buffer::~Buffer()
	{
	}

	BufferView Buffer::AsView() const {
		return BufferView(source_, start_pos_, GetByteCount());
	}
}
