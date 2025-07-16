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

	Quat ToQuat(const Euler& euler) {
		return glm::quat(glm::radians(euler));
	}

	Euler ToEuler(const Quat& quat) {
		return glm::degrees(glm::eulerAngles(quat));
	}
}
