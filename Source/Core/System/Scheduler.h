#pragma once

#include "Core/Container/MPSC.h"
#include "Core/System/Context.h"

namespace System {
class Context;
class Scheduler {
public:
    Scheduler();
    ~Scheduler();

    static void Launch(int thread_count = std::thread::hardware_concurrency());
    static void Destroy();
    static Scheduler& RoundRobin();
    static Scheduler& Current();
    static int32_t ThreadId();

    void Post(std::function<void()> task);

    int32_t thread_id() const { return thread_id_; }
    int32_t scheduler_index() const { return scheduler_index_; }

    std::shared_ptr<Context> GetContext() { return context_; }
    const std::shared_ptr<Context>& GetContext() const { return context_; }

private:
    friend class SchedulerStorage;

    bool is_running_ = true;
    int32_t thread_id_ = 0;
    int32_t scheduler_index_ = 0;
    std::shared_ptr<Context> context_;
  
    void Run();
    void Stop();
};

}
