#include "stdafx.h"
#include "Pc.h"
#include "Core/Math/Collision/CapsuleCollider.h"
#include "Server/GameFramework/CollisionComponent.h"
#include "Server/Session/WorldSession.h"
#include "Server/Utilites/Protobuf.h"

namespace Server {

	const uint32_t Pc::kTypeId = System::hashcode<Pc>();

	Pc::Pc()
		:
		GameObject(kTypeId, System::type_name_v<Pc>) {
		auto collision_comp = std::make_unique<CollisionComponent>(this);
		auto collider = std::make_unique<Math::CapsuleCollider>(transform_.translation, kRadius, kHalfHeight);
		collision_component_ = collision_comp.get();
		collision_component_->set_collider(std::move(collider));
		collision_component_->set_on_collision_callback(
			std::bind(&Pc::OnCollision, this, std::placeholders::_1, std::placeholders::_2));
		AddComponent(std::move(collision_comp));
	}

	Pc::~Pc() = default;

	void Pc::Update(float delta_time) {
		GameObject::Update(delta_time);
	}

	void Pc::ReadFrom(const types::CharacterPose& char_pose) {
		std::lock_guard<std::mutex> lock(mutex_);

		transform_.translation.x() = char_pose.position().x();
		transform_.translation.y() = char_pose.position().y();
		transform_.translation.z() = char_pose.position().z();

		velocity_.x() = char_pose.velocity().x();
		velocity_.y() = char_pose.velocity().y();
		velocity_.z() = char_pose.velocity().z();

		acceleration_.x() = char_pose.acceleration().x();
		acceleration_.y() = char_pose.acceleration().y();
		acceleration_.z() = char_pose.acceleration().z();

		transform_.rotation = Utilites::ReadFrom(char_pose.rotation());
		angular_velocity_ = Utilites::ReadFrom(char_pose.angular_velocity());
		aim_rotation_ = Utilites::ReadFrom(char_pose.aim_rotation());
		aim_angular_velocity_ = Utilites::ReadFrom(char_pose.aim_angular_velocity());

		net_delay_ms_ = char_pose.net_delay_ms();
		timestamp_ = System::Tick::Current();
	}

	void Pc::WriteTo(types::CharacterPose* out_char_pose) const {
		std::lock_guard<std::mutex> lock(mutex_);

		auto* position = out_char_pose->mutable_position();
		position->set_x(transform_.translation.x());
		position->set_y(transform_.translation.y());
		position->set_z(transform_.translation.z());

		auto* velocity = out_char_pose->mutable_velocity();
		velocity->set_x(velocity_.x());
		velocity->set_y(velocity_.y());
		velocity->set_z(velocity_.z());

		auto* acceleration = out_char_pose->mutable_acceleration();
		acceleration->set_x(acceleration_.x());
		acceleration->set_y(acceleration_.y());
		acceleration->set_z(acceleration_.z());

		auto* rotation = out_char_pose->mutable_rotation();
		Utilites::WriteTo(transform_.rotation, rotation);

		auto* angular_velocity = out_char_pose->mutable_angular_velocity();
		Utilites::WriteTo(angular_velocity_, angular_velocity);

		auto* aim_rotation = out_char_pose->mutable_aim_rotation();
		Utilites::WriteTo(aim_rotation_, aim_rotation);

		auto* aim_angular_velocity = out_char_pose->mutable_aim_angular_velocity();
		Utilites::WriteTo(aim_angular_velocity_, aim_angular_velocity);

		out_char_pose->set_net_delay_ms(net_delay_ms_);
	}

	void Pc::WriteTo(types::PcInfo* out_pc_info) const {
		out_pc_info->set_object_id(object_id());
		out_pc_info->set_character_id(character_id_);
		out_pc_info->set_server_id(server_id_);
		WriteTo(out_pc_info->mutable_character_pose());
	}

	bool Pc::IsValidProjectile(const Math::Vec3& base, const types::Pose& pose) const {
		std::lock_guard<std::mutex> lock(mutex_);

		float diff_x = base.x() - pose.location().x();
		float diff_y = base.y() - pose.location().y();
		float diff_z = base.z() - pose.location().z();

		float diff_distance_radius_squared = diff_x * diff_x + diff_y * diff_y;
		float diff_distnace_height_squared = diff_z * diff_z;

		float diff_distance_radius = std::sqrtf(diff_distance_radius_squared);
		float diff_distance_height = std::sqrtf(diff_distnace_height_squared);

		return diff_distance_radius < kRadius
			&& diff_distance_height < kHalfHeight;
	}

	Math::Vec3 Pc::GetExpectedPosition(const System::Tick& tick) const {
		std::lock_guard<std::mutex> lock(mutex_);
		auto delta_time = (tick - timestamp_).AsSecs();
		if (delta_time > 0.f) {
			return position() + velocity_ * delta_time;
		}
		else {
			// If the delta time is zero or negative, return the current position
			return position();
		}
	}

	void Pc::set_client(std::shared_ptr<WorldSession> client) {
		client_ = std::move(client);
		if (client) {
			character_id_ = client->character_id();
			server_id_ = client->server_id();
			session_id_ = client->session_id();
		}
		else {
			character_id_ = 0;
			server_id_ = 0;
			session_id_ = 0;
		}
	}

	void Pc::OnCollision(CollisionComponent& other_component, CollisionState state) {
		if (other_component.owner()->type_id() == Pc::kTypeId) {
			LOG_INFO("Collision with another PC: {} state: {}", other_component.owner()->object_id(), System::Enums::ToString(state));
		}
	}
}