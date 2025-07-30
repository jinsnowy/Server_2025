#include "stdafx.h"
#include "Player.h"
#include "Combat.h"

namespace Server::Model {

	Player::Player()
		:
		combat_(std::make_unique<Combat>()) {
	}

	Player::~Player() = default;

	bool Player::LoadFromDb() {
		return true;
	}


}
