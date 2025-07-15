#include "stdafx.h"
#include "TimerContext.h"
#include "Core/System/Time.h"
#include "Core/System/Tick.h"
#include "Core/System/Duration.h"
#include "Core/System/Context.h"

namespace System {
	
	TimerContext::TimerContext(Context* context)
		:
		tick_base_(Tick::Current().GetEpocMilliseconds()),
		tick_index_(0ULL),
		queues_{},
		context_(context)
	{
	}

	TimerContext::~TimerContext() {
		queues_.fill(ReserveItemList()); // Clear all queues
		while (!longterm_queue_.empty()) {
			longterm_queue_.pop(); // Clear long term queue
		}
	}

	void TimerContext::ReserveAt(const Tick& time, std::function<void()> func) {
		DEBUG_ASSERT(Context::Current() == context_);
		int64_t target_tick = time.GetEpocMilliseconds();
		size_t index = std::max(0LL, target_tick - tick_base_);
		if (index >= kShortTermQueueLength) {
			longterm_queue_.push({ target_tick, std::move(func) });
			return;
		}

		auto& queue = queues_[(index + tick_index_) % kShortTermQueueLength];
		queue.Enqueue(std::move(func), target_tick);
	}

	void TimerContext::Reserve(int32_t milliseconds, std::function<void()> func) {
		DEBUG_ASSERT(Context::Current() == context_);
		int64_t target_tick = Tick::Current().GetEpocMilliseconds() + milliseconds;
		size_t index = std::max(0LL, target_tick - tick_base_);
		if (index >= kShortTermQueueLength) {
			longterm_queue_.push({target_tick, std::move(func)});
			return;
		}

		auto& queue = queues_[(index + tick_index_) % kShortTermQueueLength];
		queue.Enqueue(std::move(func), target_tick);
	}

	void TimerContext::Reserve(const System::Duration& duration, std::function<void()> func) {
		Reserve(duration.Milliseconds(), std::move(func));
	}

	void TimerContext::Flush() {
		DEBUG_ASSERT(Context::Current() == context_);
		static constexpr int64_t kMaxFlushCount = 1000; // Limit flush to prevent infinite loops (1s)
		int64_t current_tick = Tick::Current().GetEpocMilliseconds();
		size_t flush_tick_count = std::min(kMaxFlushCount, current_tick - tick_base_);
		if (flush_tick_count == 0) {
			return; // No items to flush
		}

		size_t tick_index = tick_index_;
		for (size_t cnt = 0; cnt < flush_tick_count; ++cnt) {
			auto& queue = queues_[tick_index];
			while (auto item = queue.Dequeue()) {
				try {
					(item->functor)(); // Execute the function
				}
				catch (const std::exception& e) {
					LOG_ERROR("TimerContext: Exception in reserved function: {}", e.what());
				}
			}
			tick_index = (tick_index + 1) % kShortTermQueueLength;
		}

		tick_index_ = tick_index;
		tick_base_ += flush_tick_count;

		HotloadFromLongTermQueue();
	}

	void TimerContext::HotloadFromLongTermQueue() {
		while (!longterm_queue_.empty()) {
			auto& item = const_cast<ReserveItem&>(longterm_queue_.top());
			if (item.target_tick > tick_base_) {
				break; // No more items to process
			}
			size_t index = item.target_tick < tick_base_ ? 0 : item.target_tick - tick_base_;
			if (index >= kShortTermQueueLength) {
				continue;
			}
			auto& queue = queues_[(index + tick_index_) % kShortTermQueueLength];
			queue.Enqueue(std::move(item.func), item.target_tick);
			longterm_queue_.pop();
		}
	}

	TimerContext::ReserveItemList::~ReserveItemList() {
		for (auto node = head; node != nullptr;) {
			auto next = node->next;
			delete node;
			node = next;
		}
	}

	void TimerContext::ReserveItemList::Enqueue(Functor&& functor, int64_t target_tick) {
		if (head == nullptr) {
			head = tail = new ReserveItemNode(std::move(functor), target_tick);
		}
		else {
			auto prev = tail;
			tail = new ReserveItemNode(std::move(functor), target_tick);
			prev->next = tail;
		}
	}

	std::optional<TimerContext::ReserveItemNode> TimerContext::ReserveItemList::Dequeue() {
		if (head == nullptr) {
			return std::nullopt; // Queue is empty
		}
		auto node = head;
		head = head->next;
		if (head == nullptr) {
			tail = nullptr; // If the queue is now empty, reset tail
		}
		TimerContext::ReserveItemNode item = std::move(*node);
		delete node;
		return item;
	}


}