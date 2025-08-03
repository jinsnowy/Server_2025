#include "stdafx.h"
#include "ClientSession.h"
#include "Protobuf/Public/User.h"
#include "Server/Service/ClientServiceDef.h"

namespace Server {
	int ClientSession::session_id_counter_ = 1;

	ClientSession::ClientSession()
		:
		Protobuf::ProtobufSession() {
	}

	std::unique_ptr<Network::Protocol> ClientSession::CreateProtocol() {
		return std::make_unique<ClientProtocol>();
	}

	void ClientSession::OnConnected(const Network::IPAddress& address) {
		Network::Session::OnConnected(address);

		session_id_ = session_id_counter_++;

		LOG_INFO("ClientSession::OnConnected session_id:{}, address:{}", session_id_, connection()->ToString());
	}

	void ClientSession::OnConnectFailed(const Network::IPAddress& address, const std::string& error_message)
	{
		LOG_ERROR("ClientSession::OnConnectFailed session_id:{}, address:{}, error:{}", session_id_, address.ToString(), error_message);
	}


	static void OnHelloClient(ClientSession& session, const std::shared_ptr<const user::HelloClient>&) {
		LOG_INFO("ClientSession::OnHelloClient session_id:{}, address:{}", session.session_id(), session.connection()->ToString());
	}

	void ClientSession::RegisterHandler(ClientHandlerMap* handler_map) {
		handler_map->Register(OnHelloClient);
	}
}