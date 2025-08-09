#pragma once

#include "Core/System/Exceptions.h"
#include "Core/System/Function.h"

namespace System {
namespace Detail {
	template<typename R>
	class Thenable;

	class FutureBase {
	public:
		virtual ~FutureBase() = default;

		void OnException(const std::exception& e) const;
		void OnExceptionNoThrow(const std::exception& e) const;

		void SetExceptionCallback(std::function<void(const std::exception&)> callback) {
			exception_callback_ = std::move(callback);
		}

	private:
		std::function<void(const std::exception&)> exception_callback_;
	};

	template<typename T>
	class FutureState : public FutureBase {
	public:
		FutureState() = default;
		FutureState(T&& result)
			:
			result_(std::move(result)) {
		}
		FutureState(const T& result)
			:
			result_(result) {
		}
		~FutureState();

		void SetResult(const T& result) {
			if constexpr (std::is_void_v<T>) {
				result_ = std::monostate{};
			}
			else {
				result_ = result;
			}
		}

		void SetResult(T&& result) {
			if constexpr (std::is_void_v<T>) {
				result_ = std::monostate{};
			}
			else if constexpr (std::is_move_constructible_v<T>) {
				result_ = std::move(result);
			}
			else {
				result_ = result;
			}
		}

		void SetException(std::exception_ptr exception) {
			result_ = exception;
		}

		void SetCallback(Function<void(T)>&& callback) {
			callback_ = std::move(callback);
		}

	private:
		std::variant<T, std::exception_ptr> result_;
		Function<void(T)> callback_;
	};

	template<typename T>
	inline FutureState<T>::~FutureState() {
		if (std::holds_alternative<T>(result_)) {
			if (callback_) {
				callback_(std::move(std::get<T>(result_)));
			}
		}
		else if (std::holds_alternative<std::exception_ptr>(result_)) {
			try {
				std::rethrow_exception(std::get<std::exception_ptr>(result_));
			}
			catch (const ActorNullException& e) {
				OnExceptionNoThrow(e);
			}
			catch (const std::exception& e) {
				OnException(e);
			}
		}
		else {
			try {
				throw FutureNoResultException();
			}
			catch (const FutureNoResultException& e) {
				OnExceptionNoThrow(e);
			}
		}
	}

	template<>
	class FutureState<void> : public FutureBase {
	public:
		~FutureState() {
			if (std::holds_alternative<std::monostate>(result_)) {
				if (callback_) {
					callback_();
				}
			}
			else if (std::holds_alternative<std::exception_ptr>(result_)) {
				try {
					std::rethrow_exception(std::get<std::exception_ptr>(result_));
				}
				catch (const ActorNullException& e) {
					OnExceptionNoThrow(e);
				}
				catch (const std::exception& e) {
					OnException(e);
				}
			}
			else {
				try {
					throw FutureNoResultException();
				}
				catch (const FutureNoResultException& e) {
					OnExceptionNoThrow(e);
				}
			}
		}

		void SetResult() {
			result_ = std::monostate{};
		}

		void SetException(std::exception_ptr exception) {
			result_ = exception;
		}

		void SetCallback(Function<void()>&& callback) {
			callback_ = std::move(callback);
		}

	private:
		std::variant<std::monostate, std::exception_ptr> result_;
		Function<void()> callback_;
	};
}
}