#pragma once

#include "Core/Math/LinAlgebra.h"

namespace Server {
	class GameObjectComponent;
	class GameObject : public std::enable_shared_from_this<GameObject> {
	public:
		GameObject(uint32_t type_id, std::string_view type_name);
		virtual ~GameObject();

		virtual void BeginPlay();
		virtual void Update(float);
		virtual void EndPlay();

		void set_object_id(int64_t object_id) {
			object_id_ = object_id;
		}

		int64_t object_id() const {
			return object_id_;
		}

		const Math::Vec3& position() const {
			return transform_.translation;
		}

		void set_position(const Math::Vec3& position) {
			transform_.translation = position;
		}

		const Math::Vec3& rotation() const {
			return transform_.rotation;
		}

		void set_rotation(const Math::Vec3& rotation) {
			transform_.rotation = rotation;
		}

		const Math::Vec3& scale() const {
			return transform_.scale;
		}

		void set_scale(const Math::Vec3& scale) {
			transform_.scale = scale;
		}

		const Math::Transform& transform() const {
			return transform_;
		}

		void set_transform(const Math::Transform& transform) {
			transform_ = transform;
		}

		Math::Pose GetPose() const {
			return Math::Pose(transform_);
		}

		void SetPose(const Math::Pose& pose) {
			transform_ = pose;
		}

		void SetLifetimeSeconds(float seconds) {
			lifetime_seconds_ = seconds;
		}

		bool IsExpired(const System::Tick& current_tick) const {
			return lifetime_seconds_.has_value() 
				&& (current_tick - created_tick_).AsSecs() >= lifetime_seconds_.value();
		}

		uint32_t type_id() const {
			return type_id_;
		}

		void set_section_id(uint64_t section_id) {
			section_id_ = section_id;
		}

		const System::Tick& created_tick() const {
			return created_tick_;
		}

		void set_created_tick(const System::Tick& tick) {
			created_tick_ = tick;
		}

		void AddComponent(std::unique_ptr<GameObjectComponent> component);

		template<typename T>
		T* GetComponent() {
			uint32_t type_id = T::kTypeId; // Assuming T has a static member kTypeId
			auto it = components_by_type_id_.find(type_id);
			if (it != components_by_type_id_.end()) {
				return static_cast<T*>(it->second.get());
			}
			return nullptr;
		}

	protected:
		uint32_t type_id_;
		std::string type_name_;
		int64_t object_id_;
		uint64_t section_id_ = 0; // Optional section ID if the object is associated with a section
		Math::Transform transform_; // Transform for position, rotation, and scale
		System::Tick created_tick_;
		std::optional<float> lifetime_seconds_; // Optional tick for expiration, if applicable
		std::vector<GameObjectComponent*> components_;
		std::unordered_map<uint32_t, std::unique_ptr<GameObjectComponent>> components_by_type_id_; // Map for fast access to components by type ID
	};
}
