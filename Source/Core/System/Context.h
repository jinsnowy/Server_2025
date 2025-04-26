#pragma once

#include "Core/ThirdParty/BoostAsio.h"

namespace System {
	class Callable;
	class Context : public std::enable_shared_from_this<Context> {
	public:
		static Context* Current() { return current_context; }

		Context();
		~Context();

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(Context&&);
		Context& operator=(Context&&);

		void Post(std::function<void()> func);
		void Post(std::unique_ptr<Callable> callable);

		boost::asio::io_context& io_context() { return *io_context_; }
		const boost::asio::io_context& io_context() const { return *io_context_; }

	private:
		static thread_local Context* current_context;

		friend class Scheduler;
		
		std::unique_ptr<boost::asio::io_context> io_context_;

		void Run();
		void RunFor(int32_t milliseconds);
		void Stop();
	};
} // namespace System
