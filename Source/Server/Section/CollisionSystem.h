#pragma once

#include "Server/Section/SectionComponent.h"

namespace Server {
	class Section;
	class GameObject;
	class Collider;
	class CollisionComponent;
	class CollisionSystem : public SectionComponent {
	public:
		CollisionSystem(Section* owner);
		~CollisionSystem();
	
		void OnSectionCreated(Section& section) override;
		void OnSectionDestroyed(Section& section) override;
		void OnTick(float delta_time) override;
		void OnGameObjectAdded(std::shared_ptr<GameObject> object) override;
		void OnGameObjectRemoved(std::shared_ptr<GameObject> object) override;
	
	private:
		Section* owner_;
		std::unordered_map<int64_t, CollisionComponent*> queryable_colliders_;

		static bool IsColliding(const CollisionComponent* lhs, const CollisionComponent* rhs);
	};
}

