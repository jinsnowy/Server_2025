#pragma once

namespace Server {
	namespace Model {
		class Player;
	} // namespace Model

	namespace PlayerRepository {
		System::Future<std::unique_ptr<Model::Player>> Pop(int64_t character_id);
		void Insert(std::unique_ptr<Model::Player> player);
		void Remove(int64_t character_id);
	} // namespace PlayerRepository
}
