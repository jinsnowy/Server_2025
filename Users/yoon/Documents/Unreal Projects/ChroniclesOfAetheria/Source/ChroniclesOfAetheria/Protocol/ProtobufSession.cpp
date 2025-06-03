#include "stdafx.h"
#include "Protobuf/Public/ProtobufSession.h"
#include "Protobuf/Public/ProtobufSerializer.h"
#include "Protobuf/Public/ProtobufProtocol.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/OutputStream.h"

namespace Protobuf {
	struct ZeroCopyOutputStream  : public google::protobuf::io::ZeroCopyOutputStream {
		ZeroCopyOutputStream(Network::OutputStream stream)
			:
			stream_(stream) {
		}

		bool Next(void** data, int32_t* size) override {
			return stream_.Next(data, size);
		}

		void BackUp(int32_t count) override {
			stream_.BackUp(count);
		}

		int64_t ByteCount() const override {
			return stream_.ByteCount();
		}

		bool WriteAliasedRaw(const void* data, int32_t size) override {
			return stream_.WriteAliasedRaw(data, size);
		}

		bool AllowsAliasing() const {
			return stream_.AllowsAliasing();
		}

		void WriteHeader(const Network::PacketHeader& header) {
			stream_.WriteAliasedRaw(&header, sizeof(Network::PacketHeader));
		}

		Network::OutputStream stream_;
	};

	ProtobufSession::ProtobufSession(const std::shared_ptr<System::Context>& context)
		:
		Network::Session(context) {
	}
	
	ProtobufSession::ProtobufSession()
		:
		Network::Session() {
	}

	void ProtobufSession::OnConnected() {
		Network::Session::OnConnected();
	}

	void ProtobufSession::Send(const google::protobuf::Message& message) {
		size_t message_id = ProtobufSerializer::Resolve(message);
		size_t message_size = message.ByteSizeLong();

		Network::PacketHeader header = Network::PacketHeader{
			.id = message_id,
			.size = message_size
		};

		Network::Buffer buffer(message_size + sizeof(Network::PacketHeader));
		Network::BufferWriter writer(buffer);
		writer.Write(&header, sizeof(Network::PacketHeader));
		if (message.SerializeToArray(writer.data_ptr(), writer.remaining_size()) == false) {
			LOG_ERROR("[SESSION] Send: SerializeToArray failed");
			return;
		}
		Network::Session::Send(buffer);
	}

	void ProtobufSession::Send(const std::shared_ptr<const google::protobuf::Message>& message) {
		Send(*message);
	}
}

