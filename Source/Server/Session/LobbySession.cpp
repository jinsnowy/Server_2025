#include "stdafx.h"
#include "LobbySession.h"
#include "Core/Sql/Database.h"
#include "Service/LobbyServiceDef.h"
#include "Protobuf/Public/User.h"
#include "Server/Service/LobbyServiceDef.h"
#include "Server/Database/DB.h"
#include "Server/Authenticator/Authenticator.h"
#include "SessionId.h"

namespace Server {

	LobbySession::LobbySession()
	{
	}

	LobbySession::~LobbySession() {
	}

	std::unique_ptr<Network::Protocol> LobbySession::CreateProtocol() {
		return std::make_unique<LobbyProtocol>();
	}

	void LobbySession::OnConnected() {
		ProtobufSession::OnConnected();

		session_id_ = SessionId::Issue(SessionType::kLobbySession);
		LOG_INFO("LobbySession::OnConnected session_id:{}, address:{}", session_id_, connection()->ToString());

		Send(user::HelloClient{});
	}

	bool LobbySession::LoadAccount(const Model::AccountTokenInfo& account_token_info) {
		DEBUG_ASSERT(account_token_info.user_id.empty() == false);

		if (account_ == nullptr) {
			account_ = std::make_unique<Model::Account>(account_token_info.user_id, account_token_info.username);
		}
		else {
			account_->set_user_id(account_token_info.user_id);
			account_->set_username(account_token_info.username);
		}

		account_->set_last_login_time(System::Time::UtcNow());

		auto agent = LOBBYDB.GetAgent();
		if (account_->UpsertToDb(*agent) == false) {
			LOG_ERROR("Failed to upsert account to database for user_id: {}", account_token_info.user_id);
			return false;
		}
		if (account_->LoadFromDb(*agent) == false) {
			LOG_ERROR("Failed to load account from database for user_id: {}", account_token_info.user_id);
			return false;
		}

		return true;
	}

	static void OnHelloServer(LobbySession& session, const std::shared_ptr<const user::HelloServer>& message) {
		std::string serialized_string = message->SerializeAsString();
		LOG_INFO("LobbySession::OnHelloServer session_id:{}, address:{}, user_id:{}, access_token:{}",
			session.session_id(), session.connection()->ToString(), message->user_id(), message->access_token());

			Ctrl(Authenticator::GetInstance()).Async([message](Authenticator& authenticator) mutable {
				return authenticator.ConsumeToken(message->access_token());
			}).Then([message, session_ptr = MakeShared(&session)](std::optional<Model::AccountTokenInfo> result) -> std::shared_ptr<Server::LobbySession> {
				if (result.has_value() == false) {
					LOG_ERROR("Invalid access token for user_id: {}", message->user_id());
					session_ptr->Disconnect();
					return {};
				}

				Ctrl(*session_ptr).Async([account_token_info = result.value()](LobbySession& session) -> std::shared_ptr<LobbySession> {
				if (session.LoadAccount(account_token_info) == false) {
					LOG_ERROR("Failed to load account for user_id: {}", account_token_info.user_id);
					session.Disconnect();
					return {};
				}

				return MakeShared(session);
			}).ThenPost([](LobbySession& session) {
				DEBUG_ASSERT(session.IsSynchronized());

				LOG_INFO("[LOGIN] account: {} logined", session.account().ToString());

				const auto& account = session.account();

				user::HelloClientAck ack;
				ack.set_account_id(account.account_id());
				ack.set_username(account.username());
				ack.set_user_id(account.user_id());
				*ack.mutable_last_login_time() = Protobuf::ToTimestamp(account.last_login_time());
				*ack.mutable_last_logout_time() = Protobuf::ToTimestamp(account.last_logout_time());
				*ack.mutable_created_at() = Protobuf::ToTimestamp(account.created_at());
				session.Send(ack);
			});

			return session_ptr;
		});
	}

	void LobbySession::RegisterHandler(LobbyHandlerMap* handler_map) {
		handler_map->Register(OnHelloServer);
	}
}

