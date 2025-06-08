#pragma once

#include "Protobuf/Public/ProtobufSession.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

namespace Server {
	class ServerHandlerMap;
	class ServerSession : public Protobuf::ProtobufSession {
	public:
		ServerSession();
		~ServerSession();

		void OnConnected();

		void OnDisconnected() {
			LOG_INFO("ServerSession::OnDisconnect session_id:{}", session_id_);
		}

		std::unique_ptr<Network::Protocol> CreateProtocol() override;

		int32_t session_id() const { return session_id_; }

		static void RegisterHandler(ServerHandlerMap* handler_map);

	private:
		int session_id_ = 0;
		static int session_id_counter_;
	};
}

