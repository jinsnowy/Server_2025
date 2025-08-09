#include "stdafx.h"
#include "Channel.h"
#include "Core/System/Scheduler.h"
#include "Core/System/Dispatcher.h"
#include "Core/System/Context.h"
#include "Core/System/Message.h"

namespace System {

Channel::Channel()  
	:
	state_(std::make_shared<State>()) {
}

Channel::~Channel() = default;

Channel Channel::Acquire() {
	return Dispatcher::GetInstance().CreateChannel();
}

Channel Channel::Empty() {
	return Channel(EmptyTag{});
}

Channel::Channel(const Channel& channel)
	:
	state_(channel.state_) {
}

Channel& Channel::operator=(const Channel& channel) {
	if (this != &channel) {
		state_ = channel.state_;
	}
	return *this;
}

Channel::Channel(Channel&& channel) noexcept
	:
	state_(std::move(channel.state_)) {
}

Channel& Channel::operator=(Channel&& channel) noexcept {
	if (this != &channel) {
		state_ = std::move(channel.state_);
	}
	return *this;
}

void Channel::Post(Message* message) {
	Push(message);
}

size_t Channel::Dispatched(size_t count) const {
	return state_->message_queue_.DecreaseNodeCount(count);
}

void Channel::Push(Detail::IMessage* message) {
	state_->message_queue_.Push(message);
	if (state_->message_queue_.IncreaseNodeCount(1) == 0) {
		Dispatcher::GetInstance().Schedule(*this);
	}
}

bool Channel::TryPop(Detail::IMessage*& message) {
	return state_->message_queue_.TryPop(message);
}

size_t Channel::GetRefCount() const {
	return state_.use_count();
}

bool Channel::IsEmpty() const {
	return state_->message_queue_.IsEmpty();
}

Channel::Channel(EmptyTag) {
}

Channel::State::State() 
	:
	message_queue_{}  {
}

Channel::State::~State() {
	Detail::IMessage* message;
	while (message_queue_.TryPop(message)) {
		delete message;
	}
}

}  // namespace System
