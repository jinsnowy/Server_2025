#pragma once

namespace Server {
	class WorldSession;
}

namespace Server::PlayerTick {
	static int32_t kPlayerTickInterval = 50; // milliseconds

	void SetServerTickInterVal(int32_t interval);
	float GetServerTick();
	void BeginTick(const std::shared_ptr<WorldSession>& world_session);
	void EndTick(const std::shared_ptr<WorldSession>& world_session);
}
