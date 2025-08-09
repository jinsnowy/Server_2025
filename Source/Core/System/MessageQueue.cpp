#include "stdafx.h"
#include "MessageQueue.h"
#include "Core/System/Dispatcher.h"
#include "Core/System/Detail/IMessage.h"

namespace System {
	MessageQueue::MessageQueue()	
		:
		channel_(std::make_unique<Channel>(Dispatcher::GetInstance().CreateChannel()))
	{
	}

	MessageQueue::~MessageQueue() = default;

	void MessageQueue::Enqueue(std::unique_ptr<Detail::IMessage>&& message) {
		messages_.Push(message.release());
		if (messages_.IncreaseNodeCount(1) == 0) {
			Dispatcher::GetInstance().Schedule(*channel_);
		}
	}
}