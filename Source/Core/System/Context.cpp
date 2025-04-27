#include "stdafx.h"
#include "Context.h"

#include "Core/ThirdParty/BoostAsio.h"

namespace System {
	thread_local Context* Context::current_context = nullptr;

	Context::Context() 
		:
		io_context_(std::make_unique<boost::asio::io_context>()) {
	}

	Context::~Context() = default;

	Context::Context(Context&&) = default;
	Context& Context::operator=(Context&&) = default;

	void Context::Post(std::function<void()> func) {
		boost::asio::post(*io_context_, std::move(func));
	}

	void Context::Post(std::unique_ptr<Callable> callable) {
		boost::asio::post(*io_context_, CallableWrapper(std::move(callable)));
	}

	void Context::Run() {
		current_context = this;
		io_context_->run();
		current_context = nullptr;
	}

	size_t Context::RunFor(int32_t milliseconds) {
		current_context = this;
		size_t count = io_context_->run_for(std::chrono::milliseconds(milliseconds));
		current_context = nullptr;
		return count;
	}

	void Context::Stop() {
		io_context_->stop();
	}

}  // namespace System
