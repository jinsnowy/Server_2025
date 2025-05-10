#include "stdafx.h"
#include "ClientSession.h"
#include "Protocol/ClientHandlerMap.h"
#include "Protocol/ClientProtocol.h"
#include "Protobuf/Public/User.h"

namespace RTS {
	int ClientSession::session_id_counter_ = 1;

	ClientSession::ClientSession(std::shared_ptr<Network::Connection> conn)
		:
		Protobuf::ProtobufSession(std::move(conn)) {
	}

	void ClientSession::OnConnected()
	{
		{
			session_id_ = session_id_counter_++;

			LOG_INFO("ClientSession::OnConnected session_id:{}, address:{}", session_id_, connection()->ToString());

			InstallProtobuf();

			SendMessage("Hello, server!");

			Send(user::HelloServer{});
		}
	}

	void ClientSession::OnMessage(const std::string& message) {
		LOG_INFO("ClientSession::OnMessage: {}", message.c_str());
	}

	void ClientSession::InstallProtobuf() {
		InstallProtocol(std::make_unique<ClientProtocol>());
	}

	static void OnHelloClient(ClientSession& session, const std::shared_ptr<const user::HelloClient>& message) {
		LOG_INFO("ClientSession::OnHelloClient session_id:{}, address:{}", session.session_id(), session.connection()->ToString());
	}

	void ClientSession::RegisterHandler(ClientHandlerMap* handler_map) {
		handler_map->Register(OnHelloClient);
	}
}