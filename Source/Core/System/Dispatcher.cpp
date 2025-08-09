#include "stdafx.h"
#include "Dispatcher.h"
#include "Core/System/Scheduler.h"
#include "Core/System/Scheduler.h"

namespace System {
	Dispatcher::Dispatcher(Protection)  
		:
		channels_() {
		const uint16_t thread_count = std::max(1ui16, Scheduler::ThreadPoolCount());
		channels_.assign(MaxChannelsPerThread * thread_count, Channel());
	}

	Dispatcher::~Dispatcher() = default;

	Channel Dispatcher::CreateChannel() {
		return channels_[counter_++ % channels_.size()];
	}

	bool Dispatcher::TryPop(Channel& channel) {
		return ready_channels_.TryPop(channel);
	}

	void Dispatcher::Schedule(const Channel& channel) {
		ready_channels_.Push(channel);
	}
}
