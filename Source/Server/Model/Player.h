#pragma once

namespace Server::Model {
	class Movable;
	class Combat;
	class Player {
	public:
		Player(int64_t character_id);
		~Player();

		int64_t character_id() const { return character_id_; }

		bool LoadFromDb();

		Movable& movable() { return *movable_; }
		const Movable& movable() const { return *movable_; }

		Combat& combat() { return *combat_; }
		const Combat& combat() const { return *combat_; }

	private:
		int64_t character_id_;
		std::unique_ptr<Movable> movable_;
		std::unique_ptr<Combat> combat_;
	};
}
