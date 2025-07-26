#pragma once

namespace Server::Model {
	class Character {
	public:
		Character() = default;

		bool UpsertToDb(Sql::Agent& agent);
		static std::vector<std::unique_ptr<Character>> LoadByAccountIdAndServerId(Sql::Agent& agent, int64_t account_id, int32_t server_id);
		static bool IsCharacterExists(Sql::Agent& agent, const std::string& character_name);
		static bool IsCharacterExists(Sql::Agent& agent, int64_t account_id, int32_t server_id, int64_t character_id);

		void set_character_id(int64_t character_id) { character_id_ = character_id; }
		int64_t character_id() const { return character_id_; }
		void set_character_name(const std::string& character_name) { character_name_ = character_name; }
		const std::string& character_name() const { return character_name_; }
		void set_account_id(int64_t account_id) { account_id_ = account_id; }
		int64_t account_id() const { return account_id_; }
		void set_server_id(int32_t server_id) { server_id_ = server_id; }
		int32_t server_id() const { return server_id_; }
		void set_level(int32_t level) { level_ = level; }
		int32_t level() const { return level_; }
		void set_exp(int64_t exp) { exp_ = exp; }
		int64_t exp() const { return exp_; }
		void set_last_played(const System::Time& last_played) { last_played_ = last_played; }
		const System::Time& last_played() const { return last_played_; }

		void WriteTo(types::CharacterInfo* out_character);

	private:
		int64_t character_id_ = 0;
		std::string character_name_;
		int64_t account_id_;
		int32_t server_id_ = 0;
		int32_t level_ = 1;
		int64_t exp_ = 0;
		System::Time last_played_;
		System::Time created_at_;
	};
}


