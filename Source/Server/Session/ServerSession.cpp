#include "stdafx.h"
#include "ServerSession.h"
#include "Protocol/ServerProtocol.h"
#include "Protocol/ServerHandlerMap.h"
#include "Protobuf/Public/User.h"

#include "Server/Authenticator/Authenticator.h"

namespace Server {
	int ServerSession::session_id_counter_ = 1;

	ServerSession::ServerSession()
	{
	}

	ServerSession::~ServerSession() {
	}

	std::unique_ptr<Network::Protocol> ServerSession::CreateProtocol() {
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

		Ctrl(Authenticator::GetInstance()).Async([message](Authenticator& authenticator) mutable {
			return authenticator.ValidateAccessToken(message->access_token());
		}).Then([message, session_ptr = Shared(&session)](bool result) -> std::shared_ptr<Server::ServerSession> {
			if (result == false) {
				LOG_ERROR("Invalid access token for user_id: {}", message->user_id());
				return {};
			}
			return session_ptr;
		}).ThenPost([](ServerSession& session) {
			DEBUG_ASSERT(session.IsSynchronized());
			LOG_INFO("Session {} authenticated successfully.", session.session_id());
		});
	}

	void ServerSession::RegisterHandler(ServerHandlerMap* handler_map) {
		handler_map->Register(OnHelloServer);
	}
}

