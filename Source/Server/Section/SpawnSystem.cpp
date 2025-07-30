#include "stdafx.h"
#include "SpawnSystem.h"
#include "../DataTable/SpawnerDataRecord.h"
#include "Server/Section/Section.h"
#include "Server/GameObject/GameObject.h"
#include "Protobuf/Public/Types.h"
#include "Protobuf/Public/World.h"
#include "Server/Utilites/Protobuf.h"
#include "Core/System/DependencyInjection.h"
#include "Server/Service/WorldService.h"
#include "Server/Model/UniqueId.h"
#include "Server/GameObject/Pc.h"
#include "Server/GameObject/Npc.h"
#include "Server/GameObject/Projectile.h"

namespace Server {

	template<typename T>
	static bool DoSpawn(bool (SpawnSystem::* spawn_func_wrapper)(std::shared_ptr<T>), SpawnSystem* spawn_system, std::shared_ptr<GameObject> game_object) {
		return (spawn_system->*spawn_func_wrapper)(std::static_pointer_cast<T>(game_object));
	}

	template<typename T>
	static bool DoDespawn(bool (SpawnSystem::* despawn_func_wrapper)(std::shared_ptr<T>), SpawnSystem* spawn_system, std::shared_ptr<GameObject> game_object) {
		return (spawn_system->*despawn_func_wrapper)(std::static_pointer_cast<T>(game_object));
	}

	SpawnSystem::SpawnSystem(Section* owner)
		:
		owner_(owner),
		service_(System::DependencyInjection::Get<WorldService>()) {
		spawn_methods_[Npc::kTypeId] = std::bind(DoSpawn<Npc>, &SpawnSystem::SpawnNpcOnSection, std::placeholders::_1, std::placeholders::_2);
		spawn_methods_[Pc::kTypeId] = std::bind(DoSpawn<Pc>, &SpawnSystem::SpawnPcOnSection, std::placeholders::_1, std::placeholders::_2);
		spawn_methods_[Projectile::kTypeId] = std::bind(DoSpawn<Projectile>, &SpawnSystem::SpawnProjectileOnSection, std::placeholders::_1, std::placeholders::_2);

		despawn_methods_[Npc::kTypeId] = std::bind(DoDespawn<Npc>, &SpawnSystem::DespawnNpcOnSection, std::placeholders::_1, std::placeholders::_2);
		despawn_methods_[Pc::kTypeId] = std::bind(DoDespawn<Pc>, &SpawnSystem::DespawnPcOnSection, std::placeholders::_1, std::placeholders::_2);
		despawn_methods_[Projectile::kTypeId] = std::bind(DoDespawn<Projectile>, &SpawnSystem::DespawnProjectileOnSection, std::placeholders::_1, std::placeholders::_2);
	}

	void SpawnSystem::OnSectionCreated(Section& section) {
		spawner_data_record_ = DataTable::SpawnerDataRecord::GetInstance().Find(section.map_uid());
		if (!spawner_data_record_) {
			LOG_ERROR("SpawnerDataRecord not found for map_uid: {}", section.map_uid());
		}
	}

	void SpawnSystem::OnSectionDestroyed(Section&) {
		npcs_.clear();
		pcs_.clear();
		projectiles_.clear();
	}

	void SpawnSystem::OnTick(float) {
	}

	void SpawnSystem::OnGameObjectAdded(std::shared_ptr<GameObject> object) {
		const uint32_t type_id = object->type_id();
		auto it = spawn_methods_.find(type_id);
		if (it != spawn_methods_.end()) {
			SpawnMethod spawn_method = it->second;
			(spawn_method)(this, object);
			return;
		}
	}

	void SpawnSystem::OnGameObjectRemoved(std::shared_ptr<GameObject> object) {
		const uint32_t type_id = object->type_id();
		auto it = despawn_methods_.find(type_id);
		if (it != despawn_methods_.end()) {
			DespawnMethod despawn_method = it->second;
			(despawn_method)(this, object);
			return;
		}
	}

	bool SpawnSystem::SpawnNpcOnSection(std::shared_ptr<Npc> npc) {
		std::shared_ptr<world::SpawnNpcOnSectionNotify> notify = std::make_shared<world::SpawnNpcOnSectionNotify>();
		if (npcs_.emplace(npc->object_id(), npc).second) {
			npc->WriteTo(notify->add_npc_infos());

			const int32_t spawner_id = npc->spawner_id();
			if (spawner_id > 0) {
				auto& spawn_count = spawn_counts_[spawner_id];
				++spawn_count;
			}

			owner_->Multicast(notify); // 0 means no source session ID

			return true;
		}

		return false;
	}

	bool SpawnSystem::SpawnNpcsOnSection(std::vector<std::shared_ptr<Npc>>& npcs) {
		std::shared_ptr<world::SpawnNpcOnSectionNotify> notify = std::make_shared<world::SpawnNpcOnSectionNotify>();
		for (auto iter = npcs.begin(); iter != npcs.end();) {
			auto& npc = *iter;
			if (npcs_.emplace(npc->object_id(), npc).second) {
				npc->WriteTo(notify->add_npc_infos());

				const int32_t spawner_id = npc->spawner_id();
				if (spawner_id > 0) {
					auto& spawn_count = spawn_counts_[spawner_id];
					++spawn_count;
				}

				++iter;
			}
			else {
				iter = npcs.erase(iter); // Remove the npc if it already exists
			}
		}

		if (!notify->npc_infos().empty()) {
			owner_->Multicast(notify); // 0 means no source session ID
		}

		return !npcs.empty();
	}

	bool SpawnSystem::DespawnNpcOnSection(std::shared_ptr<Npc> object) {
		auto it = npcs_.find(object->object_id());
		if (it != npcs_.end()) {
			npcs_.erase(it);
			
			const int32_t spawner_id = object->spawner_id();
			if (spawner_id > 0) {
				auto& spawn_count = spawn_counts_[spawner_id];
				if (spawn_count > 0) {
					--spawn_count;
				}
			}
			
			LOG_INFO("Despawned npc with actor ID: {}", object->object_id());

			auto notify = std::make_shared<world::DespawnNpcOnSectionNotify>();
			notify->set_object_id(object->object_id());
			owner_->Multicast(notify); // 0 means no source session ID

			return true;
		}

		return false;
	}

	std::shared_ptr<Npc> SpawnSystem::FindNpc(int64_t object_id) const {
		auto it = npcs_.find(object_id);
		if (it != npcs_.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<Pc> SpawnSystem::FindPc(int64_t object_id) const {
		auto it = pcs_.find(object_id);
		if (it != pcs_.end()) {
			return it->second;
		}
		return nullptr;
	}

	bool SpawnSystem::SpawnProjectileOnSection(std::shared_ptr<Projectile> projectile) {
		if (projectiles_.emplace(projectile->object_id(), projectile).second) {

			LOG_INFO("Spawned projectile with actor ID: {}", projectile->object_id());

			auto notify = std::make_shared<world::SpawnProjectileOnSectionNotify>();
			projectile->WriteTo(notify->mutable_projectile_info());

			owner_->Multicast(notify);

			return true;
		}

		return false;
	}

	bool SpawnSystem::DespawnProjectileOnSection(std::shared_ptr<Projectile> projectile) {
		auto it = projectiles_.find(projectile->object_id());
		if (it != projectiles_.end()) {
			LOG_INFO("Despawned projectile with actor ID: {}", projectile->object_id());

			projectiles_.erase(it);

			auto notify = std::make_shared<world::DespawnProjectileOnSectionNotify>();
			notify->set_object_id(projectile->object_id());
			owner_->Multicast(notify); // 0 means no source session ID

			return true;
		}

		return false;
	}

	bool SpawnSystem::SpawnPcOnSection(std::shared_ptr<Pc> object) {
		if (pcs_.emplace(object->character_id(), object).second) {
			LOG_INFO("Spawned PC with character ID: {}", object->character_id());

			auto notify = std::make_shared<world::OtherClientEnterNotify>();
			object->WriteTo(notify->mutable_pc_info());
			owner_->Multicast(notify, object->session_id()); // Notify other clients about the PC entering
		}
		else {
			LOG_ERROR("Failed to spawn PC with character ID: {}. Already exists.", object->character_id());
		}

		return false;
	}

	bool SpawnSystem::DespawnPcOnSection(std::shared_ptr<Pc> object) {
		auto it = pcs_.find(object->character_id());
		if (it != pcs_.end()) {
			LOG_INFO("Despawned projectile with character ID: {}", object->character_id());

			auto notify = std::make_shared<world::OtherClientLeaveNotify>();
			notify->set_character_id(object->character_id());
			owner_->Multicast(notify, object->session_id()); // Notify other clients about the PC leaving

			pcs_.erase(it);

			return true;
		}

		return false;
	}

	std::shared_ptr<Projectile> SpawnSystem::FindProjectile(int64_t object_id) const {
		auto it = projectiles_.find(object_id);
		if (it != projectiles_.end()) {
			return it->second;
		}
		return nullptr;
	}

	bool SpawnSystem::IsValidSpawnInfo(int32_t spawner_id, const std::vector<types::NpcSpawnInfo>& requested_infos) {
		if (spawner_data_record_ == nullptr) {
			LOG_ERROR("Spawner data record not initialized.");
			return false;
		}

		auto it = spawner_data_record_->items.find(spawner_id);
		if (it == spawner_data_record_->items.end()) {
			LOG_ERROR("Spawner ID {} not found in spawner data record.", spawner_id);
			return false;
		}

		const auto current = System::Tick::Current();
		if ((current - last_spawn_tick_).AsSecs() < static_cast<float>(it->second.spawnDurationSeconds)) {
			LOG_ERROR("Spawn request too soon after last spawn, required cooldown: {} seconds", it->second.spawnDurationSeconds);
			return false;
		}

		int32_t& spawn_count = spawn_counts_[spawner_id];
		if (spawn_count >= it->second.maxSpawnCount) {
			LOG_ERROR("Max spawn count reached for spawner ID {}", spawner_id);
			return false;
		}

		Math::Vec3 spawner_location = it->second.location;
		float spawner_radius = it->second.radius;
		for (const auto& spawn_info : requested_infos) {
			Math::Vec3 spawn_location = Utilites::ReadFrom(spawn_info.pose().location());
			if (Math::DistanceTo(spawner_location, spawn_location) > spawner_radius + 50.f) {
				LOG_ERROR("Spawn location is outside the spawner radius {}", spawner_radius);
				return false;
			}
		}

		return true;
	}

	std::vector<std::shared_ptr<Npc>> SpawnSystem::MakeNpcsFromSpawner(int32_t spawner_id, std::vector<types::NpcSpawnInfo> requested_infos) {
		last_spawn_tick_ = System::Tick::Current();
		std::vector<std::shared_ptr<Npc>> npcs;
		for (auto& info : requested_infos) {
			int64_t actor_id = UniqueId::Issue(service_->server_id());
			auto game_object = std::make_shared<Npc>();
			game_object->set_object_id(actor_id);
			game_object->set_spawner_id(spawner_id);
			game_object->SetPose(Utilites::ReadFrom(info.pose()));
			game_object->set_hp(100);
			npcs.push_back(game_object);
		}

		return npcs;
	}

	void SpawnSystem::WriteTo(types::SectionInfo* spawn_info) {
		for (const auto& pair : npcs_) {
			const auto& object = pair.second;
			object->WriteTo(spawn_info->add_npc_infos());
		}

		for (const auto& pair : projectiles_) {
			const auto& object = pair.second;
			object->WriteTo(spawn_info->add_projectile_infos());
		}

		for (const auto& pair : pcs_) {
			const auto& object = pair.second;
			object->WriteTo(spawn_info->add_pc_infos());
		}
	}
}

