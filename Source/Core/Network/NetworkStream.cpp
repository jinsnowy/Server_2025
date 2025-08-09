#include "stdafx.h"
#include "NetworkStream.h"
#include "Core/Network/SendNode.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/BufferPool.h"

namespace Network {
	SendNetworkStream::~SendNetworkStream() = default;

	RecvNetworkStream::RecvNetworkStream()
		:
		buffer(new Buffer(RequestRecvBuffer())),
		pending(std::nullopt)
	{
	}
} // namespace Network