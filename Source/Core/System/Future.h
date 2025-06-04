#pragma once

#include "Core/System/Exceptions.h"
#include "Core/System/Thenable.h"

namespace System {
	template<typename T>
	class Future {
	public:
		Future() 
			: 
			state_(std::make_shared<FutureState<T>>()) {
		}

		void SetResult(const T& result) {
			state_->SetResult(result);
		}

		void SetResult(T&& result) {
			state_->SetResult(std::move(result));
		}

		void SetException(std::exception_ptr exception) {
			state_->SetException(exception);
		}

		template<typename Func>
		decltype(auto) Then(Func&& func) {
			using R = typename FuncReturn<Func>::Type;
			Thenable<R> thenable(state_);
			state_->callback_ = Detail::WhenResult<T, R>(thenable.thenable_state(), std::forward<Func>(func));
			return thenable;
		}

	private:
		std::shared_ptr<FutureState<T>> state_;
	};


	template<>
	class Future<void> {
	private:
		Future()
			: 
			state_(std::make_shared<FutureState<void>>()) {
		}

		void SetResult() {
			state_->SetResult();
		}

		void SetException(std::exception_ptr exception) {
			state_->SetException(exception);
		}

	private:
	
		std::shared_ptr<FutureState<void>> state_;
	};

}


