#pragma once

namespace types {
	class SectionInfo;
} 

namespace Server {
	class WorldSession;
	class Section : public System::Actor {
	public:
		struct Id {
			union Value {
				uint64_t value;
				struct Persistent {
					int32_t map_uid;
					uint32_t auto_increment;
				} persistent;
			};

			Value value;
			static std::atomic<uint32_t> auto_increment_counter;

			Id (int32_t map_uid) {
				value.persistent.map_uid = map_uid;
				value.persistent.auto_increment = auto_increment_counter.fetch_add(1);
			}

			Id(int32_t map_uid, uint32_t auto_increment) {
				value.persistent.map_uid = map_uid;
				value.persistent.auto_increment = auto_increment;
			}

			static uint64_t Persistent(int32_t map_uid) {
				return Id(map_uid, 0);
			}

			operator uint64_t() {
				return value.value;
			}
		};

		Section(uint64_t section_id) 
			:
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

		bool IsEmpty() const {
			return _session_count;
		}

		void EnterSession(std::shared_ptr<WorldSession> session);
		void LeaveSession(std::shared_ptr<WorldSession> session);

		const std::array<std::shared_ptr<WorldSession>, 1024>& GetSessions() const {
			return world_sessions_arr_;
		}

		void WriteTo(types::SectionInfo* out_section) const;

		void Multicast(const std::shared_ptr<const google::protobuf::Message>& message, const int64_t source_session_id);
		void ForEach(std::function<void(WorldSession&)> session, const int64_t source_session_id);

	private:
		uint64_t _section_id;
		int32_t _map_uid = 0;
		size_t _session_count = 0;
		std::array<std::atomic<bool>, 1024> world_sessions_indexes_;
		std::array<std::shared_ptr<WorldSession>, 1024> world_sessions_arr_;
	};
}
