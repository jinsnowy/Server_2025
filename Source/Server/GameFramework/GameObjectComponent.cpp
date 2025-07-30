#include "stdafx.h"
#include "GameObjectComponent.h"
#include "Core/Misc/Utils.h"

namespace Server {
	GameObjectComponent::GameObjectComponent(uint32_t type_id)
		:
		type_id_(type_id),
		instance_id_(Misc::AutoIncrement<GameObjectComponent>())
	{
	}

	GameObjectComponent::~GameObjectComponent() = default;
}
