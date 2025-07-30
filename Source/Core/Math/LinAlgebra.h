#pragma once

#include "Core/ThirdParty/Eigen.h"

namespace Math {
    using Vec2 = Eigen::Vector2f;
    using Vec3 = Eigen::Vector3f;
    using Vec4 = Eigen::Vector4f;

    using Mat2 = Eigen::Matrix2f;
    using Mat3 = Eigen::Matrix3f;
    using Mat4 = Eigen::Matrix4f;

    using Quat = Eigen::Quaternionf;
    // Assuming Euler angles are represented as a Vec3 (Roll, Pitch, Yaw) in degrees.
    // Convention: euler.x = Roll, euler.y = Pitch, euler.z = Yaw.
    // This matches the common ZYX (Yaw, Pitch, Roll) application order for Euler to Quaternion conversion.
    using Euler = Eigen::Vector3f;
	using AxisAngle = Eigen::AngleAxisf;
	
    // --- Constant Zero Vectors and Identity Matrices/Quaternion ---
    static const Vec2 ZeroVec2 = Vec2::Zero();
    static const Vec3 ZeroVec3 = Vec3::Zero();
    static const Vec4 ZeroVec4 = Vec4::Zero();

    static const Mat2 IdentityMat2 = Mat2::Identity();
    static const Mat3 IdentityMat3 = Mat3::Identity();
    static const Mat4 IdentityMat4 = Mat4::Identity();

    // Eigen's Quaternion constructor is (w, x, y, z).
    // Identity is (1, 0, 0, 0)
    static const Quat IdentityQuat = Quat(1.0f, 0.0f, 0.0f, 0.0f);

    struct Pose;
    struct Transform
    {
        Vec3 translation = ZeroVec3; // Translation in 3D space
        Euler rotation = Euler::Zero(); // Rotation in degrees (Roll, Pitch, Yaw)
        Vec3 scale = Vec3::Ones(); // Scale in 3D space, default is (1, 1, 1)

        Transform() = default;
        Transform(const Vec3& translation, const Euler& rotation, const Vec3& scale)
            : translation(translation), rotation(rotation), scale(scale) {
        }

        Transform(const Pose& pose);
        Transform& operator=(const Pose& pose);
    };

    struct Pose
    {
        Vec3 location = ZeroVec3; // Position in 3D space
        Euler rotation = Euler::Zero(); // Rotation in degrees (Roll, Pitch, Yaw)
        // Scale is not included in Pose, as it is typically not needed for positioning.
        // If needed, it can be added later.
        Pose() = default;
        Pose(const Vec3& location, const Euler& rotation)
            :
            location(location), rotation(rotation) {
        }
        Pose(const Transform& transform);
        Pose& operator=(const Transform& transform);
    };

    // --- Utility Functions ---

    // Helper function to convert degrees to radians for Eigen
    inline float radians(float degrees) {
        return degrees * (M_PI_F / 180.0f);
    }

    // Helper function to convert radians to degrees for Eigen
    inline float degrees(float radians_val) {
        return radians_val * (180.0f / M_PI_F);
    }

    // Converts Euler angles (Roll, Pitch, Yaw in degrees) to a Quaternion.
    // Assumes ZYX (Yaw, Pitch, Roll) order of application.
    // euler.x = Roll (around X), euler.y = Pitch (around Y), euler.z = Yaw (around Z)
    static inline Quat ToQuat(const Euler& euler_degrees) {
        // Convert degrees to radians
        float roll_rad = radians(euler_degrees.x());
        float pitch_rad = radians(euler_degrees.y());
        float yaw_rad = radians(euler_degrees.z());

        // Eigen's AngleAxisd takes angle in radians and a vector.
        // Order of multiplication matters for Euler angles. ZYX (Yaw, Pitch, Roll)
        // Means: Apply Roll (X) first, then Pitch (Y), then Yaw (Z).
        // The multiplication order in C++ `Q_yaw * Q_pitch * Q_roll` means Q_roll is applied first.
        return Eigen::AngleAxisf(yaw_rad, Vec3::UnitZ()) * // Yaw around Z
            Eigen::AngleAxisf(pitch_rad, Vec3::UnitY()) * // Pitch around Y
            Eigen::AngleAxisf(roll_rad, Vec3::UnitX());    // Roll around X
    }

    static inline Quat ToQuat(const AxisAngle& axis_angle) {
        // Eigen's Quaternionf constructor takes angle in radians and a vector
        return Quat(axis_angle);
	}

    static inline Mat3 MakeFromEuler(const Euler& euler_degrees) {
        // Convert Euler angles to Quaternion first
       Quat quat = ToQuat(euler_degrees);
       return quat.toRotationMatrix();
	}

    // Converts a Quaternion to Euler angles (Roll, Pitch, Yaw in degrees).
    // Returns in ZYX order (Yaw, Pitch, Roll) as a Vector3f.
    // euler.x = Roll, euler.y = Pitch, euler.z = Yaw.
    static inline Euler ToEuler(const Quat& quat) {
        Eigen::Vector3f euler_rad = quat.toRotationMatrix().eulerAngles(0, 1, 2); // (Roll, Pitch, Yaw)

        // Convert radians to degrees and match the assumed (Roll, Pitch, Yaw) order for Euler type
        return Euler(degrees(euler_rad.x()), // Roll (from X-angle)
            degrees(euler_rad.y()), // Pitch (from Y-angle)
            degrees(euler_rad.z()));// Yaw (from Z-angle)
    }

    static inline Euler ToEuler(const AxisAngle& axis_angle) {
        // Convert AxisAngle to Quaternion first, then to Euler
        return ToEuler(ToQuat(axis_angle));
	}

    static inline AxisAngle ToAxisAngle(const Quat& quat) {
        // Eigen's AngleAxisf can be constructed directly from a Quaternion
        return Eigen::AngleAxisf(quat);
    }

    static inline AxisAngle ToAxisAngle(const Euler& euler_degrees) {
        // Convert Euler to Quaternion first, then to AxisAngle
        return ToAxisAngle(ToQuat(euler_degrees));
    }

    // Calculates the Euclidean distance between two 3D points.
    static inline float DistanceTo(const Vec3& a, const Vec3& b) {
        return (a - b).norm(); // Eigen's .norm() is equivalent to glm::length()
    }

    // Calculates the 2D distance between two 3D points, ignoring their Z-component.
    static inline float DistanceXY(const Vec3& a, const Vec3& b) {
        Vec2 a_xy(a.x(), a.y());
        Vec2 b_xy(b.x(), b.y());
        return (a_xy - b_xy).norm();
    }
    
    static inline float AngleBetween(const Vec3& a, const Vec3& b) {
        // Calculate the angle in radians between two vectors using the dot product
        float dot_product = a.dot(b);
        float magnitude_product = a.norm() * b.norm();
        
        // Clamp the value to avoid numerical issues with acos
        float cos_angle = std::clamp(dot_product / magnitude_product, -1.0f, 1.0f);
        
		return degrees(std::acos(cos_angle)); // Returns angle in degrees
	}

    // Derives a forward 3D vector from Euler angles (Roll, Pitch, Yaw in degrees).
    // This function assumes a coordinate system where:
    // - X is Forward
    // - Y is Up
    // - Z is Right
    // - Pitch rotates around X (up/down)
    // - Yaw rotates around Y (left/right)
    // - Roll rotates around Z (tilt) - though Roll is not used for forward vector calculation.
    //
    // The formula used here is suitable for such a Y-up system.
    // Input Euler components: euler.x = Roll, euler.y = Pitch, euler.z = Yaw (all in degrees).
    static inline Vec3 ForwardVectorFromEuler(const Euler& euler_degrees) {
        // Convert degrees to radians for trigonometric functions
        float pitch_rad = radians(std::fmodf(euler_degrees.y(), 360.f)); // Pitch is euler.y
        float yaw_rad = radians(std::fmodf(euler_degrees.z(), 360.f));   // Yaw is euler.z
        // Roll (euler.x) is not used for the forward vector.

        // Calculate components based on the implied Y-up system
        float cos_yaw = std::cos(yaw_rad);
        float sin_yaw = std::sin(yaw_rad);
        float cos_pitch = std::cos(pitch_rad);
        float sin_pitch = std::sin(pitch_rad);

        // left hand rule for Y-up system:
        return Vec3(cos_yaw * cos_pitch, sin_yaw * cos_pitch, sin_pitch).normalized();
    }
}

#define KINDA_SMALL_NUMBER 0.0001f
#define IS_NEARLY_ZERO(x) (std::abs(x) < KINDA_SMALL_NUMBER)
