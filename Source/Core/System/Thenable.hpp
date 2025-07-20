#pragma once

#include "Core/System/Actor.h"
#include "Core/System/Thenable.h"

namespace System {
	namespace Detail {
		template<typename T>
		struct SharedPtrWrapper {
			static constexpr bool value = false;
		};

		template<typename T>
		struct SharedPtrWrapper<std::shared_ptr<T>> {
			using Type = std::remove_cvref_t<T>;
			static constexpr bool value = true;
		};

		template<typename T>
		static constexpr bool IsSharedPtrWrapperV = SharedPtrWrapper<T>::value;

		template<typename F, typename R, typename _Actor>
		struct WhenResultAndPatchActorMessage {
			std::shared_ptr<FutureState<R>> thenable_state;
			F func;

			WhenResultAndPatchActorMessage(const std::shared_ptr<FutureState<R>>& state, F&& f)
				:
				thenable_state(state),
				func(std::forward<F>(f)) {
			}

			void operator()(_Actor& actor) {
				try {
					if constexpr (std::is_void_v<R>) {
						func(actor);
						thenable_state->SetResult();
					}
					else {
						thenable_state->SetResult(func(actor));
					}
				}
				catch (...) {
					thenable_state->SetException(std::current_exception());
				}
			}
		};
			
		template<typename T, typename R, typename F>
		static inline std::function<void(T)> WhenResultAndPatch(std::shared_ptr<FutureState<R>> thenable_state, F&& func) {
			static_assert(std::is_base_of_v<Actor, typename SharedPtrWrapper<T>::Type>, "T must not be an std::shared_ptr<Actor> for WhenResultAndPatch");
			using A = typename SharedPtrWrapper<T>::Type;
			return[thenable_state, func = std::forward<F>(func)](std::shared_ptr<A> shared_actor) mutable {
				if (!shared_actor) {
					thenable_state->SetException(std::make_exception_ptr(ActorNullException()));
					return;
				}
				System::ActorController<A>(*shared_actor).Patch(WhenResultAndPatchActorMessage<F, R, A>(thenable_state, std::forward<F>(func)));
			};
		}

		template<typename R>
		template<typename F>
		inline Thenable<typename FuncTraits<F>::ReturnType> Thenable<R>::Then(F&& func) {
			using R_ = typename FuncTraits<F>::ReturnType;
			Thenable<R_> thenable(thenable_state_);
			thenable_state_->callback_ = Detail::WhenResult<R, R_>(thenable, std::forward<F>(func));
			return thenable;
		}

		template<typename R>
		template<typename F>
		inline Thenable<typename FuncTraits<F>::ReturnType> Thenable<R>::ThenPost(F&& func) {
			using R_ = typename FuncTraits<F>::ReturnType;
			Thenable<R_> thenable(thenable_state_);
			thenable_state_->callback_ = Detail::WhenResultAndPatch<R, R_>(thenable.thenable_state(), std::forward<F>(func));
			return thenable;
		}
	}
}
