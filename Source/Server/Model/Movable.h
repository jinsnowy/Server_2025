#pragma once

namespace Server::Model {

	enum class MovementMode {
		kNone = 0,
		kWalking,
		kNavWalking,
		kFalling,
		kSwimming,
		kFlying,
		kCustom,
		kMax
	};

	class Movable final {
	public:
		Movable() = default;
		~Movable() = default;

		const Math::Vec3& position() const {
			return position_;
		}

		std::string GetPositionString() const {
			return FORMAT("Position: ({}, {}, {})", position_.x, position_.y, position_.z);
		}

		void set_position(const Math::Vec3& position) {
			position_ = position;
		}

		const Math::Quat& rotation() const {
			return rotation_;
		}

		void set_rotation(const Math::Quat& rotation) {
			rotation_ = rotation;
		}

		void UpsertToDb(Sql::Agent& agent);

		void ReadFrom(const types::CharacterPose& char_pose);
		void WriteTo(types::CharacterPose* out_char_pose) const;

	private:
		float last_delta_time_ = 0.0f; // Optional: Add last delta time if needed
		Math::Vec3 position_ = Math::ZeroVec3;
		Math::Quat rotation_ = Math::IdentityQuat;
		Math::Vec3 angular_velocity_ = Math::ZeroVec3; // Optional: Add angular velocity if needed
		Math::Vec3 aim_angular_velocity_ = Math::ZeroVec3; // Optional: Add aim angular velocity if needed
		Math::Quat aim_rotation_ = Math::IdentityQuat; // Optional: Add aim rotation if needed
		Math::Vec3 velocity_ = Math::ZeroVec3; // Optional: Add velocity if needed
		Math::Vec3 acceleration_ = Math::ZeroVec3; // Optional: Add acceleration if needed
		MovementMode movement_mode_ = MovementMode::kNone; // Optional: Add movement mode if needed
		float net_delay_ms = 0.f;
	};
}


