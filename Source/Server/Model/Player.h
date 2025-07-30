#pragma once

namespace Server::Model {
	class Combat;
	class Player {
	public:
		Player();
		~Player();

		void set_character_id(int64_t character_id) { character_id_ = character_id; }
		int64_t character_id() const { return character_id_; }
		
		void set_account_id(int64_t account_id) { account_id_ = account_id; }
		int64_t account_id() const { return account_id_; }

		bool LoadFromDb();

		Combat& combat() { return *combat_; }
		const Combat& combat() const { return *combat_; }

	private:
		int64_t character_id_;
		int64_t account_id_;
		std::unique_ptr<Combat> combat_;
	};
}
