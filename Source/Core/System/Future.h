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

		static Future<T> FromResult(const T& result) {
			return Future<T>(result);
		}	
		
		static Future<T> FromResult(T&& result) {
			return Future<T>(std::move(result));
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

		template<typename F>
		Detail::Thenable<typename FuncTraits<F>::ReturnType> Then(F&& func);

		template<typename F>
		Detail::Thenable<typename FuncTraits<F>::ReturnType> ThenPost(F&& func);

	private:
		std::shared_ptr<Detail::FutureState<T>> state_;

		Future(T&& result)
			: 
			state_(std::make_shared<Detail::FutureState<T>>(std::move(result))) {
		}

		Future(const T& result)
			: 
			state_(std::make_shared<Detail::FutureState<T>>(result)) {
		}
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

	namespace Detail {
		template<typename F>
		using FutureType = ::System::Future<typename FuncTraits<F>::ReturnType>;

		class FutureFactory {
		public:
			template<typename F>
			static std::pair<Future<typename FuncTraits<F>::ReturnType>, std::function<void()>> Create(F&& func) {
				Future<typename FuncTraits<F>::ReturnType> future;
				std::function<void()> callback = [func = std::forward<F>(func), future]() mutable {
					try {
						auto result = func();
						future.SetResult(std::move(result));
					}
					catch (...) {
						future.SetException(std::current_exception());
					}
				};

				return std::make_pair(std::move(future), std::move(callback));
			}
		};
	}
}


