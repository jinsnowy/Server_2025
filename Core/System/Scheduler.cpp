#include "stdafx.h"
#include "Scheduler.h"
#include <boost/asio.hpp>
#include "Core/System/Singleton.h"

namespace System {

thread_local Scheduler* current_scheduler_ = nullptr;

class SchedulerStorage : public System::Singleton<SchedulerStorage> {
public:
    struct Element {
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
        schedulers_[counter_++] = Element{std::move(scheduler), std::move(thread)};
    }

    int GetSchedulerCount() const {
        return counter_.load(std::memory_order_relaxed);
    }

    Scheduler* GetScheduler(int index) {
        return schedulers_[index].scheduler.get();
    }

    Scheduler& GetRoundRobinScheduler() {
        int count = counter_.load(std::memory_order_relaxed);
        int index = round_robin_index_.fetch_add(1, std::memory_order_relaxed) % count;
        return *schedulers_[index].scheduler;
    }

private:
    std::atomic<int> counter_;
    std::atomic<int> round_robin_index_;
    std::array<Element, 128> schedulers_;
};

Scheduler::Scheduler() {
    io_context_ = std::make_unique<boost::asio::io_context>();
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
    int scheduler_count = SchedulerStorage::GetInstance().GetSchedulerCount();
    for (int i = 0; i < scheduler_count; ++i) {
        SchedulerStorage::GetInstance().GetScheduler(i)->Stop();
    }
    LOG_INFO("Scheduler::Destroy thread_count: {}", scheduler_count);
}

Scheduler& Scheduler::RoundRobin() {
    return SchedulerStorage::GetInstance().GetRoundRobinScheduler();
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
    std::function<void()> task;
    while (is_running_) {
        io_context_->run_one_for(std::chrono::milliseconds(10));

        auto current_time = std::chrono::steady_clock::now();
        auto deadline = current_time + std::chrono::milliseconds(30);
        while (task_queue_.TryPop(task)) {
            task();
            current_time = std::chrono::steady_clock::now();
            if (current_time >= deadline) {
                break;
            }
        }
    }
}

void Scheduler::Stop() {
    is_running_ = false;
    io_context_->stop();
}

void Scheduler::Post(std::function<void()> task) {
    task_queue_.Push(std::move(task));
}

boost::asio::io_context& Scheduler::GetIoContext() {
    return *io_context_;
}

}

