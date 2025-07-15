#include "stdafx.h"
#include "PeriodicTimer.h"

#include "Core/System/Duration.h"
#include "Core/System/Tick.h"
#include "Core/System/Time.h"
#include "Core/System/Scheduler.h"
#include "Core/System/SingletonActor.h"

namespace System {
	struct PeriodicTimerItem : public std::enable_shared_from_this<PeriodicTimerItem> {
		uint64_t id;
		bool is_running = true;
		int32_t repeated_count = 0;
		System::Tick next_run_time;
		System::Duration period;
		std::function<void(PeriodicTimer::Handle&)> body;

		void Execute();
		void Cancel();
		void Reserve();
	};

	namespace Detail {
		class PeriodicTimerManager : public SingletonActor<PeriodicTimerManager> {
		public:
			std::shared_ptr<PeriodicTimerItem> Schedule(const Duration& period, std::function<void(PeriodicTimer::Handle&)> body, bool first_launch) {
				DEBUG_ASSERT(IsSynchronized());
				auto item = std::make_shared<PeriodicTimerItem>();
				item->id = ++id_counter_;
				item->repeated_count = 0;
				item->is_running = true;
				item->period = period;
				item->body = std::move(body);
				item->next_run_time = first_launch ? Tick::Current() : Tick::Current().AddMilliseconds(period.Milliseconds());
				timers_.emplace(item->id, item);
				item->Reserve();
				return item;
			}

			void Remove(uint64_t id) {
				DEBUG_ASSERT(IsSynchronized());
				auto it = timers_.find(id);
				if (it != timers_.end()) {
					timers_.erase(it);
				}
			}

		private:
			uint64_t id_counter_ = 0;
			std::unordered_map<uint64_t, std::shared_ptr<PeriodicTimerItem>> timers_;
		};
	}

	void PeriodicTimerItem::Execute() {
		PeriodicTimer::Handle handle(new std::weak_ptr<PeriodicTimerItem>(shared_from_this()));

		try {
			body(handle);
		}
		catch (const std::exception& e) {
			LOG_ERROR("[PeriodicTimerItem] exeception occurs : {}", e.what());
		}

		if (is_running) {
			++repeated_count;
			next_run_time = next_run_time.AddMilliseconds(period.Milliseconds());
			Reserve();
		}
	}

	void PeriodicTimerItem::Cancel() {
		is_running = false;
		Ctrl(Detail::PeriodicTimerManager::GetInstance()).Post([id = id](Detail::PeriodicTimerManager& manager) {
			manager.Remove(id);
		});
	}

	void PeriodicTimerItem::Reserve() {
		std::weak_ptr<PeriodicTimerItem> weak_self = shared_from_this();
		Scheduler::ReserveAt(next_run_time, [weak_self]() {
			auto self = weak_self.lock();
			if (self && self->is_running) {
				self->Execute();
			}
		});
	}

	PeriodicTimer::Handle::~Handle() {
		Release();
	}

	void PeriodicTimer::Handle::Cancel() {
		if (handle_ == nullptr) {
			return;
		}
		auto ref = reinterpret_cast<std::weak_ptr<PeriodicTimerItem>*>(handle_);
		if (auto item = ref->lock()) {
			item->Cancel();
		}
		Release();
	}

	void PeriodicTimer::Handle::Release() {
		if (handle_ == nullptr) {
			return;
		}
		auto ref = reinterpret_cast<std::weak_ptr<PeriodicTimerItem>*>(handle_);
		delete ref;
		handle_ = nullptr;
	}

	PeriodicTimer::Handle PeriodicTimer::Schedule(const System::Duration& period, std::function<void(PeriodicTimer::Handle&)> body, bool first_launch) {
		auto item = Detail::PeriodicTimerManager::GetInstance().Schedule(period, std::move(body), first_launch);
		return Handle(new std::weak_ptr<PeriodicTimerItem>(item));
	}
}