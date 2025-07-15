#pragma once

#include "Core/Container/MpSc.h"

namespace System {
	class Time;
	class Duration;
	class Tick;
} 

namespace System {
	class Callable;
	class Context;
	class TimerContext {
	public:
		static constexpr int64_t kShortTermQueueLength = 1000 * 60 * 15; // 15 minutes

		TimerContext(Context* context);
		~TimerContext();

		void Reserve(int32_t milliseconds, std::function<void()> func);
		void Reserve(const Duration& duration, std::function<void()> func);
		void ReserveAt(const Tick& time, std::function<void()> func);
		void Flush();

	private:
		using Functor = std::function<void()>;

		struct ReserveItem {
			int64_t target_tick;
			Functor func;
		};

		struct ReserveItemComparator {
			bool operator()(const ReserveItem& lhs, const ReserveItem& rhs) const {
				return lhs.target_tick > rhs.target_tick; // Min-heap based on enqueued_tick
			}
		};

		struct ReserveItemNode {
			int64_t target_tick;
			Functor functor;
			ReserveItemNode* next = nullptr;
			ReserveItemNode(Functor&& f, int64_t target_tick) 
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
			void Enqueue(Functor&& functor, int64_t target_tick);
			std::optional<TimerContext::ReserveItemNode> Dequeue();
		};
		
		size_t tick_index_ = 0;
		int64_t tick_base_ = 0;
		std::array<ReserveItemList, kShortTermQueueLength> queues_;
		std::priority_queue<ReserveItem, std::deque<ReserveItem>, ReserveItemComparator> longterm_queue_;
		System::Context* context_ = nullptr;

		void HotloadFromLongTermQueue();
	};
}


