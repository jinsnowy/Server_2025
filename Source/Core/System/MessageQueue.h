#pragma once

#include "Core/Container/MPSC.h"

namespace System {
	namespace Detail {
		class IMessage;
	} // namespace Detail

	class Channel;
	class MessageQueue {
	public:
		MessageQueue();
		~MessageQueue();

		void Enqueue(std::unique_ptr<Detail::IMessage>&& message);

		Container::MPSCQueue<Detail::IMessage*>& messages() {
			return messages_;
		}

	private:
		Container::MPSCQueue<Detail::IMessage*> messages_;
		std::unique_ptr<Channel> channel_;
	};
} // namespace System