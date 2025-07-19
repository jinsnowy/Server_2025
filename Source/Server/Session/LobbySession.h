#pragma once

#include "Protobuf/Public/ProtobufSession.h"

namespace google::protobuf {
	class Message;
} // namespace google::protobuf


namespace Server {

	namespace Model {
		class Account;
		class Character;
		struct AccountTokenInfo;
	}

	class LobbyHandlerMap;
	class LobbySession : public Protobuf::ProtobufSession {
	public:
		LobbySession();
		~LobbySession();

		static void RegisterHandler(LobbyHandlerMap* handler_map);

		void OnConnected();

		void OnDisconnected() {
			LOG_INFO("LobbySession::OnDisconnect session_id:{}", session_id_);
		}

		std::unique_ptr<Network::Protocol> CreateProtocol() override;

		int64_t session_id() const { return session_id_; }

		bool LoadAccount(const Model::AccountTokenInfo& account_token_info);

		const Model::Account& account() const {
			return *account_;
		}

		bool IsAccountLoaded() const {
			return account_ != nullptr;
		}

		void AddCharacter(std::unique_ptr<Model::Character> character);

		Model::Character* GetCharacter(int64_t character_id);

	private:
		int64_t session_id_ = 0;

		std::unique_ptr<Model::Account> account_;
		std::unordered_map<int64_t, std::unique_ptr<Model::Character>> characters_;
	};
}

