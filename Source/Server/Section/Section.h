#pragma once

namespace types {
	class SectionInfo;
}  // namespace types

namespace Server {
	class WorldSession;
	class GameObject;
	class SpawnSystem;
	class CollisionSystem;
	class Npc;
	class Pc;
	class Projectile;
	class WorldService;
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

		static const System::Tick& GetLastTick();

		Section(uint64_t section_id);
		~Section();

		uint64_t section_id() const {
			return section_id_;
		}

		void set_map_uid(int32_t map_uid) {
			map_uid_ = map_uid;
		}

		int32_t map_uid() const {
			return map_uid_;
		}

		bool IsEmpty() const {
			return _session_count;
		}

		size_t GetCount() const {
			return _session_count;
		}

		void EnterSession(std::shared_ptr<WorldSession> session);
		void LeaveSession(std::shared_ptr<WorldSession> session);

		const std::array<std::shared_ptr<WorldSession>, 1024>& GetSessions() const {
			return world_sessions_arr_;
		}

		void WriteTo(types::SectionInfo* out_section) const;

		void Multicast(const std::shared_ptr<const google::protobuf::Message>& message, const int64_t source_session_id);
		void Multicast(const std::shared_ptr<const google::protobuf::Message>& message);
		void ForEach(std::function<void(WorldSession&)> session, const int64_t source_session_id);

		void OnCreated();
		void OnTick(float delta_time);
		void OnDestroyed();
		void OnOwnershipChanged(std::shared_ptr<WorldSession> prev_session);

		int64_t GetOwnerCharacterId() const;

		SpawnSystem& spawn_system() {
			return *spawn_system_;
		}

		bool SpawnObject(const std::shared_ptr<GameObject>& object);
		void SpawnMany(std::vector<std::shared_ptr<Npc>> objects);
		bool DespawnObject(const std::shared_ptr<GameObject>& object);

	private:
		uint64_t section_id_;
		int32_t map_uid_ = 0;
		size_t _session_count = 0;
		System::Tick last_tick_;

		std::shared_ptr<WorldService> service_;
		std::shared_ptr<WorldSession> owner_session_;
		std::array<std::atomic<bool>, 1024> world_sessions_indexes_;
		std::array<std::shared_ptr<WorldSession>, 1024> world_sessions_arr_;

		std::unordered_map<int64_t, std::shared_ptr<GameObject>> game_objects_; // Map of all game objects in the section
		std::vector<std::shared_ptr<GameObject>> objects_to_despawn_; // Vector of all sessions in the section
		std::vector<std::shared_ptr<GameObject>> all_tick_objects_; // Vector of despawned objects
		std::vector<std::shared_ptr<GameObject>> await_first_tick_objects_; // Vector of objects awaiting first tick update

		std::unique_ptr<SpawnSystem> spawn_system_;
		std::unique_ptr<CollisionSystem> collision_system_;

		bool AddGameObject(std::shared_ptr<GameObject> object);
		bool RemoveGameObject(std::shared_ptr<GameObject> object);

		void OnGameObjectAdded(std::shared_ptr<GameObject> object);
		void OnGameObjectRemoved(std::shared_ptr<GameObject> object);
	};
}
