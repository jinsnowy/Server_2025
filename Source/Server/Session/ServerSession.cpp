#include "stdafx.h"
#include "ServerSession.h"
#include "Core/Sql/Database.h"
#include "Protocol/ServerProtocol.h"
#include "Protocol/ServerHandlerMap.h"
#include "Protobuf/Public/User.h"

#include "Server/Database/DB.h"
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
			return authenticator.ConsumeToken(message->access_token());
		}).Then([message, session_ptr = MakeShared(&session)](std::optional<Model::AccountTokenInfo> result) -> std::shared_ptr<Server::ServerSession> {
			if (result.has_value() == false) {
				LOG_ERROR("Invalid access token for user_id: {}", message->user_id());
				return {};
			}

			Ctrl(*session_ptr).Async([account_token_info = result.value()](ServerSession& session) -> std::shared_ptr<ServerSession> {
				if (session.LoadAccount(account_token_info) == false) {
					LOG_ERROR("Failed to load account for user_id: {}", account_token_info.user_id);
					session.Disconnect();
					return {};
				}
				return MakeShared(session);
			}).ThenPost([](ServerSession& session) {
				DEBUG_ASSERT(session.IsSynchronized());

				LOG_INFO("[LOGIN] account: {} logined", session.account().ToString());

				//session_ptr->Send(user::HelloClientAck{});
			});

			return session_ptr;
		});
}

	bool ServerSession::LoadAccount(const Model::AccountTokenInfo& account_token_info) {
		DEBUG_ASSERT(account_token_info.user_id.empty() == false);

		if (account_ == nullptr) {
			account_ = std::make_unique<Model::Account>(account_token_info.user_id, account_token_info.username);
		}
		else {
			account_->set_user_id(account_token_info.user_id);
			account_->set_username(account_token_info.username);
		}

		account_->set_last_login_time(System::Time::GetCurrent());
		
		auto agent = LOBBYDB.GetAgent();
		if (account_->UpsertToDb(agent) == false) {
			LOG_ERROR("Failed to upsert account to database for user_id: {}", account_token_info.user_id);
			return false;
		}

		return true;
	}


	void ServerSession::RegisterHandler(ServerHandlerMap* handler_map) {
		handler_map->Register(OnHelloServer);
	}
}

