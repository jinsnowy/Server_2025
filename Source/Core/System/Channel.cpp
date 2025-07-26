#include "stdafx.h"
#include "Channel.h"
#include "Core/System/Scheduler.h"
#include "Core/System/Context.h"

namespace System {
Channel::Channel() 
	:
	context_(Scheduler::Current().GetContext()) {
}

Channel Channel::RoundRobin() {
	return Channel(Scheduler::RoundRobin().GetContext());
}

bool Channel::IsSynchronized() const {
	return context_.get() == Context::Current();
}

void Channel::Post(std::function<void()> func) const {
	context_->Post(std::move(func));
}

void Channel::Post(std::unique_ptr<Callable> callable) const {
	context_->Post(std::move(callable));
}

void Channel::Post(Callable* callable) const {
	context_->Post(std::unique_ptr<Callable>(callable));
}

}  // namespace System
