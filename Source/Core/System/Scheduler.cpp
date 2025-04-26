#include "stdafx.h"
#include "Scheduler.h"
#include "Core/System/Singleton.h"
#include "Core/System/Channel.h"
#include "Core/System/Context.h"

namespace System {

thread_local Scheduler* current_scheduler_ = nullptr;

class SchedulerStorage : public System::Singleton<SchedulerStorage> {
public:
    struct SchedulerThreadPair {
        std::unique_ptr<Scheduler> scheduler;
        std::thread thread;
    };

    SchedulerStorage() {
    }

    ~SchedulerStorage() {
        for (auto& element : schedulers_) {
            if (element.scheduler == nullptr) {
				continue;
			}
            element.scheduler->Stop();
            element.thread.join();
        }
    }

    void AddScheduler(std::unique_ptr<Scheduler> scheduler, std::thread thread) {
        int32_t index = counter_.fetch_add(1, std::memory_order_acq_rel);
        scheduler->scheduler_index_ = index;
        schedulers_[index] = SchedulerThreadPair{std::move(scheduler), std::move(thread)};
    }

    int GetSchedulerCount() const {
        return counter_.load(std::memory_order_relaxed);
    }

    Scheduler& GetSchedulerByIndex(int32_t index) {
        return *schedulers_[index % GetSchedulerCount()].scheduler.get();
    }

    Scheduler& GetSchedulerByRoundRobin() {
        int32_t index = round_robin_index_.fetch_add(1, std::memory_order_relaxed);
        return GetSchedulerByIndex(index);
    }

private:
    std::atomic<int> counter_;
    std::atomic<int> round_robin_index_;
    std::array<SchedulerThreadPair, 1024> schedulers_;
};

Scheduler::Scheduler() 
    :
    context_(std::make_shared<System::Context>()) {
}

Scheduler::~Scheduler() {
    Stop();
}

void Scheduler::Launch(int thread_count) {
    LOG_INFO("Scheduler::Launch thread_count: {}", thread_count);
    for (int i = 0; i < thread_count; ++i) {
        std::promise<std::unique_ptr<Scheduler>> promise;
        auto thread = std::thread([&promise, thread_id = i + 1]() mutable {
            auto scheduler = std::make_unique<Scheduler>();
            current_scheduler_ = scheduler.get();
            current_scheduler_->thread_id_ = thread_id;
            promise.set_value(std::move(scheduler));
            current_scheduler_->Run();
            current_scheduler_ = nullptr;
        });
        auto scheduler = promise.get_future().get();
        SchedulerStorage::GetInstance().AddScheduler(std::move(scheduler), std::move(thread));
    }
}

void Scheduler::Destroy() {
    int32_t scheduler_count = SchedulerStorage::GetInstance().GetSchedulerCount();
    for (int32_t i = 0; i < scheduler_count; ++i) {
        SchedulerStorage::GetInstance().GetSchedulerByIndex(i).Stop();
    }
    LOG_INFO("Scheduler::Destroy thread_count: {}", scheduler_count);
}

Scheduler& Scheduler::RoundRobin() {
    return SchedulerStorage::GetInstance().GetSchedulerByRoundRobin();
}

Scheduler& Scheduler::Current() {
    return *current_scheduler_;
}

int32_t Scheduler::ThreadId() {
    if (current_scheduler_ == nullptr) {
        return 0;
    }
    return current_scheduler_->thread_id_;
}

void Scheduler::Run() {
    while (is_running_) {
        context_->RunFor(60);
    }
}

void Scheduler::Stop() {
    if (!is_running_) {
		return;
	}
    is_running_ = false;
    context_->Run();
    context_->Stop();
}

void Scheduler::Post(std::function<void()> task) {
    context_->Post(std::move(task));
}

}

