#pragma once

#include "Core/System/Actor.h"
#include "Core/System/Thenable.h"

namespace System {
	namespace Detail {
		template<typename T>
		struct SharedPtrWrapper {
		};

		template<typename T>
		struct SharedPtrWrapper<std::shared_ptr<T>> {
			using Type = T;
		};

		template<typename F, typename A, typename R>
		struct WhenResultAndPatchMessage {
			std::shared_ptr<FutureState<R>> thenable_state;
			F func;

			WhenResultAndPatchMessage(std::shared_ptr<FutureState<R>> state, F&& f)
				:
				thenable_state(std::move(state)),
				func(std::forward<F>(f)) {
			}

			void operator()(A& actor) {
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
			return[thenable_state, func = std::forward<F>(func)](T shared_actor) mutable {
				if (!shared_actor) {
					thenable_state->SetException(std::make_exception_ptr(ActorNullException()));
					return;
				}
				System::ActorController<A>(*shared_actor).Patch(WhenResultAndPatchMessage<F, A, R>(thenable_state, std::forward<F>(func)));
			};
		}

		template<typename R>
		template<typename Func>
		inline Thenable<typename FuncReturn<Func>::Type> Thenable<R>::Then(Func&& func) {
			using R_ = typename FuncReturn<Func>::Type;
			Thenable<R_> thenable(thenable_state_);
			thenable_state_->callback_ = Detail::WhenResult<R, R_>(thenable, std::forward<Func>(func));
			return thenable;
		}

		template<typename R>
		template<typename Func>
		inline Thenable<typename FuncReturn<Func>::Type> Thenable<R>::ThenPost(Func&& func) {
			using R_ = typename FuncReturn<Func>::Type;
			Thenable<R_> thenable(thenable_state_);
			thenable_state_->callback_ = Detail::WhenResultAndPatch<R, R_>(thenable.thenable_state(), std::forward<Func>(func));
			return thenable;
		}
	}
}
