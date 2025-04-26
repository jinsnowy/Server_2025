#pragma once

#include "Core/System/Channel.h"
#include "Core/System/Callable.h"
#include "Core/System/FuncTraits.h"

namespace System {
	class Channel;

	template<typename T>
	class Actor : public std::enable_shared_from_this<T> {
	public:
		using ActorClass = Actor<T>;
		
		Actor();
		Actor(const Channel& channel);
		
		virtual ~Actor() = default;

		bool IsSynchronized() const;
		Channel GetChannel() const;

		template<typename F>
		void Post(F&& func);

		std::shared_ptr<T> Get() {
			return std::enable_shared_from_this<T>::shared_from_this();
		}

	private:
		Channel channel_;
	};

	template<typename T>
	inline Actor<T>::Actor()
		:
		channel_(Channel()) {
	}

	template<typename T>
	inline Actor<T>::Actor(const Channel& channel)
		:
		channel_(channel) {
	}

	template<typename T>
	inline bool Actor<T>::IsSynchronized() const {
		return channel_.IsSynchronized();
	}

	template<typename T>
	inline Channel Actor<T>::GetChannel() const {
		return Channel(channel_.GetContext());
	}

	template<typename F,
		typename A, 
		typename FArg = typename std::tuple_element<0, typename FuncTraits<F>::FArgsType>::type,
		typename = std::enable_if_t<std::is_base_of_v<A, std::remove_reference_t<FArg>>>>
	class PostMessage : public Callable {
	public:
		PostMessage(F&& f, std::shared_ptr<A> a) 
			:
			func_(std::forward<F>(f)),
			actor_(std::move(a)) {
		}

		void operator()() override {
			(func_)(static_cast<FArg&>(*actor_));
		}
	private:
		F func_;
		std::shared_ptr<A> actor_;
	};

	template<typename T>
	template<typename F>
	inline void Actor<T>::Post(F&& func) {
		channel_.Post(std::make_unique<PostMessage<F, T>>(std::forward<F>(func), Get()));
	}

}
