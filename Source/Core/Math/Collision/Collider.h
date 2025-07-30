#pragma once

namespace Math {
	class Collider {
	public:
		Collider(const Vec3& center)
			: 
			center_(center) {}

		virtual ~Collider() = default;
		
		const Vec3& center() const {
			return center_;
		}

		void set_center(const Vec3& center) {
			center_ = center;
		}

		void set_scale(const Vec3& scale) {
			scale_ = scale;
		}

		const Vec3& scale() const {
			return scale_;
		}

		void set_rotation(const Vec3& rotation) {
			rotation_ = rotation;
		}

		const Vec3& rotation() const {
			return rotation_;
		}

		void set_owner(void* owner) {
			owner_ = owner;
		}

		void * owner() const {
			return owner_;
		}

		Math::Mat3 RotationMatrix() const {
			return Math::MakeFromEuler(rotation_);
		}

	protected:
		Vec3 center_;
		Vec3 scale_ = Vec3(1.0f, 1.0f, 1.0f); // Default scale
		Vec3 rotation_ = Vec3::Zero(); // Default rotation
		void* owner_ = nullptr; // Pointer to the owner of this collider, if needed
	};
}

