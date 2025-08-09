#pragma once

#include "Core/System/Channel.h"
#include "Core/System/Detail/AsyncResult.h"
#include "Core/Misc/Utils.h"

namespace System {
	class Context;
	class Actor : public std::enable_shared_from_this<Actor> {
	public:
		Actor();
		Actor(const Channel& channel);
		
		virtual ~Actor() = default;

		uint64_t actor_id() const {
			return actor_id_;
		}

		Channel& GetChannel() {
			return channel_;
		}

		const Channel& GetChannel() const {
			return channel_;
		}

		bool IsSynchronized() const {
			return channel_.IsSynchronized();
		}

	private:
		uint64_t actor_id_ = 0;
		Channel channel_;
	};

	template<typename T>
	class Future;

	template<typename A>
	class ActorController {
	public:
		ActorController(A& a, const void* signature);

		template<typename F>
		void Post(F&& func);

		template<typename F>
		void Patch(F&& func);

		template<typename F>
		Future<Detail::AsyncResult<F>> Async(F&& func);

	private:
		A& actor_;
		const void* signature_ = nullptr;
	};
}

#define Ctrl(actor) System::ActorController(actor, _ReturnAddress())

