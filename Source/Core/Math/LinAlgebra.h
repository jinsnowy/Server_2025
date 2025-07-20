#pragma once

#include "Core/ThirdParty/Glm.h"

namespace Math {
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;

	using Mat2 = glm::mat2;
	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;
	
	using Quat = glm::quat;
	using Euler = glm::vec3; // Assuming Euler angles are represented as a Vec3 (pitch, yaw, roll)

	static constexpr const Vec2 ZeroVec2 = Vec2(0.0f);
	static constexpr const Vec3 ZeroVec3 = Vec3(0.0f);
	static constexpr const Vec4 ZeroVec4 = Vec4(0.0f);

	static constexpr const Mat2 IdentityMat2 = Mat2(1.0f);
	static constexpr const Mat3 IdentityMat3 = Mat3(1.0f);
	static constexpr const Mat4 IdentityMat4 = Mat4(1.0f);

	static constexpr const Quat IdentityQuat = Quat(1.0f, 0.0f, 0.0f, 0.0f);

	static inline Quat ToQuat(const Euler& euler) {
		return glm::quat(glm::radians(euler));
	}

	static inline Euler ToEuler(const Quat& quat) {
		return glm::degrees(glm::eulerAngles(quat));
	}
}
