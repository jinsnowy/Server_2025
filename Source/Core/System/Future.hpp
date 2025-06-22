#pragma once

#include "Core/System/Future.h"
#include "Core/System/Thenable.h"

namespace System {
	template<typename T>
	template<typename F>
	inline Detail::Thenable<typename FuncTraits<F>::ReturnType> Future<T>::Then(F&& func) {
		using R = typename FuncTraits<F>::ReturnType;
		Detail::Thenable<R> thenable(state_);
		state_->callback_ = Detail::WhenResult<T, R>(thenable.thenable_state(), std::forward<F>(func));
		return thenable;
	}

	template<typename T>
	template<typename F>
	inline Detail::Thenable<typename FuncTraits<F>::ReturnType> Future<T>::ThenPost(F&& func) {
		using R = typename FuncTraits<F>::ReturnType;
		Detail::Thenable<R> thenable(state_);
		state_->callback_ = Detail::WhenResultAndPatch<T, R>(thenable.thenable_state(), std::forward<F>(func));
		return thenable;
	}
}