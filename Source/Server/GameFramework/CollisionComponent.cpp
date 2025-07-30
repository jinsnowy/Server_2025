#include "stdafx.h"
#include "CollisionComponent.h"
#include "Core/Math/Collision/Collider.h"
#include "Core/Misc/Utils.h"
#include "Server/GameObject/GameObject.h"

namespace Server {
	
	const uint32_t CollisionComponent::kTypeId = System::hashcode<CollisionComponent>();

	CollisionComponent::CollisionComponent(GameObject* owner)
		:
		GameObjectComponent(System::hashcode<CollisionComponent>()),
		owner_(owner),
		collider_(nullptr)
	{
	}

	CollisionComponent::~CollisionComponent() = default;

	void CollisionComponent::set_collider(std::unique_ptr<Math::Collider> collider) {
		collider_ = std::move(collider);
	}

	void CollisionComponent::BeginPlay() {
		if (collider_) {
			collider_->set_center(owner_->position());
			collider_->set_scale(owner_->scale());
			collider_->set_owner(owner_);
		}
	}

	void CollisionComponent::Update(float) {
		if (collider_) {
			collider_->set_center(owner_->position());
			collider_->set_scale(owner_->scale());
		}
	}

	void CollisionComponent::SetCollisionState(int64_t object_id, CollisionComponent& other_comp, CollisionState state) {
		auto it = collision_states_.find(object_id);
		if (it != collision_states_.end()) {
			it->second = state; // Update existing state
		} else {
			collision_states_.emplace(object_id, state); // Add new state
		}
		if (on_collision_callback_) {
			on_collision_callback_(other_comp, state);
		}
	}

	void CollisionComponent::OnCollision(CollisionComponent& other_comp) {
		if (owner_ == nullptr || other_comp.owner_ == nullptr) {
			return;
		}

		auto it = collision_states_.find(other_comp.owner_->object_id());
		if (it != collision_states_.end()) {
			SetCollisionState(other_comp.owner_->object_id(), other_comp, CollisionState::kContinue); // Existing collision continues
		} else {
			SetCollisionState(other_comp.owner_->object_id(), other_comp, CollisionState::kEnter); // New collision detected
		}

		LOG_INFO("Collision detected for GameObject ID: {}, with Collider ID: {}", owner_->object_id(), other_comp.owner_->object_id());
	}
}
