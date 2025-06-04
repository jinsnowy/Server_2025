#pragma once

#include "Core/System/Actor.h"
#include "Core/System/Future.h"

namespace System {
	namespace Detail {
		template<
		typename F,
		typename A,
		typename R,
		typename FArg = typename std::tuple_element<0, typename FuncTraits<F>::FArgsType>::type,
		typename = std::enable_if_t<std::is_base_of_v<A, std::remove_reference_t<FArg>>>>
		class AsyncMessage : public Callable {
		public:
			AsyncMessage(F&& f, const std::shared_ptr<A>& a, const Future<R>& future)
				:
				func_(std::forward<F>(f)),
				actor_(std::move(a)),
				future_(future)
			{
			}

			void operator()() override {
				try {
					if constexpr (std::is_void_v<R>) {
						(func_)(static_cast<FArg&>(*actor_));
						future_.SetResult();
					}
					else {
						R result = (func_)(static_cast<FArg&>(*actor_));
						future_.SetResult(std::move(result));
					}
				}
				catch (...) {
					future_.SetException(std::current_exception());
				}
			}

			Future<R> future() {
				return future_;
			}

		private:
			F func_;
			std::shared_ptr<A> actor_;
			Future<R> future_;
		};
	}

	template<typename A>
	template<typename F>
	inline Future<typename FuncTraits<F>::ReturnType> ActorController<A>::Async(F&& func) {
		using R = typename FuncReturn<F>::Type;
		Future<R> future;
		const Channel& channel = actor.GetChannel();
		channel.Post(std::make_unique<Detail::AsyncMessage<F, A, R>>(std::forward<F>(func), Actor::GetShared(&actor), future));
		return future;
	}
}
