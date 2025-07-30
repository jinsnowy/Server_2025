#pragma once

#include "Server/GameObject/GameObject.h"
#include "Server/GameFramework/CollisionComponent.h"

namespace Server {
	class Projectile : public GameObject {
	public:
		static const uint32_t kTypeId;
		static const Math::Vec3 kGravity;

		Projectile();

		void BeginPlay() override;

		void Update(float delta_time) override;

		void set_initial_speed(float speed) {
			initial_speed_ = speed;
		}

		float initial_speed() const {
			return initial_speed_;
		}

		void set_direction(const Math::Vec3& direction) {
			direction_ = direction;
		}

		const Math::Vec3& direction() const {
			return direction_;
		}

		void set_action_id(int64_t action_id) {
			action_id_ = action_id;
		}

		void set_trigger_id(int64_t trigger_id) {
			trigger_id_ = trigger_id;
		}

		int64_t trigger_id() const {
			return trigger_id_;
		}

		std::vector<Math::Vec3> simulate_trajectory(float delta_time, int32_t steps) const {
			std::vector<Math::Vec3> trajectory;
			Math::Vec3 position = this->position();
			Math::Vec3 velocity = direction_ * initial_speed_;
			Math::Vec3 old_velocity = velocity;
			for (int32_t i = 0; i < steps; ++i) {
				trajectory.push_back(position);
				velocity += kGravity * delta_time;
				position += 0.5f * (velocity + old_velocity) * delta_time;
			}
			return trajectory;
		}

		int64_t damage() const {
			return damage_;
		}

		void set_damage(int32_t damage) {
			damage_ = damage;
		}

		void WriteTo(types::ProjectileInfo* out_info) const;

		Math::Vec3 GetExpectedPosition(const System::Tick& tick) const;

		void HitObject(const std::shared_ptr<GameObject>& hit_object);

	private:
		int64_t trigger_id_ = 0; // Unique identifier for the trigger that launched this projectile
		float initial_speed_ = 0.f;
		Math::Vec3 direction_ = Math::Vec3(0.f, 0.f, 0.f);
		Math::Vec3 velocity_ = Math::Vec3(0.f, 0.f, 0.f); // Current velocity of the projectile
		int64_t action_id_ = 0;
		int64_t damage_ = 0; // Optional damage value, if applicable

		static constexpr float kRadius = 22.0f; // Radius of the capsule collider
		static constexpr float kHalfHeight = 44.0f; // Half height of the capsule collider
		class CollisionComponent* collision_component_ = nullptr; // Optional: Add collision component if needed

		void OnCollision(CollisionComponent& other_component, CollisionState state);
	};
}

