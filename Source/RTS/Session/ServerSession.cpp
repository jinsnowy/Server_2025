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

	std::unique_ptr<Network::Protocol> ServerSession::CreateProtocol()
	{
		return std::make_unique<ServerProtocol>();
	}

	void ServerSession::OnConnected() {
		ProtobufSession::OnConnected();

		session_id_ = session_id_counter_++;
		LOG_INFO("ServerSession::OnConnected session_id:{}, address:{}", session_id_, connection()->ToString());

		Send(user::HelloClient{});
	}

	static void OnHelloServer(ServerSession& session, const std::shared_ptr<const user::HelloServer>& message) {
		std::string serialized_string = message->SerializeAsString();
		LOG_INFO("ServerSession::OnHelloServer session_id:{}, address:{}, user_id:{}, access_token:{}",
			session.session_id(), session.connection()->ToString(), message->user_id(), message->access_token());
	}

	void ServerSession::RegisterHandler(ServerHandlerMap* handler_map) {
		handler_map->Register(OnHelloServer);
	}
}

