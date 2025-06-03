#pragma once

#include "Core/Network/Session.h"

namespace Protobuf {
	class ProtobufSession : public Network::Session {
	public:
		ProtobufSession(const std::shared_ptr<System::Context>& context);
		ProtobufSession();

		void OnConnected() override;

		void Send(const google::protobuf::Message& message);
		void Send(const std::shared_ptr<const google::protobuf::Message>& message);

	private:
		void SendInternal(const google::protobuf::Message& message);
		void SendInternal(const size_t message_id, const std::string& serialized_string);
	};


}
