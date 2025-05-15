#include "stdafx.h"
#include "ServerSession.h"
#include "Protocol/ServerProtocol.h"
#include "Protocol/ServerHandlerMap.h"
#include "Protobuf/Public/User.h"

namespace RTS {
	int ServerSession::session_id_counter_ = 1;

	ServerSession::ServerSession()
	{
	}

	ServerSession::~ServerSession() {
	}

	void ServerSession::OnConnected() {
		session_id_ = session_id_counter_++;

		LOG_INFO("ServerSession::OnConnected session_id:{}, address:{}", session_id_, connection()->ToString());

		InstallProtobuf();

		Send(user::HelloClient{});
	}

	void ServerSession::OnMessage(const std::string& message) {
		LOG_INFO("ServerSession::OnMessage: {}", message.c_str());

		SendInternalMessage("Hello, client!");
	}

	void ServerSession::InstallProtobuf() {
		InstallProtocol(std::make_unique<ServerProtocol>());
	}

	static void OnHelloServer(ServerSession& session, const std::shared_ptr<const user::HelloServer>& message) {
		LOG_INFO("ServerSession::OnHelloServer session_id:{}, address:{}", session.session_id(), session.connection()->ToString());
	}

	void ServerSession::RegisterHandler(ServerHandlerMap* handler_map) {
		handler_map->Register(OnHelloServer);
	}
}

