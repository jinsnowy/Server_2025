#include "stdafx.h"
#include "GameObject.h"
#include "Server/GameFramework/GameObjectComponent.h"

namespace Server {
	GameObject::GameObject(uint32_t type_id, std::string_view type_name)
		:
		type_id_(type_id),
		type_name_(type_name),
		object_id_(0LL),
		transform_{},
		created_tick_{} {
	}

	GameObject::~GameObject() = default;

	void GameObject::BeginPlay() {
		for (auto& component : components_) {
			component->set_owner(this);
			component->BeginPlay();
		}
	}

	void GameObject::Update(float delta_time) {
		for (auto& component : components_) {
			component->Update(delta_time);
		}
	}

	void GameObject::EndPlay() {
	}

	void GameObject::AddComponent(std::unique_ptr<GameObjectComponent> component) {
		const uint32_t type_id = component->type_id();
		auto result = components_by_type_id_.try_emplace(type_id, std::move(component));
		if (result.second) {
			components_.emplace_back(result.first->second.get());
		} else {
			LOG_ERROR("Component with type ID {} already exists in GameObject.", type_id);
		}
	}
}