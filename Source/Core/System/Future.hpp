#pragma once

#include "Core/System/Future.h"
#include "Core/System/Thenable.h"

namespace System {
	template<typename T>
	template<typename Func>
	inline Detail::Thenable<typename FuncReturn<Func>::Type> Future<T>::Then(Func&& func) {
		using R = typename FuncReturn<Func>::Type;
		Detail::Thenable<R> thenable(state_);
		state_->callback_ = Detail::WhenResult<T, R>(thenable.thenable_state(), std::forward<Func>(func));
		return thenable;
	}

	template<typename T>
	template<typename Func>
	inline Detail::Thenable<typename FuncReturn<Func>::Type> Future<T>::ThenPost(Func&& func) {
		using R = typename FuncReturn<Func>::Type;
		Detail::Thenable<R> thenable(state_);
		state_->callback_ = Detail::WhenResultAndPatch<T, R>(thenable.thenable_state(), std::forward<Func>(func));
		return thenable;
	}
}