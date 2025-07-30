#pragma once

#include "Protobuf/Public/Types.h"
#include "Protobuf/Public/ProtoUtils.h"

namespace Server {

	namespace Utilites {
		static inline Math::Vec3 ReadFrom(const types::Vector3& vector) {
			return Math::Vec3(vector.x(), vector.y(), vector.z());
		}

		static inline Math::Vec3 ReadFrom(const types::Rotator& rotator) {
			return Math::Vec3(rotator.roll(), rotator.pitch(), rotator.yaw());
		}

		static inline Math::Quat ReadFrom(const types::Quat& quat) {
			return Math::Quat(quat.x(), quat.y(), quat.z(), quat.w());
		}

		static inline Math::AxisAngle ReadFrom(const types::AxisAndAngle& axis_and_angle) {
			return Math::AxisAngle(axis_and_angle.angle_in_rad(), Math::Vec3(axis_and_angle.x(), axis_and_angle.y(), axis_and_angle.z()));
		}

		static inline Math::Transform ReadFrom(const types::Transform& transform) {
			return Math::Transform{
				ReadFrom(transform.location()),
				ReadFrom(transform.rotation()),
				ReadFrom(transform.scale())
			};
		}

		static inline Math::Pose ReadFrom(const types::Pose& pose) {
			return Math::Pose {
				ReadFrom(pose.location()),
				ReadFrom(pose.rotation())
			};
		}

		static inline void WriteTo(const Math::Vec3& vector, types::Vector3* out_vector) {
			out_vector->set_x(vector.x());
			out_vector->set_y(vector.y());
			out_vector->set_z(vector.z());
		}

		static inline void WriteTo(const Math::Vec3& rotator, types::Rotator* out_rotator) {
			out_rotator->set_roll(rotator.x());
			out_rotator->set_pitch(rotator.y());
			out_rotator->set_yaw(rotator.z());
		}

		static inline void WriteTo(const Math::AxisAngle& axis_and_angle, types::AxisAndAngle* out_axis_and_angle) {
			out_axis_and_angle->set_x(axis_and_angle.axis().x());
			out_axis_and_angle->set_y(axis_and_angle.axis().y());
			out_axis_and_angle->set_z(axis_and_angle.axis().z());
			out_axis_and_angle->set_angle_in_rad(axis_and_angle.angle());
		}

		static inline void WriteTo(const Math::Transform& transform, types::Transform* out_transform) {
			WriteTo(transform.translation, out_transform->mutable_location());
			WriteTo(transform.rotation, out_transform->mutable_rotation());
			WriteTo(transform.scale, out_transform->mutable_scale());
		}

		static inline void WriteTo(const Math::Quat& quat, types::Quat* out_quat) {
			out_quat->set_x(quat.x());
			out_quat->set_y(quat.y());
			out_quat->set_z(quat.z());
			out_quat->set_w(quat.w());
		}

		static inline void WriteTo(const Math::Vec3& axis, float angle_in_rad, types::AxisAndAngle* out_axis_and_angle) {
			out_axis_and_angle->set_x(axis.x());
			out_axis_and_angle->set_y(axis.y());
			out_axis_and_angle->set_z(axis.z());
			out_axis_and_angle->set_angle_in_rad(angle_in_rad);
		}

		static inline void WriteTo(const Math::Transform& transform, types::Pose* out_pose) {
			WriteTo(transform.translation, out_pose->mutable_location());
			WriteTo(transform.rotation, out_pose->mutable_rotation());
		}

		static inline void WriteTo(const System::Tick& tick, google::protobuf::Timestamp* out_timestamp) {
			*out_timestamp = Protobuf::ToTimestamp(tick);
		}

		static inline System::Tick ReadTick(const google::protobuf::Timestamp& timestamp) {
			return Protobuf::ToTick(timestamp);
		}

		static inline void WriteTo(const System::Time& timepoint, google::protobuf::Timestamp* out_timestamp) {
			*out_timestamp = Protobuf::ToTimestamp(timepoint);
		}

		static inline System::Time ReadTime(const google::protobuf::Timestamp& timestamp) {
			return Protobuf::ToTime(timestamp);
		}
	}

}
