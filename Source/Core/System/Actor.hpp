#pragma once

#include "Core/System/Actor.h"
#include "Core/System/Future.h"
#include "Core/System/Detail/IMessage.h"

namespace System {
	namespace Detail {
		template<typename A>
		class PostMessage : public IMessage {
		public:
			template<typename F>
			PostMessage(F&& func, const std::shared_ptr<A>& a, const void* signature)
				:
				func_(std::forward<F>(func)),
				actor_(std::move(a)),
				signature_(signature) {
			}

			void Execute() override {
				(func_)(static_cast<A&>(*actor_));
			}

			const void* signature() const override {
				return signature_;
			}

		private:
			Function<void(A&)> func_;
			std::shared_ptr<A> actor_;
			const void* signature_ = nullptr;
		};

		template<
		typename A,
		typename R>
		class AsyncMessage : public IMessage {
		public:
			template<typename F>
			AsyncMessage(F&& f, const std::shared_ptr<A>& a, const void* signature)
				:
				func_(std::forward<F>(f)),
				actor_(std::move(a)),
				future_(),
				signature_(signature)
			{
			}

			void Execute() override {
				try {
					if constexpr (std::is_void_v<R>) {
						(func_)(static_cast<A&>(*actor_));
						future_.SetResult();
					}
					else {
						future_.SetResult((func_)(static_cast<A&>(*actor_)));
					}
				}
				catch (...) {
					future_.SetException(std::current_exception());
				}
			}

			Future<R> future() {
				return future_;
			}

			const void* signature() const override {
				return signature_;
			}

		private:
			Function<R(A&)> func_;
			std::shared_ptr<A> actor_;
			Future<R> future_;
			const void* signature_ = nullptr;
		};
	}

	template<typename A>
	inline ActorController<A>::ActorController(A& a, const void* signature)
		:
		actor_(a),
		signature_(signature) {
	}

	template<typename A>
	template<typename F>
	inline void ActorController<A>::Post(F&& func) {
		Channel& channel = actor_.GetChannel();
		if (channel.IsSynchronized()) {
			func(actor_);
		}
		else {
			channel.Push(new Detail::PostMessage<A>(std::forward<F>(func), ::SharedFrom(&actor_), signature_));
		}
	}

	template<typename A>
	template<typename F>
	inline void ActorController<A>::Patch(F&& func) {
		Channel& channel = actor_.GetChannel();
		channel.Push(new Detail::PostMessage<A>(std::forward<F>(func), ::SharedFrom(&actor_), signature_));
	}

	template<typename A>
	template<typename F>
	inline Future<Detail::AsyncResult<F>> ActorController<A>::Async(F&& func) {
		using R = Detail::AsyncResult<F>;
		auto message = new Detail::AsyncMessage<A, R>(std::forward<F>(func), ::SharedFrom(&actor_), signature_);
		auto future = message->future();
		Channel& channel = actor_.GetChannel();
		channel.Push(message);
		return future;
	}
}
