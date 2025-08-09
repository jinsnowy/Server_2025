#include "stdafx.h"
#include "ExecutionContext.h"
#include "Core/System/Dispatcher.h"
#include "Core/System/Detail/IMessage.h"
#include "Core/System/Dispatcher.h"
#include "Core/System/Channel.h"
#include "Core/System/Message.h"

namespace System {
	ExecutionContext::ExecutionContext(Context* owner)
		:
		owner_(owner),
		channel_(std::make_unique<Channel>(Dispatcher::GetInstance().CreateChannel())){
	}

	void ExecutionContext::run_for(int32_t milliseconds) {
		time_slice_ms_ = milliseconds;
		spin_wait_.Reset();
	
		Channel channel(Channel::EmptyTag{});

		while (time_slice_ms_ > 0) {
			if (Dispatcher::GetInstance().TryPop(channel) == false) {
				spin_wait_.Wait();
				continue;
			}

			auto tick_start = System::Tick::Current();
			channel.BeginContext(owner_);
			Detail::IMessage* message = nullptr;
			while (channel.TryPop(message)) {
				message->Execute();
				delete message;
			}
			channel.EndContext();
			auto tick_end = System::Tick::Current();

			time_slice_ms_ -= (tick_end - tick_start).AsMilliSecs();
		}

		time_slice_ms_ = 0;
	}

	void ExecutionContext::Post(Message* message) {
		if (message == nullptr) {
			return;
		}
		if (channel_) {
			channel_->Post(message);
		}
		else {
			delete message; // Cleanup if channel is not available
		}
	}
}

