#pragma once

namespace Server {
	class Section;
	class GameObject;
	class SectionComponent {
	public:
		virtual ~SectionComponent() = default;

		virtual void OnSectionCreated(Section&) = 0;
		virtual void OnSectionDestroyed(Section&) = 0;

		virtual void OnTick(float delta_time) = 0;

		virtual void OnGameObjectAdded(std::shared_ptr<GameObject>) = 0;
		virtual void OnGameObjectRemoved(std::shared_ptr<GameObject>) = 0;
	};
}
