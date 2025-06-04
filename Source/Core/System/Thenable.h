#pragma once

#include "Core/System/FuncTraits.h"
#include "Core/System/Exceptions.h"

namespace System {
	namespace Detail {
		template<typename R>
		class Thenable;

		struct FutureBase {
			std::function<void(const std::exception&)> exception_callback_;
			virtual ~FutureBase() = default;
			void OnException(const std::exception& e) const;
		};

		template<typename T>
		struct FutureState : public FutureBase {
			std::variant<T, std::exception_ptr> result_;
			std::function<void(T)> callback_;

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
				else if constexpr (std::is_move_assignable_v<T>) {
					result_ = std::move(result);
				}
				else {
					result_ = result;
				}
			}

			void SetException(std::exception_ptr exception) {
				result_ = exception;
			}
		};

		template<typename T>
		inline FutureState<T>::~FutureState() {
			if (std::holds_alternative<T>(result_)) {
				if constexpr (std::is_move_assignable_v<T>) {
					if (callback_) {
						callback_(std::move(std::get<T>(result_)));
					}
				}
				else {
					if (callback_) {
						callback_(std::get<T>(result_));
					}
				}
			}
			else if (std::holds_alternative<std::exception_ptr>(result_)) {
				try {
					std::rethrow_exception(std::get<std::exception_ptr>(result_));
				}
				catch (const std::exception& e) {
					OnException(e);
				}
			}
			else {
				if (exception_callback_) {
					exception_callback_(FutureNoResultException{});
				}
			}
		}

		template<>
		struct FutureState<void> : public FutureBase {
			std::variant<std::monostate, std::exception_ptr> result_;
			std::function<void()> callback_;

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
					catch (const std::exception& e) {
						OnException(e);
					}
				}
				else {
					if (exception_callback_) {
						exception_callback_(FutureNoResultException{});
					}
				}
			}

			void SetResult() {
				result_ = std::monostate{};
			}

			void SetException(std::exception_ptr exception) {
				result_ = exception;
			}
		};

		template<typename T, typename R, typename Func>
		static inline std::function<void(T)> WhenResult(std::shared_ptr<FutureState<R>> thenable_state, Func&& func) {
			return[thenable_state, func = std::forward<Func>(func)](T result) mutable {
				if constexpr (std::is_void_v<T>) {
					if constexpr (std::is_void_v<R>) {
						func();
						thenable_state->SetResult();
					}
					else {
						thenable_state->SetResult(func());
					}
				}
				else if constexpr (std::is_move_assignable_v<T>) {
					if constexpr (std::is_void_v<R>) {
						func(std::move(result));
						thenable_state->SetResult();
					}
					else {
						thenable_state->SetResult(func(result));
					}
				}
				else {
					if constexpr (std::is_void_v<R>) {
						func(std::move(result));
						thenable_state->SetResult();
					}
					else {
						thenable_state->SetResult(func(result));
					}
				}
			};
		}

		template<typename R>
		class Thenable {
		public:
			Thenable(std::shared_ptr<FutureBase> waiting_state)
				:
				waiting_state_(waiting_state) {
			}

			Thenable& Catch(std::function<void(const std::exception&)> on_exception) {
				waiting_state_->exception_callback_ = on_exception;
				return *this;
			}

			template<typename Func>
			Thenable<typename FuncReturn<Func>::Type> Then(Func&& func);

			template<typename Func>
			Thenable<typename FuncReturn<Func>::Type> ThenPost(Func&& func);

			std::shared_ptr<FutureState<R>> thenable_state() const {
				return thenable_state_;
			}

		private:
			std::shared_ptr<FutureBase> waiting_state_;
			std::shared_ptr<FutureState<R>> thenable_state_ = std::make_shared<FutureState<R>>();
		};
	} // namespace Detail
}
