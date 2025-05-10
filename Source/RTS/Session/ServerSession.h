#pragma once

#include "Protobuf/Public/ProtobufSession.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf

namespace RTS {
	class ServerHandlerMap;
	class ServerSession : public Protobuf::ProtobufSession {
	public:
		ServerSession(std::shared_ptr<Network::Connection> conn);
		~ServerSession();

		void OnConnected();

		void OnDisconnected() {
			LOG_INFO("ServerSession::OnDisconnect session_id:{}, address:{}", session_id_, connection()->ToString());
		}

		void OnMessage(const std::string& message) override;

		int32_t session_id() const { return session_id_; }


		static void RegisterHandler(ServerHandlerMap* handler_map);

	private:
		int session_id_ = 0;
		static int session_id_counter_;

		void InstallProtobuf();
	};
}

