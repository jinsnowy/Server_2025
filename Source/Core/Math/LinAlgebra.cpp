#include "stdafx.h"
#include "LinAlgebra.h"


namespace Math {

	Transform::Transform(const Pose& pose)
		: 
		translation(pose.location), 
		rotation(pose.rotation),
		scale(Vec3::Ones()) {
	}

	Transform& Transform::operator=(const Pose& pose) {
		translation = pose.location;
		rotation = pose.rotation;
		return *this;
	}

	Pose::Pose(const Transform& transform)
		: 
		location(transform.translation), 
		rotation(transform.rotation) {
	}

	Pose& Pose::operator=(const Transform& transform) {
		location = transform.translation;
		rotation = transform.rotation;
		return *this;
	}

} // namespace Math