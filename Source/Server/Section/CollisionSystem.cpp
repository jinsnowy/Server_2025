#include "stdafx.h"
#include "CollisionSystem.h"
#include "Server/GameObject/GameObject.h"
#include "Server/GameFramework/CollisionComponent.h"
#include "Core/Math/Collision/Collider.h"
#include "Core/Math/Collision/CapsuleCollider.h"

namespace Server {
	CollisionSystem::CollisionSystem(Section* owner)
		:
		owner_(owner)
	{
	}

	CollisionSystem::~CollisionSystem() = default;

	void CollisionSystem::OnSectionCreated(Section&) {
	}
	
	void CollisionSystem::OnSectionDestroyed(Section& ) {
		queryable_colliders_.clear();
	}

	void CollisionSystem::OnTick(float) {
		std::vector<CollisionComponent*> components;
		components.reserve(queryable_colliders_.size());
		for (auto& [object_id, component] : queryable_colliders_) {
			if (component && component->collider()) {
				components.push_back(component);
			}
		}

		for (size_t i = 0; i < components.size(); ++i) {
			CollisionComponent* lhs = static_cast<CollisionComponent*>(components[i]);
			for (size_t j = i + 1; j < components.size(); ++j) {
				CollisionComponent* rhs = static_cast<CollisionComponent*>(components[j]);
				if (IsColliding(lhs, rhs)) {
					lhs->OnCollision(*rhs);
					rhs->OnCollision(*lhs);
				}else if (lhs->HasCollisionState(rhs->owner_->object_id())) {
					lhs->SetCollisionState(rhs->owner_->object_id(), *rhs, CollisionState::kLeave);
					rhs->SetCollisionState(lhs->owner_->object_id(), *lhs, CollisionState::kLeave);
					lhs->RemoveCollisionState(rhs->owner_->object_id());
					rhs->RemoveCollisionState(lhs->owner_->object_id());
				}
			}
		}
	}

	bool CollisionSystem::IsColliding(const CollisionComponent* lhs, const CollisionComponent* rhs) {
		if (!lhs || !rhs || lhs->collider() == nullptr || rhs->collider() == nullptr) {
			return false;
		}
		const Math::CapsuleCollider* lhs_capsule = static_cast<const Math::CapsuleCollider*>(lhs->collider());
		const Math::CapsuleCollider* rhs_capsule = static_cast<const Math::CapsuleCollider*>(rhs->collider());
		return lhs_capsule->CollidesWith(*rhs_capsule);
	}

	void CollisionSystem::OnGameObjectAdded(std::shared_ptr<GameObject> game_object) {
		auto component = game_object->GetComponent<CollisionComponent>();
		if (component) {
			queryable_colliders_.emplace(game_object->object_id(), component);
		}
	}

	void CollisionSystem::OnGameObjectRemoved(std::shared_ptr<GameObject> game_object) {
		auto it = queryable_colliders_.find(game_object->object_id());
		if (it != queryable_colliders_.end()) {

			CollisionComponent* other_comp = it->second;
			for (const auto& [_, component] : queryable_colliders_) {
				if (component->owner_ == game_object.get()) {
					break;
				}
				if (component->HasCollisionState(game_object->object_id())) {
					component->SetCollisionState(game_object->object_id(), *other_comp, CollisionState::kLeave);
					component->RemoveCollisionState(game_object->object_id());
				}
			}

			queryable_colliders_.erase(it);
		}
	}
}

