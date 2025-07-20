#include "stdafx.h"
#include "LobbySession.h"
#include "Core/Sql/Database.h"
#include "Service/LobbyServiceDef.h"
#include "Protobuf/Public/User.h"
#include "Server/Service/LobbyServiceDef.h"
#include "Server/Database/DB.h"
#include "Server/Authenticator/Authenticator.h"
#include "../Model/Character.h"
#include "../Model/Server.h"
#include "Core/Network/IPAddress.h"

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

		LOG_INFO("LobbySession::OnConnected session_id:{}, address:{}", session_id(), connection()->ToString());

		Send(user::HelloClient{});
	}

	void LobbySession::AddCharacter(std::unique_ptr<Model::Character> character) {
		if (character) {
			characters_[character->character_id()] = std::move(character);
		}
	}

	Model::Character* LobbySession::GetCharacter(int64_t character_id)
	{
		auto it = characters_.find(character_id);
		if (it != characters_.end()) {
			return it->second.get();
		}
		return nullptr;
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

	static void OnWorldServerInfoReq (LobbySession& lobby_session, const std::shared_ptr<const user::WorldServerInfoReq>& ) {
		auto agent = LOBBYDB.GetAgent();

		auto worldservers = Model::Server::LoadByType(*agent, Model::ServerType::kWorldServer);

		user::WorldServerInfoRes res;
		for (const auto& server : worldservers) {
			auto* info = res.add_server_infos();
			info->set_server_id(server.server_id);
			info->set_server_address(server.server_address);
			info->set_current_player_num(0); // Placeholder for current player count
		}
		
		lobby_session.Send(res);
	}

	static void OnCreateCharacterReq (LobbySession& lobby_session, const std::shared_ptr<const user::CreateCharacterReq>& message) {
		auto agent = LOBBYDB.GetAgent();

		if (lobby_session.IsAccountLoaded() == false) {
			LOG_ERROR("Account not loaded for session_id: {}", lobby_session.session_id());
			return;
		}

		std::string character_name = message->character_name();
		if (character_name.empty()) {
			user::CreateCharacterRes res;
			res.set_result(types::kInvalidRequest);
			lobby_session.Send(res);
		}

		if (Model::Character::IsCharacterExists(*agent, character_name)) {
			LOG_ERROR("Character already exists: {}", character_name);
			user::CreateCharacterRes res;
			res.set_result(types::kDuplicatedName);
			lobby_session.Send(res);
			return;
		}

		auto server = Model::Server::LoadByServerId(*agent, message->server_id());
		if (server.has_value() == false) {
			LOG_ERROR("Server not found for server_id: {}", message->server_id());
			user::CreateCharacterRes res;
			res.set_result(types::kNotFound);
			lobby_session.Send(res);
			return;
		}

		auto character = std::make_unique<Model::Character>();
		character->set_account_id(lobby_session.account().account_id());
		character->set_server_id(message->server_id()); // Assuming server ID is 1 for this example
		character->set_character_name(message->character_name());
		character->set_level(1);
		character->set_exp(0);
		character->set_last_played(System::Time::UtcNow());
		if (character->UpsertToDb(*agent) == false) {
			LOG_ERROR("Failed to create character: {}", message->character_name());
			return;
		}

		user::CreateCharacterRes res;
		res.set_result(types::kSuccess);
		character->WriteTo(res.mutable_character_info());
		lobby_session.Send(res);
		lobby_session.AddCharacter(std::move(character));
	}

	static void OnGetCharacterListReq(LobbySession& lobby_session, const std::shared_ptr<const user::GetCharacterListReq>& msg) {
		if (lobby_session.IsAccountLoaded() == false) {
			LOG_ERROR("Account not loaded for session_id: {}", lobby_session.session_id());
			return;
		}
		auto agent = LOBBYDB.GetAgent();
		auto characters = Model::Character::LoadByAccountIdAndServerId(*agent, lobby_session.account().account_id(), msg->server_id()); // Assuming server ID is 1
		
		user::GetCharacterListRes res;
		res.set_result(types::kSuccess);
		for (auto& character : characters) {
			character->WriteTo(res.add_character_infos());
			lobby_session.AddCharacter(std::move(character));
		}

		lobby_session.Send(res);
	}

	static void OnPlayStartCharacterReq(LobbySession& lobby_session, const std::shared_ptr<const user::PlayStartCharacterReq>& msg) {
		if (lobby_session.IsAccountLoaded() == false) {
			LOG_ERROR("Account not loaded for session_id: {}", lobby_session.session_id());
			return;
		}
		
		auto character = lobby_session.GetCharacter(msg->character_id());
		if (character == nullptr) {
			LOG_ERROR("Character not found for character_id: {}", msg->character_id());
			user::PlayStartCharacterRes res;
			res.set_result(types::kNotFound);
			lobby_session.Send(res);
			return;
		}

		if (character->server_id() != msg->server_id()) {
			LOG_ERROR("Character {} is not on the requested server_id: {}", character->character_name(), msg->server_id());
			user::PlayStartCharacterRes res;
			res.set_result(types::kInvalidRequest);
			lobby_session.Send(res);
			return;
		}

		auto agent = LOBBYDB.GetAgent();
		auto server = Model::Server::LoadByServerId(*agent, msg->server_id());
		if (server.has_value() == false)
		{
			LOG_ERROR("Server not found for server_id: {}", msg->server_id());
			user::PlayStartCharacterRes res;
			res.set_result(types::kNotFound);
			lobby_session.Send(res);
			return;
		}

		user::PlayStartCharacterRes res;
		res.set_result(types::kSuccess);
		res.set_world_server_address(server->server_address);
		res.set_server_id(server->server_id);
		res.set_character_id(character->character_id());
		lobby_session.Send(res);
		
		LOG_INFO("Character {} started by session_id: {}", character->character_name(), lobby_session.session_id());
	}

	void LobbySession::RegisterHandler(LobbyHandlerMap* handler_map) {
		handler_map->Register(OnHelloServer);
		handler_map->Register(OnWorldServerInfoReq);
		handler_map->Register(OnCreateCharacterReq);
		handler_map->Register(OnGetCharacterListReq);
		handler_map->Register(OnPlayStartCharacterReq);
	}
}

