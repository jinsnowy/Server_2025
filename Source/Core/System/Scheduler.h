#pragma once

#include "Core/Concurrency/MPSC.h"

namespace boost {
    namespace asio {
        class io_context;
    }
}

namespace System {
class Scheduler {
public:
    Scheduler();
    ~Scheduler();

    static void Launch(int thread_count = std::thread::hardware_concurrency());
    static void Destroy();
    static Scheduler& RoundRobin();
    static Scheduler& Current();
    static Scheduler& Get(const boost::asio::io_context& io_context);
    static int32_t ThreadId();

    void Post(std::function<void()> task);

    boost::asio::io_context& GetIoContext();

    int32_t thread_id() const { return thread_id_; }

private:
    friend class SchedulerStorage;

    bool is_running_ = true;
    int32_t thread_id_ = 0;
    std::unique_ptr<boost::asio::io_context> io_context_;
  
    void Run();
    void Stop();
};

}
