#include "stdafx.h"
#include "Protobuf/Public/ProtobufSession.h"
#include "Protobuf/Public/ProtobufSerializer.h"
#include "Protobuf/Public/ProtobufProtocol.h"
#include "Core/Network/Packet/Packet.h"
#include "Core/Network/Buffer.h"
#include "Core/Network/OutputStream.h"

namespace Protobuf {
	

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
		if (IsSynchronized()) {
			SendInternal(message);
		}
		else {
			const size_t message_id = ProtobufSerializer::Resolve(message);
			Ctrl(*this).Post([message_id, priory_serialized = message.SerializeAsString()](ProtobufSession& session) {
				session.SendInternal(message_id, priory_serialized);
			});
		}
	}

	void ProtobufSession::Send(const std::shared_ptr<const google::protobuf::Message>& message) {
		if (IsSynchronized()) {
			SendInternal(*message);
		}
		else {
			Ctrl(*this).Post([message](ProtobufSession& session) {
				session.SendInternal(*message);
			});
		}
	}

	void ProtobufSession::SendInternal(const google::protobuf::Message& message) {
		const size_t message_id = ProtobufSerializer::Resolve(message);
		if (message_id == 0) {
			LOG_ERROR("Failed to resolve message ID for protobuf message.");
			return;
		}

		Network::StreamWriter writer(*output_stream_);
		if (writer.WriteMessage(message_id, message) == false) {
			LOG_ERROR("Failed to write message to output stream.");
			Disconnect();
			return;
		}

		auto connection = connection_;
		if (connection != nullptr && connection->IsSendInProgress() == false) {
			Ctrl(*connection).Post([conn = GetShared(this), buffer = writer.Flush()](Network::Connection& connection) {
				if (buffer.has_value() == false) {
					return;
				}
				connection.Send(buffer.value());
			});
		}
	}

	void ProtobufSession::SendInternal(const size_t message_id, const std::string& serialized_string) {
		Network::StreamWriter writer(*output_stream_);
		if (writer.WriteMessage(message_id, serialized_string.data(), serialized_string.size()) == false) {
			LOG_ERROR("Failed to write message to output stream.");
			Disconnect();
			return;
		}

		auto connection = connection_;
		if (connection != nullptr && connection->IsSendInProgress() == false) {
			Ctrl(*connection).Post([conn = GetShared(this), buffer = writer.Flush()](Network::Connection& connection) {
				if (buffer.has_value() == false) {
					return;
				}
				connection.Send(buffer.value());
			});
		}
	}
}

