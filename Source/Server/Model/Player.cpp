#include "stdafx.h"
#include "Player.h"
#include "Movable.h"

namespace Server::Model {

	Player::Player(int64_t character_id)
		:
		character_id_(character_id),
		movable_(std::make_unique<Movable>()) {
	}

	Player::~Player() = default;

	bool Player::LoadFromDb()
	{
		return true;
	}
}
