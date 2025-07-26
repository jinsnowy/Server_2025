#include "stdafx.h"
#include "Movable.h"

namespace Server::Model {

	void Movable::UpsertToDb(Sql::Agent&) {

	}

	void Movable::ReadFrom(const types::CharacterPose& char_pose) {
		position_.x = char_pose.position().x();
		position_.y = char_pose.position().y();
		position_.z = char_pose.position().z();

		velocity_.x = char_pose.velocity().x();
		velocity_.y = char_pose.velocity().y();
		velocity_.z = char_pose.velocity().z();

		acceleration_.x = char_pose.acceleration().x();
		acceleration_.y = char_pose.acceleration().y();
		acceleration_.z = char_pose.acceleration().z();
		
		rotation_.x = char_pose.rotation().x();
		rotation_.y = char_pose.rotation().y();
		rotation_.z = char_pose.rotation().z();
		rotation_.w = char_pose.rotation().w();

		angular_velocity_.x = char_pose.angular_velocity().x();
		angular_velocity_.y = char_pose.angular_velocity().y();
		angular_velocity_.z = char_pose.angular_velocity().z();

		aim_rotation_.x = char_pose.aim_rotation().x();
		aim_rotation_.y = char_pose.aim_rotation().y();
		aim_rotation_.z = char_pose.aim_rotation().z();
		aim_rotation_.w = char_pose.aim_rotation().w();

		aim_angular_velocity_.x = char_pose.aim_angular_velocity().x();
		aim_angular_velocity_.y = char_pose.aim_angular_velocity().y();
		aim_angular_velocity_.z = char_pose.aim_angular_velocity().z();

		net_delay_ms = char_pose.net_delay_ms();
	}

	void Movable::WriteTo(types::CharacterPose* out_char_pose) const {
		auto* position = out_char_pose->mutable_position();
		position->set_x(position_.x);
		position->set_y(position_.y);
		position->set_z(position_.z);

		auto* velocity = out_char_pose->mutable_velocity();
		velocity->set_x(velocity_.x);
		velocity->set_y(velocity_.y);
		velocity->set_z(velocity_.z);

		auto* acceleration = out_char_pose->mutable_acceleration();
		acceleration->set_x(acceleration_.x);
		acceleration->set_y(acceleration_.y);
		acceleration->set_z(acceleration_.z);

		auto* rotation = out_char_pose->mutable_rotation();
		rotation->set_x(rotation_.x);
		rotation->set_y(rotation_.y);
		rotation->set_z(rotation_.z);
		rotation->set_w(rotation_.w);

		auto* angular_velocity = out_char_pose->mutable_angular_velocity();
		angular_velocity->set_x(angular_velocity_.x);
		angular_velocity->set_y(angular_velocity_.y);
		angular_velocity->set_z(angular_velocity_.z);

		auto* aim_rotation = out_char_pose->mutable_aim_rotation();
		aim_rotation->set_x(aim_rotation_.x);
		aim_rotation->set_y(aim_rotation_.y);
		aim_rotation->set_z(aim_rotation_.z);
		aim_rotation->set_w(aim_rotation_.w);

		auto* aim_angular_velocity = out_char_pose->mutable_aim_angular_velocity();
		aim_angular_velocity->set_x(aim_angular_velocity_.x);
		aim_angular_velocity->set_y(aim_angular_velocity_.y);
		aim_angular_velocity->set_z(aim_angular_velocity_.z);

		out_char_pose->set_net_delay_ms(net_delay_ms);
	}
}