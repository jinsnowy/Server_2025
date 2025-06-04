#pragma once

#include "Core/System/Exceptions.h"
#include "Core/System/Thenable.h"

namespace System {
	template<typename T>
	class Future {
	public:
		Future() 
			: 
			state_(std::make_shared<Detail::FutureState<T>>()) {
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
		Detail::Thenable<typename FuncReturn<Func>::Type> Then(Func&& func);

		template<typename Func>
		Detail::Thenable<typename FuncReturn<Func>::Type> ThenPost(Func&& func);

	private:
		std::shared_ptr<Detail::FutureState<T>> state_;
	};

	template<>
	class Future<void> {
	private:
		Future()
			: 
			state_(std::make_shared<Detail::FutureState<void>>()) {
		}

		void SetResult() {
			state_->SetResult();
		}

		void SetException(std::exception_ptr exception) {
			state_->SetException(exception);
		}

	private:
	
		std::shared_ptr<Detail::FutureState<void>> state_;
	};

}


