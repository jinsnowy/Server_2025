#pragma once

#include "Core/Container/MpSc.h"
#include "Core/System/Function.h"

namespace System {
	class Time;
	class Duration;
	class Tick;
} // namespace System

namespace System {
	class Callable;
	class Context;
	class TimerContext final {
	public:
		static constexpr int64_t kShortTermQueueLength = 1000 * 60 * 5; // 5 minutes
		static constexpr int64_t kMaxFlushCount = 1000; // Limit flush to prevent infinite loops (1s)

		TimerContext(Context* context);
		~TimerContext();

		void Reserve(const int64_t milliseconds, Function<void()> func);
		void Reserve(const Duration& duration, Function<void()> func);
		void ReserveAt(const Tick& time, Function<void()> func);
		void Flush();

	private:
		struct ReserveItem {
			int64_t target_tick;
			Function<void()> func;
		};

		struct ReserveItemComparator {
			bool operator()(const ReserveItem& lhs, const ReserveItem& rhs) const {
				return lhs.target_tick > rhs.target_tick; // Min-heap based on enqueued_tick
			}
		};

		struct ReserveItemNode {
			int64_t target_tick;
			Function<void()> functor;
			ReserveItemNode* next = nullptr;
			ReserveItemNode(Function<void()>&& f, int64_t target_tick)
				:
				target_tick(target_tick),
				functor(std::move(f))
			{
			}
		};

		struct ReserveItemList {
			ReserveItemNode* head = nullptr;
			ReserveItemNode* tail = nullptr;
			~ReserveItemList();

			void Enqueue(Function<void()>&& functor, int64_t target_tick);
			std::optional<ReserveItemNode> Dequeue();
		};

		size_t tick_index_ = 0;
		int64_t tick_base_ = 0;
		std::vector<ReserveItemList> shorterm_queues_;
		std::priority_queue<ReserveItem, std::deque<ReserveItem>, ReserveItemComparator> longterm_queue_;
		System::Context* context_ = nullptr;

		void HotloadFromLongTermQueue();
	};
}


