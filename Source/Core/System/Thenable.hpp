#pragma once

#include "Core/System/Actor.h"
#include "Core/System/Thenable.h"
#include "Core/System/Detail/Templates.h"
#include "Core/System/Detail/IMessage.h"
#include "Core/System/FutureController.h"

namespace System {
	namespace Detail {
		template<typename F, typename R, typename _R>
		inline void WhenResult<F, R, _R>::operator()(R&& result) {
			if constexpr (std::is_void_v<R>) {
				if constexpr (std::is_void_v<_R>) {
					func_();
					thenable_state_->SetResult();
				}
				else {
					thenable_state_->SetResult(func_());
				}
			}
			else {
				if constexpr (std::is_void_v<_R>) {
					func_(std::move(result));
					thenable_state_->SetResult();
				}
				else {
					thenable_state_->SetResult(func_(std::move(result)));
				}
			}
		}

		template<typename F, typename R, typename _R>
		inline void WhenResultAndPost<F, R, _R>::operator()(R&& result) {
			if (!result) {
				thenable_state_->SetException(std::make_exception_ptr(ActorNullException()));
				return;
			}

			using A = typename SharedPtrWrapper<R>::InnerType;
			ActorController<A> controller(*result, _ReturnAddress());
			controller.Post(WhenResultAndPostBody<std::remove_cvref_t<A>, _R>(std::move(func_), thenable_state_, signature_));
		}

		template<typename A, typename _R>
		inline void WhenResultAndPostBody<A, _R>::operator()(A& actor) {
			if constexpr (std::is_void_v<_R>) {
				func_(actor);
				thenable_state_->SetResult();
			}
			else {
				thenable_state_->SetResult(func_(actor));
			}
		}

		template<typename R>
		inline Detail::FutureController<R> Thenable<R>::GetController(const void* signature) {
			return Detail::FutureController<R>(thenable_state_, signature);
		}
	}
}
