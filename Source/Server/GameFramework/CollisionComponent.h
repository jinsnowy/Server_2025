#pragma once

#include "Server/GameFramework/GameObjectComponent.h"

namespace Math {
	class Collider;
} // namespace Math

namespace Server {
	enum class CollisionState
	{
		kNone = 0,
		kEnter,
		kContinue,
		kLeave,
	};

	class GameObject;
	class CollisionComponent : public GameObjectComponent {
	public:
		static const uint32_t kTypeId;

		CollisionComponent(GameObject* owner);
		~CollisionComponent();

		void set_collider(std::unique_ptr<Math::Collider> collider);
		
		const Math::Collider* collider() const {
			return collider_.get();
		}
		
		Math::Collider* collider() {
			return collider_.get();
		}
		
		virtual void BeginPlay();
		virtual void Update(float);

		void RemoveCollisionState(int64_t object_id) {
			collision_states_.erase(object_id);
		}

		bool HasCollisionState(int64_t object_id) const {
			return collision_states_.find(object_id) != collision_states_.end();
		}

		void SetCollisionState(int64_t object_id, CollisionComponent& other_comp, CollisionState state);

		void set_on_collision_callback(std::function<void(CollisionComponent&, CollisionState)> callback) {
			on_collision_callback_ = std::move(callback);
		}

		const GameObject* owner() const {
			return owner_;
		}

		GameObject* owner() {
			return owner_;
		}

	private:
		friend class CollisionSystem;

		GameObject* owner_;
		std::unique_ptr<Math::Collider> collider_;
		std::unordered_map<int64_t, CollisionState> collision_states_;
		std::function<void(CollisionComponent&, CollisionState)> on_collision_callback_;

		void OnCollision(CollisionComponent& other_component);
	};
} // namespace Server

