#pragma once

namespace Server {
	class WorldSession;
}

namespace Server::PlayerMovableTick {
	static int32_t kPlayerMovableTickInterval = 50; // milliseconds

	void SetServerTickInterVal(int32_t interval);
	float GetServerTick();
	void BeginTick(const std::shared_ptr<WorldSession>& world_session);
	void EndTick(const std::shared_ptr<WorldSession>& world_session);
}
