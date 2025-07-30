#pragma once

#include "Core/System/TypeInfo.h"
#include "Server/GameObject/GameObject.h"
#include "Server/GameFramework/CollisionComponent.h"

namespace types {
	class PcInfo;
	class CharacterPose;
} // namespace types

namespace Server {

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

	class WorldSession;
	class Pc : public GameObject {
	public:
		static const uint32_t kTypeId;

		Pc();
		~Pc();

		void Update(float delta_time) override;

		void ReadFrom(const types::CharacterPose& char_pose);
		void WriteTo(types::CharacterPose* out_char_pose) const;
		void WriteTo(types::PcInfo* out_pc_infos) const;
			
		bool IsValidProjectile(const Math::Vec3& base, const types::Pose& pose) const;
		Math::Vec3 GetExpectedPosition(const System::Tick& tick) const;

		int64_t character_id() const {
			return character_id_;
		}

		int64_t session_id() const {
			return session_id_;
		}

		int16_t server_id() const {
			return server_id_;
		}

		void set_client(std::shared_ptr<WorldSession> client);

		std::shared_ptr<WorldSession> GetClient() const {
			return client_.lock();
		}

		float net_delay_ms() const {
			return net_delay_ms_;
		}

	private:
		std::weak_ptr<WorldSession> client_;
		mutable std::mutex mutex_; // TODO : remove this if not needed

		int64_t character_id_ = 0; // Optional: Add character ID if needed
		int16_t server_id_ = 0; // Optional: Add server ID if needed
		int64_t session_id_ = 0; // Optional: Add session ID if needed

		float last_delta_time_ = 0.0f; // Optional: Add last delta time if needed
		Math::AxisAngle angular_velocity_; // Optional: Add angular velocity if needed
		Math::AxisAngle aim_angular_velocity_; // Optional: Add aim angular velocity if needed
		Math::Euler aim_rotation_ = Math::ZeroVec3; // Optional: Add aim rotation if needed
		Math::Vec3 velocity_ = Math::ZeroVec3; // Optional: Add velocity if needed
		Math::Vec3 acceleration_ = Math::ZeroVec3; // Optional: Add acceleration if needed
		MovementMode movement_mode_ = MovementMode::kNone; // Optional: Add movement mode if needed
		System::Tick timestamp_;
		float net_delay_ms_ = 0.f;

		static constexpr float kRadius = 46.0f; // Radius of the capsule collider
		static constexpr float kHalfHeight = 90.0f; // Half height of the capsule collider
		class CollisionComponent* collision_component_ = nullptr; // Optional: Add collision component if needed

		void OnCollision(CollisionComponent& other_component, CollisionState state);
	};
}
