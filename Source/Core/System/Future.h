#pragma once

#include "Core/System/Exceptions.h"
#include "Core/System/Thenable.h"
#include "Core/System/Macro.h"
#include "Core/System/Detail/AsyncResult.h"
#include "Core/System/Message.h"

namespace System {
	template<typename T>
	class Future;

	namespace Detail {
		class FutureFactory {
		public:
			template<typename F>
			static std::pair<Future<Detail::AsyncResult<F>>, System::Message*> Create(F&& func) {
				Future<Detail::AsyncResult<F>> future;
				System::Message* message = BUILD_MESSAGE([func = std::forward<F>(func), future]() mutable {
					try {
						auto result = func();
						future.SetResult(std::move(result));
					}
					catch (...) {
						future.SetException(std::current_exception());
					}
				});

				return std::make_pair(std::move(future), message);
			}

			template<typename T>
			static Future<std::remove_cvref_t<T>> FromResult(T&& result);

			static Future<void> FromResult();

			template<typename T, typename Exception>
			static Future<std::remove_cvref_t<T>> FromException(Exception&& exception);
		};

		template<typename T>
		class FutureController;
	}

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

		template<typename _Exception>
		void SetException(const _Exception& exception) {
			state_->SetException(std::make_exception_ptr(exception));
		}

		Detail::FutureController<T> GetController(const void* signature);

	private:
		friend class Detail::FutureFactory;

		Future(T&& result)
			:
			state_(std::make_shared<Detail::FutureState<T>>(std::move(result))) {
		}

		Future(const T& result)
			:
			state_(std::make_shared<Detail::FutureState<T>>(result)) {
		}

		std::shared_ptr<Detail::FutureState<T>> state_;
	};

	template<>
	class Future<void> {
	public:
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

		Detail::FutureController<void> GetController(const void* signature);

	private:
		friend class Detail::FutureFactory;

		Future(std::monostate)
			:
			state_(std::make_shared<Detail::FutureState<void>>()) {
			state_->SetResult();
		}

		std::shared_ptr<Detail::FutureState<void>> state_;
	};

	namespace Detail {
		template<typename T>
		inline Future<std::remove_cvref_t<T>> FutureFactory::FromResult(T&& result) {
			return Future<std::remove_cvref_t<T>>(std::forward<T>(result));
		}

		inline Future<void> FutureFactory::FromResult() {
			return Future<void>(std::monostate{});
		}

		template<typename T, typename Exception>
		inline Future<std::remove_cvref_t<T>> FutureFactory::FromException(Exception&& exception) {
			Future<std::remove_cvref_t<T>> future;
			future.SetException(std::make_exception_ptr(std::forward<Exception>(exception)));
			return future;
		}
	}

	template<typename T>
	static inline decltype(auto) FromResult(T&& result) {
		return Detail::FutureFactory::FromResult(std::forward<T>(result));
	}

	template<typename T>
	static inline Future<std::remove_cvref_t<T>> FromResult(const T& result) {
		return Detail::FutureFactory::FromResult(result);
	}

	static inline Future<void> FromResult() {
		return Detail::FutureFactory::FromResult();
	}
	
}

