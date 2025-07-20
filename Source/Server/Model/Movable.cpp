#include "stdafx.h"
#include "Movable.h"

namespace Server::Model {

	void Movable::UpsertToDb(Sql::Agent&) {

	}

	void Movable::ReadFrom(const types::CharacterPose& char_pose) {
		position_.x = char_pose.position().x();
		position_.y = char_pose.position().y();
		position_.z = char_pose.position().z();
		rotation_.x = char_pose.rotation().x();
		rotation_.y = char_pose.rotation().y();
		rotation_.z = char_pose.rotation().z();
		rotation_.w = char_pose.rotation().w();
	}

	void Movable::WriteTo(types::CharacterPose* out_char_pose) const {
		auto* position = out_char_pose->mutable_position();
		position->set_x(position_.x);
		position->set_y(position_.y);
		position->set_z(position_.z);
		auto* rotation = out_char_pose->mutable_rotation();
		rotation->set_x(rotation_.x);
		rotation->set_y(rotation_.y);
		rotation->set_z(rotation_.z);
		rotation->set_w(rotation_.w);
	}
}