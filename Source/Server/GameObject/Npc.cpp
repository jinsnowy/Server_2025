#include "stdafx.h"
#include "Npc.h"
#include "Server/Utilites/Protobuf.h"
#include "Core/Math/Collision/CapsuleCollider.h"
#include "Server/GameFramework/CollisionComponent.h"
#include "Server/GameObject/Pc.h"
#include "Server/Section/Section.h"
#include "Server/Section/SpawnSystem.h"
#include "Server/Section/SectionRepository.h"

namespace Server {
	
	const uint32_t Npc::kTypeId = System::hashcode<Npc>();

	Npc::Npc()
		: 
		GameObject(kTypeId, System::type_name_v<Npc>),
		spawner_id_(0),
		hp_(0) {
		auto collision_comp = std::make_unique<CollisionComponent>(this);
		auto collider = std::make_unique<Math::CapsuleCollider>(transform_.translation, kRadius, kHalfHeight);
		collision_component_ = collision_comp.get();
		collision_component_->set_collider(std::move(collider));
		collision_component_->set_on_collision_callback(
			std::bind(&Npc::OnCollision, this, std::placeholders::_1, std::placeholders::_2));
		AddComponent(std::move(collision_comp));
	}

	Npc::~Npc() = default;

	void Npc::BeginPlay() {
		GameObject::BeginPlay();
	}

	void Npc::Update(float delta_time) {
		GameObject::Update(delta_time);

		auto collider = collision_component_->collider();
		SectionRepository::FindSection(section_id_).ThenPost([position = collider->center()](Section& section) {
			auto notify = std::make_shared<world::DebugSimulationPositionNotify>();
			notify->set_color(types::Color::kGreen);
			notify->set_shape(types::DebugShape::kCapsule);
			notify->mutable_shape_info()->mutable_capsule()->set_radius(Npc::kRadius);
			notify->mutable_shape_info()->mutable_capsule()->set_half_height(Npc::kHalfHeight);
			Utilites::WriteTo(position, notify->mutable_position());
			section.Multicast(notify); // 0 means no source session ID
			section.Multicast(notify); // 0 means no source session ID
		});
	}

	void Npc::WriteTo(types::NpcInfo* npc_info) const {
		npc_info->set_object_id(object_id());
		npc_info->set_spawner_id(spawner_id_);
		npc_info->set_current_hp(hp_);
		Utilites::WriteTo(transform(), npc_info->mutable_pose());
	}

	void Npc::OnCollision(CollisionComponent& other_component, CollisionState state) {
		if (other_component.owner()->type_id() == Pc::kTypeId) {
			LOG_INFO("Collision with another PC: {} state: {}", other_component.owner()->object_id(), System::Enums::ToString(state));
		}
	}
} // namespace Server