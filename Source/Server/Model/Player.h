#pragma once

namespace Server::Model {
	class Movable;
	class Player {
	public:
		Player(int64_t character_id);
		~Player();

		int64_t character_id() const { return character_id_; }

		bool LoadFromDb();

		Movable& movable() { return *movable_; }
		const Movable& movable() const { return *movable_; }

	private:
		int64_t character_id_;
		std::unique_ptr<Movable> movable_;
	};
}
