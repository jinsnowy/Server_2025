#pragma once

#include "Core/System/Actor.h"

namespace System {
	template<typename T>
	class SingletonActor : public Actor {
	public:
		SingletonActor();
		SingletonActor(const Channel& channel);

		static T& GetInstance() {
			static SingletonActorInstance instance;
			return *instance;
		}

		SingletonActor(const SingletonActor&) = delete;
		SingletonActor& operator=(const SingletonActor&) = delete;

		SingletonActor(SingletonActor&&) = delete;
		SingletonActor& operator=(SingletonActor&&) = delete;

	private:
		struct SingletonActorInstance {
			std::shared_ptr<T> instance;
			SingletonActorInstance() 
				: 
				instance(std::make_shared<T>()) {
			}
			T& operator*() {
				return *instance;
			}
		};
	};

	template<typename T>
	inline SingletonActor<T>::SingletonActor()
	{
	}

	template<typename T>
	inline SingletonActor<T>::SingletonActor(const Channel& channel)
		: 
		Actor(channel)
	{
	}
} 
