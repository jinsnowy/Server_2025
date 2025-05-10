#include "stdafx.h"
#include "Protobuf/Public/ProtobufSession.h"
#include "Protobuf/Public/ProtobufSerializer.h"
#include "Protobuf/Public/ProtobufProtocol.h"
#include "Core/Network/Packet/Packet.h"

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
	
	ProtobufSession::ProtobufSession(std::shared_ptr<Network::Connection> connection)
		:
		Network::Session(std::move(connection)) {
	}

	void ProtobufSession::OnConnected() {
		Network::Session::OnConnected();
	}

	void ProtobufSession::Send(const google::protobuf::Message& message) {
		size_t message_id = ProtobufSerializer::Resolve(message);
		Post([serialized_string = message.SerializeAsString(), message_id](ProtobufSession& session) { // google::protobuf::Message 16bytes
			auto header = Network::PacketHeader{
				.id = message_id,
				.size = serialized_string.size()
			};
			auto outputStream = ZeroCopyOutputStream(session.GetOutputStream());
			outputStream.WriteHeader(header);
			if (serialized_string.size() > 0) {
				if (outputStream.WriteAliasedRaw(serialized_string.data(), serialized_string.size()) == false) {
					LOG_ERROR("[SESSION] Send: WriteAliasedRaw failed");
					return;
				}
			}
		
			session.FlushSend();
		});
	}

	void ProtobufSession::Send(const std::shared_ptr<const google::protobuf::Message>& message) {
		Post([message](ProtobufSession& session) {
			auto header = Network::PacketHeader{
				.id = ProtobufSerializer::Resolve(*message),
				.size = message->ByteSizeLong()
			};
			
			auto outputStream = ZeroCopyOutputStream(session.GetOutputStream());
			outputStream.WriteHeader(header);
			if (message->SerializeToZeroCopyStream(&outputStream) == false) {
				LOG_ERROR("[SESSION] Send: SerializeToZeroCopyStream failed");
				return;
			}
			session.FlushSend();
		});
	}
}

