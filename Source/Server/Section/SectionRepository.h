#pragma once

namespace Server {
	class WorldSession;
	class Section;
} // namespace Network

namespace Server::SectionRepository {
	System::Future<std::shared_ptr<Section>> EnterSection(int32_t map_uid, std::shared_ptr<WorldSession> session);
	void LeaveSection(int32_t map_uid, std::shared_ptr<WorldSession> session);
}