#pragma once

#include "Core/System/MessageQueue.h"
#include "Core/System/SpinWait.h"

namespace System {
	class Context;
	class Channel;
	class ExecutionContext final {
	public:
		ExecutionContext(Context* owner);

		void run_for(int32_t milliseconds);

		Channel& channel() {
			return *channel_;
		}

		const Channel& channel() const {
			return *channel_;
		}

		void Post(Message* message);

	private:
		Context* owner_ = nullptr;
		int32_t time_slice_ms_ = 0;
		SpinWait spin_wait_;
		std::unique_ptr<Channel> channel_;
	};
}


