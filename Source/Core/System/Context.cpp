#include "stdafx.h"
#include "Context.h"

#include "Core/ThirdParty/BoostAsio.h"
#include "Core/System/TimerContext.h"
#include "Core/System/ExecutionContext.h"

namespace System {
	thread_local Context* Context::current_context = nullptr;

	Context::Context() 
		:
		io_context_(std::make_unique<boost::asio::io_context>()),
		timer_context_(std::make_unique<TimerContext>(this)),
		execution_context_(std::make_unique<ExecutionContext>()) {
	}

	Context::~Context() = default;

	Context::Context(Context&&) = default;
	Context& Context::operator=(Context&&) = default;

	void Context::Post(std::function<void()> func) const {
		boost::asio::post(*io_context_, std::move(func));
	}

	void Context::Post(std::unique_ptr<Callable> callable) const {
		boost::asio::post(*io_context_, Detail::CallablePtr(std::move(callable)));
	}

	void Context::Run() {
		io_context_->run();
	}

	size_t Context::RunFor(int32_t milliseconds) {
		size_t count = io_context_->run_for(std::chrono::milliseconds(milliseconds));
		return count;
	}

	void Context::Stop() {
		io_context_->stop();
	}

}  // namespace System
