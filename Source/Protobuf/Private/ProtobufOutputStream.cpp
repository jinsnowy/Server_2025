#include "stdafx.h"
#include "Protobuf/Public/ProtobufOutputStream.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/NetworkStream.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/BufferPool.h"

namespace Protobuf {
	ProtobufOutputStream::ProtobufOutputStream(Network::OutputStream& output_stream)
		:
		output_stream_(output_stream) {
	}

	bool ProtobufOutputStream::Next(void** data, int* size) {
		return output_stream_.Next(data, size);
	}

	void ProtobufOutputStream::BackUp(int count) {
		output_stream_.BackUp(count);
	}

	int64_t ProtobufOutputStream::ByteCount() const {
		return output_stream_.ByteCount();
	}

	bool ProtobufOutputStream::WriteAliasedRaw(const void* data, int32_t size) {
		return output_stream_.WriteRaw(data, size);
	}

	bool ProtobufOutputStream::AllowsAliasing() const {
		return true;
	}
	
};