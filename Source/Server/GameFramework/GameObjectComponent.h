#pragma once

#include "Core/System/TypeInfo.h"

namespace Server {
	class GameObject;
	class GameObjectComponent  {
	public:
		GameObjectComponent(uint32_t type_id);
		virtual ~GameObjectComponent();

		uint32_t type_id() const {
			return type_id_;
		}

		uint64_t instance_id() const {
			return instance_id_;
		}

		template<typename T>
		T* Cast() {
			if (type_id_ == System::hashcode<std::decay_t<T>>()) {
				return static_cast<T*>(this);
			}
			return nullptr;
		}

		void set_owner(GameObject* owner) {
			owner_ = owner;
		}

		virtual void BeginPlay() = 0;
		virtual void Update(float delta_time) = 0;

	private:
		GameObject* owner_ = nullptr; // Pointer to the GameObject that owns this component
		uint32_t type_id_ = 0;
		uint64_t instance_id_ = 0;
	};
}

