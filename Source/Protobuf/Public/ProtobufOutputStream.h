#pragma once

#include "Core/ThirdParty/Protobuf.h"
#include "Core/Network/Packet/Packet.h"

namespace Network {
	class OutputStream;
} // namespace Network

namespace Protobuf {
	class Buffer;
	class ProtobufOutputStream : public google::protobuf::io::ZeroCopyOutputStream {
	public:
		ProtobufOutputStream(Network::OutputStream& stream);

		bool Next(void** data, int* size) override;
		void BackUp(int count) override;
		int64_t ByteCount() const override;

		bool WriteAliasedRaw(const void* data, int32_t size) override;
		bool AllowsAliasing() const override;
	
	private:
		Network::OutputStream& output_stream_;
	};
}

