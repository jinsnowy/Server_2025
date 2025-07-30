#pragma once

#include "Server/Section/SectionComponent.h"

namespace types {
	class ProjectileInfo;
	class NpcInfo;
	class PcInfo;
	class SectionInfo;
	class NpcSpawnInfo;
} // namespace types

namespace google::protobuf {
	template<typename T>
	class RepeatedPtrField;
	template<typename Key, typename Value>
	class Map;
} // namespace google::protobuf

namespace Server {
	namespace DataTable {
		struct SpawnerDataRecord;
	}

	class WorldService;
	class GameObject;
	class Npc;
	class Pc;
	class Projectile;
	class SpawnSystem : public SectionComponent {
	public:
		SpawnSystem(Section* owner);
		~SpawnSystem() = default;

		void OnSectionCreated(Section& section) override;
		void OnSectionDestroyed(Section& section) override;
		void OnTick(float delta_time) override;
		void OnGameObjectAdded(std::shared_ptr<GameObject> object) override;
		void OnGameObjectRemoved(std::shared_ptr<GameObject> object) override;

		std::shared_ptr<Npc> FindNpc(int64_t object_id) const;
		std::shared_ptr<Pc> FindPc(int64_t object_id) const;
		std::shared_ptr<Projectile> FindProjectile(int64_t object_id) const;

		bool IsValidSpawnInfo(int32_t spawner_id, const std::vector<types::NpcSpawnInfo>& requested_infos);
		std::vector<std::shared_ptr<Npc>> MakeNpcsFromSpawner(int32_t spawner_id, std::vector<types::NpcSpawnInfo> requested_infos);
	
		const DataTable::SpawnerDataRecord* spawner_data_record() const {
			return spawner_data_record_;
		}

		const System::Tick& last_spawn_tick() const {
			return last_spawn_tick_;
		}

	private:
		friend class Section;

		Section* owner_;
		System::Tick last_spawn_tick_;
		std::shared_ptr<WorldService> service_;
		
		using SpawnMethod = std::function<bool(SpawnSystem*, std::shared_ptr<GameObject>)>;
		using DespawnMethod = std::function<bool(SpawnSystem*, std::shared_ptr<GameObject>)>;
		std::unordered_map<uint32_t, SpawnMethod> spawn_methods_;
		std::unordered_map<uint32_t, DespawnMethod> despawn_methods_;

		std::unordered_map<int64_t, std::shared_ptr<Projectile>> projectiles_; // Map of spawned projectiles by actor ID
		std::unordered_map<int64_t, std::shared_ptr<Npc>> npcs_; // Map of spawned objects by actor ID
		std::unordered_map<int64_t/*character_id*/, std::shared_ptr<Pc>> pcs_; // Map of spawned PCs by actor ID

		std::unordered_map<int32_t, int32_t> spawn_counts_; // Map of spawn counts by spawner ID
		const DataTable::SpawnerDataRecord* spawner_data_record_ = nullptr; // Pointer to the spawner data record

		bool SpawnNpcOnSection(std::shared_ptr<Npc> object);
		bool SpawnNpcsOnSection(std::vector<std::shared_ptr<Npc>>& npcs);
		bool DespawnNpcOnSection(std::shared_ptr<Npc> object);

		bool SpawnProjectileOnSection(std::shared_ptr<Projectile> projectile);
		bool DespawnProjectileOnSection(std::shared_ptr<Projectile> projectile);

		bool SpawnPcOnSection(std::shared_ptr<Pc> object);
		bool DespawnPcOnSection(std::shared_ptr<Pc> object);

		void WriteTo(types::SectionInfo* section_info);
	};
}
