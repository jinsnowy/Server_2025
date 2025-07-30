#include "stdafx.h"
#include "Section.h"
#include "../Session/WorldSession.h"
#include "Server/Section/SpawnSystem.h"
#include "Server/Section/CollisionSystem.h"
#include "Protobuf/Public/Types.h"
#include "Server/Service/WorldService.h"
#include "Server/Model/UniqueId.h"
#include "Server/GameFramework/CollisionComponent.h"
#include "Server/GameObject/GameObject.h"
#include "Server/GameObject/Npc.h"
#include "Server/GameObject/Pc.h"
#include "Server/GameObject/Projectile.h"
#include "Server/Section/SectionRepository.h"

namespace Server {
	std::atomic<uint32_t> Section::Id::auto_increment_counter = 1;

	const System::Tick& Section::GetLastTick() {
		return SectionRepository::GetLastTick();
	}

	Section::Section(uint64_t section_id)
		:
		section_id_(section_id) ,
		spawn_system_(std::make_unique<SpawnSystem>(this)),
		collision_system_(std::make_unique<CollisionSystem>(this)),
		service_(System::DependencyInjection::Get<WorldService>())
	{
	}

	Section::~Section() = default;

	void Section::EnterSession(std::shared_ptr<WorldSession> session) {
		DEBUG_ASSERT(IsSynchronized());

		for (size_t i = 0; i < world_sessions_indexes_.size(); ++i) {
			bool is_occupied = world_sessions_indexes_[i].load();
			if (!is_occupied && world_sessions_indexes_[i].compare_exchange_strong(is_occupied, true)) {
				world_sessions_arr_[i] = session;
				++_session_count;

				if (owner_session_ == nullptr) {
					owner_session_ = session;
					OnOwnershipChanged(nullptr);
				}

				LOG_INFO("Section::EnterSession session_id: {}, section_id: {}", session->session_id(), section_id_);

				SpawnObject(session->GetPc());

				return;
			}
		}
	}

	void Section::LeaveSession(std::shared_ptr<WorldSession> session) {
		DEBUG_ASSERT(IsSynchronized());

		for (size_t search_index = 0; search_index < world_sessions_indexes_.size(); ++search_index) {
			const auto world_session = world_sessions_arr_[search_index];
			if (world_session == nullptr) {
				continue;
			}

			if (world_sessions_arr_[search_index] == session) {
				world_sessions_indexes_[search_index].store(false);
				world_sessions_arr_[search_index].reset();

				if (owner_session_ == session) {
					owner_session_.reset();
				}

				--_session_count;

				LOG_INFO("Section::LeaveSession session_id: {}, section_id: {}", session->session_id(), section_id_);

				DespawnObject(session->GetPc());
			
				break;
			}
		}

		if (owner_session_ != nullptr) {
			return;
		}

		for (size_t i = 0; i < world_sessions_arr_.size(); ++i) {
			if (world_sessions_arr_[i] && world_sessions_arr_[i]->session_id() != session->session_id()) {
				owner_session_ = world_sessions_arr_[i];
				OnOwnershipChanged(session);
				break;
			}
		}
	}

	void Section::WriteTo(types::SectionInfo* out_section) const {
		out_section->set_section_id(section_id_);
		out_section->set_map_uid(map_uid_);
		out_section->set_owner_character_id(GetOwnerCharacterId());
		spawn_system_->WriteTo(out_section);
	}

	void Section::Multicast(const std::shared_ptr<const google::protobuf::Message>& message, const int64_t source_session_id) {
		for (size_t i = 0; i < _session_count; ++i) {
			auto session = world_sessions_arr_[i];
			if (session == nullptr) {
				continue;
			}
			if (session->session_id() != source_session_id) {
				session->Send(message);
			}
		}
	}

	void Section::Multicast(const std::shared_ptr<const google::protobuf::Message>& message) {
		for (size_t i = 0; i < _session_count; ++i) {
			auto session = world_sessions_arr_[i];
			if (session == nullptr) {
				continue;
			}
			session->Send(message);
		}
	}

	void Section::ForEach(std::function<void(WorldSession&)> functor, const int64_t source_session_id) {
		auto shared_functor = std::make_shared<std::function<void(WorldSession&)>>(functor);
		for (size_t i = 0; i < _session_count; ++i) {
			auto session = world_sessions_arr_[i];
			if (session == nullptr) {
				continue;
			}
			if (session->session_id() != source_session_id) {
				Ctrl(*session).Post([shared_functor](WorldSession& session) {
					auto& functor = *shared_functor;
					functor(session);
				});
			}
		}
	}

	void Section::OnCreated() {
		LOG_INFO("Section::OnCreated section_id: {}", section_id_);

		DEBUG_ASSERT(game_objects_.empty());
		DEBUG_ASSERT(objects_to_despawn_.empty());
		DEBUG_ASSERT(all_tick_objects_.empty());
		DEBUG_ASSERT(await_first_tick_objects_.empty());

		spawn_system_->OnSectionCreated(*this);
		collision_system_->OnSectionCreated(*this);
	}

	void Section::OnDestroyed() {
		LOG_INFO("Section::OnDestroyed section_id: {}", section_id_);

		game_objects_.clear();
		objects_to_despawn_.clear();
		all_tick_objects_.clear();
		await_first_tick_objects_.clear();

		spawn_system_->OnSectionDestroyed(*this);
		collision_system_->OnSectionDestroyed(*this);
	}

	void Section::OnOwnershipChanged(std::shared_ptr<WorldSession> prev_session) {
		if (owner_session_ != nullptr) {
			auto notify = std::make_shared<world::SectionOwnershipChangedNotify>();
			notify->set_owner_character_id(GetOwnerCharacterId());
			owner_session_->Send(notify);
		}

		if (prev_session != nullptr) {
			auto prev_notify = std::make_shared<world::SectionOwnershipChangedNotify>();
			prev_notify->set_owner_character_id(GetOwnerCharacterId());
			prev_session->Send(prev_notify);
		}
	}

	int64_t Section::GetOwnerCharacterId() const {
		DEBUG_ASSERT(IsSynchronized());
		return owner_session_ ? owner_session_->character_id() : 0;
	}

	bool Section::SpawnObject(const std::shared_ptr<GameObject>& object) {
		if (!object) {
			return false;
		}
		if (object->object_id() == 0) {
			object->set_object_id(UniqueId::Issue(service_->server_id()));
		}
		return AddGameObject(object);
	}

	void Section::SpawnMany(std::vector<std::shared_ptr<Npc>> objects) {
		for (auto iter = objects.begin(); iter != objects.end();) {
			auto& npc = *iter;
			if (npc->object_id() == 0) {
				npc->set_object_id(UniqueId::Issue(service_->server_id()));
			}
			if (game_objects_.emplace(npc->object_id(), npc).second) {
				++iter;
			}
			else {
				iter = objects.erase(iter); // Remove the npc if it already exists
			}
		}

		for (const auto& npc : objects) {
			npc->set_section_id(section_id_);
			npc->set_created_tick(System::Tick::Current());
			await_first_tick_objects_.push_back(npc);

			collision_system_->OnGameObjectAdded(npc);

		}

		if (!objects.empty()) {
			spawn_system_->SpawnNpcsOnSection(objects);
		}
	}

	bool Section::DespawnObject(const std::shared_ptr<GameObject>& object) {
		if (!object || object->object_id() == 0) {
			return false;
		}
		return RemoveGameObject(object);
	}

	bool Section::AddGameObject(std::shared_ptr<GameObject> object) {
		if (game_objects_.emplace(object->object_id(), object).second == false) {
			return false;
		}

		object->set_section_id(section_id_);
		object->set_created_tick(System::Tick::Current());
		object->BeginPlay();
		await_first_tick_objects_.push_back(object);

		spawn_system_->OnGameObjectAdded(object);
		collision_system_->OnGameObjectAdded(object);

		return true;
	}

	bool Section::RemoveGameObject(std::shared_ptr<GameObject> object) {
		DEBUG_ASSERT(IsSynchronized());
		bool is_erased = game_objects_.erase(object->object_id()) > 0;
		if (!is_erased)
		{
			return false;
		}
	
		{
			// TODO : is there a better way to handle this?
			if (std::erase(all_tick_objects_, object) == 0)
			{
				std::erase(await_first_tick_objects_, object);
			}
		}

		object->EndPlay();
		spawn_system_->OnGameObjectRemoved(object);
		collision_system_->OnGameObjectRemoved(object);

		return true;
	}

	void Section::OnGameObjectAdded(std::shared_ptr<GameObject> object) {
		DEBUG_ASSERT(IsSynchronized());
		spawn_system_->OnGameObjectAdded(object);
		collision_system_->OnGameObjectAdded(object);
	}

	void Section::OnGameObjectRemoved(std::shared_ptr<GameObject> object) {
		DEBUG_ASSERT(IsSynchronized());
		spawn_system_->OnGameObjectRemoved(object);
		collision_system_->OnGameObjectRemoved(object);
	}

	void Section::OnTick(float delta_time) {
		DEBUG_ASSERT(IsSynchronized());

		const System::Tick& tick = System::Tick::Current();

		for (const auto& object : all_tick_objects_) {
			if (object->IsExpired(tick)) {
				objects_to_despawn_.push_back(object);
			}
			else {
				object->Update(delta_time);
			}
		}

		for (const auto& object : await_first_tick_objects_) {
			if (object->IsExpired(tick)) {
				objects_to_despawn_.push_back(object);
			}
			else {
				float first_delta_time = (tick - object->created_tick()).AsSecs();
				object->Update(first_delta_time);
				all_tick_objects_.push_back(object);
			}
		}
		await_first_tick_objects_.clear();
		
		for (const auto& object : objects_to_despawn_) {
			DespawnObject(object);
		}
		objects_to_despawn_.clear();

		// This can be used to perform periodic tasks within the section
		spawn_system_->OnTick(delta_time);
		collision_system_->OnTick(delta_time);

		for (const auto& session : world_sessions_arr_) {
			if (session) {
				Ctrl(*session).Post([delta_time](WorldSession& session) {
					session.OnSectionTick(delta_time);
				});
			}
		}
	}
}