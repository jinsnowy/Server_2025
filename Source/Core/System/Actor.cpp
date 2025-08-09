#include "stdafx.h"
#include "Core/System/Actor.h"
#include "Core/System/AutoIncrement.h"
#include "Core/System/Dispatcher.h"

namespace System {
	Actor::Actor()
		:
		channel_(Channel::Acquire()),
		actor_id_(AutoIncrement<Actor, uint64_t>::Next())
	{
	}

	Actor::Actor(const Channel& channel)
		:
		channel_(channel) ,
		actor_id_(AutoIncrement<Actor, uint64_t>::Next()) {
	}
}