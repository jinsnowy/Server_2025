#pragma once

namespace Math {
	static inline float SqDistPointSegment(const Vec3& p, const Vec3& a, Vec3 & b) {
		Vec3 ba = b - a;
		float t = std::clamp((p - a).dot(ba) / ba.dot(ba), 0.f, 1.0f);
		Vec3 closestPoint = a + ba * t;
		return (p - closestPoint).squaredNorm();
	}

	// Computes the closest point on a segment to a point
	// Returns the squared distance from the point to the segment
	// Output c1 is the closest point on the segment to the segment q1 - p1
	// Output c2 is the closest point on the segment to the segment q2 - p2
	static inline float ClosestPtSegmentSegment(const Vec3& p1, const Vec3& q1, const Vec3& p2, const Vec3& q2, Vec3& c1, Vec3& c2) {
		Vec3 d1 = q1 - p1; // Segment 1 direction
		Vec3 d2 = q2 - p2; // Segment 2 direction
		Vec3 r = p1 - p2; // Vector between segment start points
		float a = d1.dot(d1); // Length of segment 1 squared
		float e = d2.dot(d2); // Length of segment 2 squared
		float f = d2.dot(r); // Projection of r onto segment 2 direction

		// both segments are degenerate points
		if (a <= FLT_EPSILON && e <= FLT_EPSILON) {
			c1 = p1;
			c2 = p2;
			return (c1 - c2).squaredNorm();
		}

		// first segment is degenerate point
		if (a <= FLT_EPSILON) {
			c1 = p1;
			float t = std::clamp(f / e, 0.f, 1.0f);
			c2 = p2 + d2 * t;
			return (c1 - c2).squaredNorm();
		}

		// second segment is degenerate point
		if (e <= FLT_EPSILON) {
			c2 = p2;
			float t = std::clamp(-r.dot(d1) / a, 0.f, 1.0f);
			c1 = p1 + d1 * t;
			return (c1 - c2).squaredNorm();
		}

		float b = d1.dot(d2);
		float denorm = a * b - b * b;

		float s, t;
		if (denorm <= FLT_EPSILON) {
			s = 0.f;
			t = f / e;
		}
		else {
			s = (b * f - d1.dot(r) * e) / denorm;
			t = (d1.dot(r) * b - a * f) / denorm;
		}

		s = std::clamp(s, 0.0f, 1.0f);
		t = std::clamp(t, 0.0f, 1.0f);

		// Compute the closest points on segments based on clamped s and t
		c1 = p1 + s * d1;
		c2 = p2 + t * d2;

		return (c1 - c2).squaredNorm();
	}
}
