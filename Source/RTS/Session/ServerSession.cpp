#include "stdafx.h"
#include "ServerSession.h"
#include "Protocol/ServerProtocol.h"
#include "Protocol/ServerHandlerMap.h"
#include "Protobuf/Public/User.h"

#include "RTS/Authenticator/Authenticator.h"

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

		System::Future<bool> future;

		Ctrl(Authenticator::GetInstance()).Post([message, future](Authenticator& authenticator) mutable {
			future.SetResult(authenticator.ValidateAccessToken(message->access_token()));
		});

		future.Then([message](bool result) {
			if (result == false) {
				LOG_ERROR("Invalid access token for user_id: {}", message->user_id());
			}
		});
	}

	void ServerSession::RegisterHandler(ServerHandlerMap* handler_map) {
		handler_map->Register(OnHelloServer);
	}
}

