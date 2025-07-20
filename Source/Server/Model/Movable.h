#pragma once



namespace Server::Model {
	class Movable final {
	public:
		Movable() = default;
		~Movable() = default;

		const Math::Vec3& position() const {
			return position_;
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
		Math::Vec3 position_ = Math::ZeroVec3;
		Math::Quat rotation_ = Math::IdentityQuat;
	};
}


