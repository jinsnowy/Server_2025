#pragma once

#include "Core/Container/MPSC.h"
#include "Core/System/Context.h"
#include "Core/System/Future.h"

namespace System {
class Context;
class Tick;
class Message;
class Scheduler {
public:
    Scheduler();
    ~Scheduler();

    static void CreateThreadPool(int thread_count = std::thread::hardware_concurrency());
	static void CreateThread(Function<void()> task);
    static void Destroy();

    static Scheduler& RoundRobin();
    static Scheduler& Current();
    static int32_t ThreadId();

	static void Any(Function<void(Scheduler&)> func);
    static void ForEach(Function<void(Scheduler&)> func);
    static void Reserve(int32_t milliseconds, Function<void()> functor);
    static void ReserveAt(const Tick& tick, Function<void()> functor);

	static bool IsThreadPool();
	static uint16_t ThreadPoolCount();

    void Post(Message* message);
  
    template<typename F>
    decltype(auto) Async(F&& func) {
        auto item = Detail::FutureFactory::Create(std::forward<F>(func));
        Post(std::move(item.second));
        return item.first;
    }

    int32_t thread_id() const { return thread_id_; }
    int32_t scheduler_index() const { return scheduler_index_; }
	Context& context() { return *context_; }
	const Context& context() const { return *context_; }

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
