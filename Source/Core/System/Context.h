#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace System {
	class Callable;
	class TimerContext;
	class ExecutionContext;
	class Context : public std::enable_shared_from_this<Context> {
	public:
		static Context* Current() { return current_context; }

		Context();
		~Context();

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(Context&&);
		Context& operator=(Context&&);

		void Post(std::function<void()> func) const;
		void Post(std::unique_ptr<Callable> callable) const;

		boost::asio::io_context& io_context() { return *io_context_; }
		const boost::asio::io_context& io_context() const { return *io_context_; }

		TimerContext& timer_context() { return *timer_context_; }
		const TimerContext& timer_context() const { return *timer_context_; }

		ExecutionContext& execution_context() { return *execution_context_; }
		const ExecutionContext& execution_context() const { return *execution_context_; }

		void BeginContext() {
			if (current_context != nullptr) {
				throw std::runtime_error("Context already exists for this thread.");
			}
			current_context = this;
		}

		void EndContext() {
			if (current_context != this) {
				throw std::runtime_error("Context mismatch for this thread.");
			}
			current_context = nullptr;
		}

	private:
		static thread_local Context* current_context;

		friend class Scheduler;
		
		std::unique_ptr<boost::asio::io_context> io_context_;
		std::unique_ptr<TimerContext> timer_context_;
		std::unique_ptr<ExecutionContext> execution_context_;

		void Run();
		size_t RunFor(int32_t milliseconds);
		void Stop();
	};
} // namespace System
