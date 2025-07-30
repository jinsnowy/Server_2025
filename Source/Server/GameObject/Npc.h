#pragma once

#include "Server/GameObject/GameObject.h"
#include "Server/GameFramework/CollisionComponent.h"

namespace types {
	class NpcInfo;
} // namespace types

namespace Server {
	class Npc : public GameObject {
	public:
		static const uint32_t kTypeId;

		Npc();
		~Npc();

		void BeginPlay() override;
		void Update(float delta_time) override;

		int64_t hp() const {
			return hp_;
		}

		void set_hp(int64_t hp) {
			hp_ = hp;
		}

		int32_t spawner_id() const {
			return spawner_id_;
		}

		void set_spawner_id(int32_t spawner_id) {
			spawner_id_ = spawner_id;
		}

		void WriteTo(types::NpcInfo* npc_info) const;

		static constexpr float kRadius = 34.0f; // Radius of the capsule collider
		static constexpr float kHalfHeight = 66.0f; // Half height of the capsule collider
		class CollisionComponent* collision_component_ = nullptr; // Optional: Add collision component if needed

		void OnCollision(CollisionComponent& other_component, CollisionState state);

	protected:
		int32_t spawner_id_;
		int64_t hp_;
	};
}


