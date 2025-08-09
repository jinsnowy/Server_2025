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
		execution_context_(std::make_unique<ExecutionContext>(this)) {
	}

	Context::~Context() = default;

	Context::Context(Context&&) = default;
	Context& Context::operator=(Context&&) = default;

	std::shared_ptr<Context> Context::Acquire() {
		return current_context->shared_from_this();
	}

	void Context::Run() {
		io_context_->run();
	}

	size_t Context::RunFor(int32_t milliseconds) {
		return io_context_->run_for(std::chrono::milliseconds(milliseconds));
	}

	void Context::Stop() {
		io_context_->stop();
	}

}  // namespace System
