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

	std::string Buffer::ToString() const {
		return FORMAT("Buffer(start_pos: {}, end_pos: {}, size: {})",
			start_pos_, end_pos_, source_->GetBufferSize());
	}
}
