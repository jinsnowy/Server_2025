#pragma once

#include "Core/System/Channel.h"
#include "Core/System/Callable.h"
#include "Core/System/FuncTraits.h"

namespace System {
	class Channel;
	class Actor : public std::enable_shared_from_this<Actor> {
	public:

		Actor();
		Actor(const Channel& channel);
		
		virtual ~Actor() = default;

		bool IsSynchronized() const;
		Channel GetChannel() const;

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<Actor, T>>>
		static std::shared_ptr<T> GetShared(T* ptr) {
			return std::static_pointer_cast<T>(ptr->shared_from_this());
		}

	private:
		Channel channel_;
	};

	namespace Detail {
		template<typename F,
			typename A,
			typename FArg = typename std::tuple_element<0, typename FuncTraits<F>::FArgsType>::type,
			typename = std::enable_if_t<std::is_base_of_v<A, std::remove_reference_t<FArg>>>>
			class PostMessage : public Callable {
			public:
				PostMessage(F&& f, const std::shared_ptr<A>& a)
					:
					func_(std::forward<F>(f)),
					actor_(std::move(a)) {
				}

				void operator()() override {
					(func_)(static_cast<FArg&>(*actor_));
				}

			private:
				F func_;
				std::shared_ptr<A> actor_;
		};
	}

	template<typename T>
	class Future;

	template<typename A>
	struct ActorController {
		A& actor;

		ActorController(A& a);

		std::shared_ptr<A> GetShared();

		template<typename F>
		void Post(F&& func);

		template<typename F>
		void Patch(F&& func);

		template<typename F>
		Future<typename FuncTraits<F>::ReturnType> Async(F&& func);
	};

	template<typename A>
	inline ActorController<A>::ActorController(A& a)
		:
		actor(a) {
	}

	template<typename A>
	inline std::shared_ptr<A> ActorController<A>::GetShared() {
		return Actor::GetShared(&actor);
	}

	template<typename A>
	template<typename F>
	inline void ActorController<A>::Post(F&& func) {
		const Channel& channel = actor.GetChannel();
		channel.Post(std::make_unique<Detail::PostMessage<F, A>>(std::forward<F>(func), GetShared()));
	}

	template<typename A>
	template<typename F>
	inline void ActorController<A>::Patch(F&& func) {
		const Channel& channel = actor.GetChannel();
		if (channel.IsSynchronized()) {
			func(actor);
		}
		else {
			channel.Post(std::make_unique<Detail::PostMessage<F, A>>(std::forward<F>(func), GetShared()));
		}
	}
}

#define Ctrl System::ActorController
