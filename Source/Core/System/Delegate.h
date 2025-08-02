#pragma once

#include "Core/System/Detail/AnyInvocable.h"

namespace System {
	namespace Detail {
		template<typename T, typename Signature, typename ...Args> 
		struct WeakLambda {
			std::weak_ptr<T> owner;
			AnyInvocable<Signature> inner_invocable;

			WeakLambda(const std::weak_ptr<T>& owner_in, Signature callback)
				:
				owner(owner_in),
				inner_invocable(callback) {
			}
			
			WeakLambda(const std::shared_ptr<T>& owner_in, Signature callback)
				:
				owner(owner_in),
				inner_invocable(callback) {
			}

			void operator()(Args&&... args) {
				auto owner_ptr = owner.lock();
				if (owner_ptr) {
					inner_invocable(owner_ptr.get(), std::forward<Args>(args)...);
				}
			}
		};

		template<typename T, typename Signature, typename ...Args>
		struct StrongLambda {
			std::shared_ptr<T> owner;
			AnyInvocable<Signature> inner_invocable;

			StrongLambda(const std::shared_ptr<T>& owner_in, Signature callback)
				:
				owner(owner_in),
				inner_invocable(callback) {
			}

			void operator()(Args&&... args) {
				if (owner) {
					inner_invocable(owner.get(), std::forward<Args>(args)...);
				}
			}
		};
	}

	template<typename ...Args>
	class Delegate {
	public:
		using Callback = AnyInvocable<void(Args...)>;

		void BindFunction(Callback&& callable) {
			callback_ = std::make_unique<Callback>(std::move(callable));
		}

		template<typename T>
		void BindFunction(T* owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			callback_ = std::make_unique<Callback>(Callback(Detail::StrongLambda<T, InnerSignature, Args...>(owner, member_func)));
		}

		template<typename T>
		void BindWeak(const std::weak_ptr<T>& owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			callback_ = std::make_unique<Callback>(Callback(Detail::WeakLambda<T, InnerSignature, Args...>(owner, member_func)));
		}

		template<typename T>
		void BindWeak(const std::shared_ptr<T>& owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			callback_ = std::make_unique<Callback>(Callback(Detail::WeakLambda<T, InnerSignature, Args...>(owner, member_func)));
		}

		template<typename T>
		void BindSP(const std::shared_ptr<T>& owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			callback_ = std::make_unique<Callback>(Callback(Detail::StrongLambda<T, InnerSignature, Args...>(owner, member_func)));
		}

		void ExecuteIfBound(Args&&... args) {
			if (callback_) {
				(*callback_)(std::forward<Args>(args)...);
			}
		}

		void Execute(Args&&... args) {
			(*callback_)(std::forward<Args>(args)...);
		}

		bool IsBound() const {
			return callback_ != nullptr;
		}

		void Unbind() {
			callback_.reset();
		}

	private:
		std::unique_ptr<Callback> callback_;
	};

	template<typename ...Args>
	class MulticastDelegate {
	public:
		using Callback = AnyInvocable<void(Args...)>;

		Callback* BindFunction(Callback&& callable) {
			return callbacks_.emplace_back(new Callback(std::move(callable))).get();
		}

		template<typename T>
		Callback* AddFunction(T* owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			return callbacks_.emplace_back(new Callback(Detail::StrongLambda<T, InnerSignature, Args...>(owner, member_func))).get();
		}

		template<typename T>
		Callback* BindWeak(const std::weak_ptr<T>& owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			return callbacks_.emplace_back(new Callback(Detail::WeakLambda<T, InnerSignature, Args...>(owner, member_func))).get();
		}

		template<typename T>
		Callback* BindWeak(const std::shared_ptr<T>& owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			return callbacks_.emplace_back(new Callback(Detail::WeakLambda<T, InnerSignature, Args...>(owner, member_func))).get();
		}

		template<typename T>
		Callback* BindSP(const std::shared_ptr<T>& owner, void (T::* member_func)(Args...)) {
			using InnerSignature = decltype(member_func);
			return callbacks_.emplace_back(new Callback(Detail::StrongLambda<T, InnerSignature, Args...>(owner, member_func))).get();
		}

		void Broadcast(Args&&... args) {
			for (const auto& callback : callbacks_) {
				(*callback)(std::forward<Args>(args)...);
			}
		}

		bool Remove(Callback* callback) {
			auto it = std::remove_if(callbacks_.begin(), callbacks_.end(),
				[callback](const std::unique_ptr<Callback>& cb) { return cb.get() == callback; });
			if (it != callbacks_.end()) {
				callbacks_.erase(it, callbacks_.end());
				return true;
			}
			return false;
		}

		void Clear() {
			callbacks_.clear();
		}

		size_t Size() const {
			return callbacks_.size();
		}

		bool IsEmpty() const {
			return callbacks_.empty();
		}

	private:
		std::vector<std::unique_ptr<Callback>> callbacks_;
	};
}

