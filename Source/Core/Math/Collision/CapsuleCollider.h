#pragma once

#include "Core/Math/Collision/Collider.h"
#include "Core/Math/Collision/Helper.h"

namespace Math {
	class CapsuleCollider : public Collider {
	public:
		CapsuleCollider(const Vec3& center, 
			float radius,
			float half_height)
			: 
			Collider(center), 
			radius_(radius), 
			half_height_(half_height) {
		}

		bool IsPointInside(const Vec3& point) const {
			float scaled_radius = radius_ * scale_.x();
			float scaled_half_height = half_height_ * scale_.z();

			Vec3 segment_end_top = center() + Vec3(0, 0, scaled_half_height);
			Vec3 segment_end_bottom = center() - Vec3(0, 0, scaled_half_height);

			Mat3 rotation_matrix = RotationMatrix();
			segment_end_top = rotation_matrix * segment_end_top;
			segment_end_bottom = rotation_matrix * segment_end_bottom;

			// Find the closest point on the capsule's central segment to the given point
			Vec3 closest_point_on_segment;
			// Use a helper similar to SqDistPointSegment but one that returns the closest point
			// For simplicity, I'll use a direct calculation here, assuming Vec3 has a LengthSq method.

			Vec3 segment_dir = segment_end_top - segment_end_bottom;
			float lenSq = segment_dir.squaredNorm();

			float t = 0.0f;
			if (lenSq > FLT_EPSILON) { // Avoid division by zero for a zero-length segment
				t = (point - segment_end_bottom).dot(segment_dir) / lenSq;
				t = std::clamp(t, 0.0f, 1.0f);
			}
			closest_point_on_segment = segment_end_bottom + t * segment_dir;

			// Check if the distance from the point to the closest point on the segment
			// is less than or equal to the scaled radius.
			return (point - closest_point_on_segment).squaredNorm() <= (scaled_radius * scaled_radius);
		}

		bool CollidesWith(const CapsuleCollider& other) const {
			// Get scaled dimensions for this capsule
			float this_scaled_radius = radius_ * scale_.x();
			float this_scaled_half_height = half_height_ * scale_.z();

			// Get scaled dimensions for the other capsule
			float other_scaled_radius = other.radius_ * other.scale_.x();
			float other_scaled_half_height = other.half_height_ * other.scale_.z();

			Mat3 this_rotation_matrix = RotationMatrix();

			// Calculate the end points of the central line segment for this capsule
			Vec3 this_segment_p1 = center() + Vec3(0, 0, this_scaled_half_height);
			Vec3 this_segment_q1 = center() - Vec3(0, 0, this_scaled_half_height);
			this_segment_p1 = this_rotation_matrix * this_segment_p1;
			this_segment_q1 = this_rotation_matrix * this_segment_q1;

			Mat3 other_rotation_matrix = other.RotationMatrix();
			// Calculate the end points of the central line segment for the other capsule
			Vec3 other_segment_p2 = other.center() + Vec3(0, 0, other_scaled_half_height);
			Vec3 other_segment_q2 = other.center() - Vec3(0, 0, other_scaled_half_height);
			other_segment_p2 = other_rotation_matrix * other_segment_p2;
			other_segment_q2 = other_rotation_matrix * other_segment_q2;

			// Find the closest points between the two central line segments
			Vec3 closest_p_on_this_segment;
			Vec3 closest_p_on_other_segment;
			float sq_dist = ClosestPtSegmentSegment(
				this_segment_p1, this_segment_q1,
				other_segment_p2, other_segment_q2,
				closest_p_on_this_segment, closest_p_on_other_segment
			);

			// Calculate the sum of the effective radii
			float sum_of_radii = this_scaled_radius + other_scaled_radius;

			// Check if the squared distance between the closest points is less than or equal
			// to the squared sum of the radii.
			return sq_dist <= (sum_of_radii * sum_of_radii);
		}

	private:
		float radius_;
		float half_height_;
	};
}

