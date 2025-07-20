#pragma once

#include "Core/System/Channel.h"
#include "Core/System/Callable.h"
#include "Core/System/FuncTraits.h"

namespace System {
	class Channel;
	class Context;
	class Actor : public std::enable_shared_from_this<Actor> {
	public:

		Actor();
		Actor(const Channel& channel);
		
		virtual ~Actor() = default;

		bool IsSynchronized() const;
		Channel GetChannel() const;
		std::shared_ptr<Context> GetContext() const;

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Actor, T>>>
		static std::shared_ptr<T> GetShared(T* ptr) {
			return std::static_pointer_cast<T>(ptr->shared_from_this());
		}

	private:
		Channel channel_;
	};

	template<typename T>
	class Future;

	template<typename A>
	struct ActorController {
		A& actor;

		ActorController(A& a);

		template<typename F>
		void Post(F&& func);

		template<typename F>
		void Patch(F&& func);

		template<typename F>
		Future<typename FuncTraits<F>::ReturnType> Async(F&& func);
	};

	template<typename A>
	static std::shared_ptr<A> MakeShared(A* ptr) {
		return std::static_pointer_cast<A>(ptr->shared_from_this());
	}

	template<typename A>
	static std::shared_ptr<A> MakeShared(A& inst) {
		return std::static_pointer_cast<A>(inst.shared_from_this());
	}
}

#define Ctrl System::ActorController

