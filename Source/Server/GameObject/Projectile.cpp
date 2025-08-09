#include "stdafx.h"
#include "Projectile.h"
#include "Server/Section/Section.h"
#include "Server/Section/SpawnSystem.h"
#include "Server/Section/SectionRepository.h"
#include "Server/Utilites/Protobuf.h"
#include "Core/Math/Collision/CapsuleCollider.h"
#include "Server/GameFramework/CollisionComponent.h"
#include "Server/GameObject/Npc.h"

namespace Server {
	const uint32_t Projectile::kTypeId = System::hashcode<Projectile>();
	const Math::Vec3 Projectile::kGravity = Math::Vec3(0.f, 0.f, -981.0f);
	
	Projectile::Projectile()
		:
		GameObject(kTypeId, System::type_name_v<Projectile>),
		initial_speed_(0.f),
		direction_(Math::Vec3::Zero()),
		action_id_(0LL),
		trigger_id_(0LL) {
		set_scale(Math::Vec3(0.1f, 1.f, 0.8f)); // Default scale for projectile
		auto collision_comp = std::make_unique<CollisionComponent>(this);
		auto collider = std::make_unique<Math::CapsuleCollider>(transform_.translation, kRadius, kHalfHeight);
		collision_component_ = collision_comp.get();
		collision_component_->set_collider(std::move(collider));
		collision_component_->set_on_collision_callback(
			std::bind(&Projectile::OnCollision, this, std::placeholders::_1, std::placeholders::_2));
		AddComponent(std::move(collision_comp));
	}

	void Projectile::BeginPlay() {
		GameObject::BeginPlay();
		velocity_ = direction_ * initial_speed_;
	}

	void Projectile::Update(float delta_time) {
		GameObject::Update(delta_time);

		// p' = v * dt + 0.5 * g * dt^2
		// p' = v * dt + 0.5 * (v'- v) /dt * dt^2
		// p' = p + v * dt + 0.5 * (v'-v) * dt
		Math::Vec3 old_velocity = velocity_;
		velocity_ += kGravity * delta_time;
		transform_.translation += old_velocity * delta_time + 0.5f * (velocity_ - old_velocity) * delta_time;

		auto collider = collision_component_->collider();
		SectionRepository::FindSection(section_id_).THEN_POST([position = collider->center()](Section& section) {
			auto notify = std::make_shared<world::DebugSimulationPositionNotify>();
			notify->set_color(types::Color::kRed);
			notify->set_shape(types::DebugShape::kSphere);
			notify->mutable_shape_info()->mutable_sphere()->set_radius(15.0f);
			Utilites::WriteTo(position, notify->mutable_position());
			section.Multicast(notify); // 0 means no source session ID
		});
	}

	Math::Vec3 Projectile::GetExpectedPosition(const System::Tick& tick) const {
		auto delta_time = (tick - Section::GetLastTick()).AsSecs();
		if (delta_time > 0.f) {
			Math::Vec3 old_velocity = velocity_;
			Math::Vec3 new_velocity = velocity_ + kGravity * delta_time;
			return position() + old_velocity * delta_time + 0.5f * (new_velocity - old_velocity) * delta_time;
		}

		else {
			// If the delta time is zero or negative, return the current position
			return position();
		}
	}

	void Projectile::HitObject(const std::shared_ptr<GameObject>& hit_object) {
		if (hit_object->type_id() != Npc::kTypeId) {
			LOG_ERROR("Hit object is not a valid Npc type: {}", hit_object->type_id());
			return;
		}

		auto projectile = std::static_pointer_cast<Projectile>(shared_from_this());
		SectionRepository::FindSection(section_id_).THEN_POST([npc = std::static_pointer_cast<Npc>(hit_object), projectile](Section& section) {
			section.DespawnObject(projectile);

			auto notify = std::make_shared<world::HitObjectByProjectileNotify>();
			notify->set_object_id(npc->object_id());
			notify->set_projectile_object_id(projectile->object_id());

			LOG_INFO("object id: {} is hit by projectile : {}", npc->object_id(), projectile->object_id());

			int64_t next_hp = npc->hp() - projectile->damage();
			if (next_hp <= 0) {
				// If the object is dead, remove it
				npc->set_hp(0);
				section.DespawnObject(npc);
				notify->set_is_alive(false);
			}
			else {
				// Update the hit object HP
				npc->set_hp(next_hp);
				notify->set_current_hp(next_hp);
				notify->set_is_alive(true);
			}

			section.Multicast(notify);
		});
	}

	void Projectile::WriteTo(types::ProjectileInfo* out_info) const {
		out_info->set_object_id(object_id());
		out_info->set_initial_speed(initial_speed_);
		out_info->set_trigger_id(trigger_id_);
		out_info->set_action_id(action_id_);
		Utilites::WriteTo(transform(), out_info->mutable_pose());
		Utilites::WriteTo(direction_, out_info->mutable_direction());
		Utilites::WriteTo(velocity_, out_info->mutable_velocity());
		Utilites::WriteTo(created_tick_, out_info->mutable_action_time());
	}

	void Projectile::OnCollision(CollisionComponent& other_component, CollisionState state) {
		if (other_component.owner()->type_id() == Npc::kTypeId) {
			LOG_INFO("Collision with another Npc: {} state: {}", other_component.owner()->object_id(), System::Enums::ToString(state));
			HitObject(other_component.owner()->shared_from_this());
		}
	}
	// cm/s^2
}
