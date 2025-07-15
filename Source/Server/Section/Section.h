#pragma once

namespace Server {
	class WorldSession;
	class Section : public System::Actor {
	public:
		Section(uint64_t section_id) :
			_section_id(section_id) {
		}

		uint64_t section_id() const {
			return _section_id;
		}

		void set_map_uid(int32_t map_uid) {
			_map_uid = map_uid;
		}

		int32_t map_uid() const {
			return _map_uid;
		}

		void EnterSession(std::shared_ptr<WorldSession> session);
		void LeaveSession(std::shared_ptr<WorldSession> session);

	private:
		uint64_t _section_id;
		int32_t _map_uid = 0;
		std::unordered_map<int64_t/*session_id*/, std::shared_ptr<WorldSession>> world_sessions_;
	};
}
