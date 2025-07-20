#include "stdafx.h"
#include "Core/System/Actor.h"

namespace System {

	Actor::Actor()
		:
		channel_(Channel()) {
	}

	Actor::Actor(const Channel& channel)
		:
		channel_(channel) {
	}

	bool Actor::IsSynchronized() const {
		return channel_.IsSynchronized();
	}

	Channel Actor::GetChannel() const {
		return Channel(channel_.GetContext());
	}

	std::shared_ptr<Context> Actor::GetContext() const {
		return channel_.GetContext();
	}
}