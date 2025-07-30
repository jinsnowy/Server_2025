#pragma once

#include "Protobuf/Public/Types.h"

namespace Server::Model {
	struct ClientAction {
		int64_t action_id;
		types::ClientAction::ClientActionFieldCase action_type;
		System::Tick action_time;
		System::Tick expire_time;
	};
}
