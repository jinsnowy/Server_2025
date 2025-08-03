#pragma once

#include "Core/Network/Buffer.h"

namespace Network {
	Buffer RequestSendBuffer();
	Buffer RequestRecvBuffer();

	Buffer RequestBuffer(size_t BufferSize);

	static constexpr size_t kRecvBufferChunkSize = 4096;
	static constexpr size_t kSendBufferChunkSize = 4096;
}
