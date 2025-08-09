#pragma once

#include "Core/System/Thenable.h"
#include "Core/System/FutureController.h"

namespace System {
	namespace Detail {
		template<typename T>
		template<typename F>
		inline decltype(auto) FutureController<T>::Then(F&& func) {
			using R = Detail::AsyncResult<F>;
			Detail::Thenable<R> thenable(state_, signature_);
			state_->SetCallback(Detail::WhenResultFactory::Create<F, T, R>(thenable, std::forward<F>(func)));
			return std::move(thenable);
		}

		template<typename T>
		template<typename F, typename>
		inline decltype(auto) FutureController<T>::ThenPost(F&& func) {
			using R = Detail::AsyncResult<F>;
			Detail::Thenable<R> thenable(state_, signature_);
			state_->SetCallback(Detail::WhenResultAndPostFactory::Create<F, T, R>(thenable, std::forward<F>(func)));
			return std::move(thenable);
		}
	}
}