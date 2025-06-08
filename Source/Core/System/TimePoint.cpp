#include "stdafx.h"
#include "TimePoint.h"
#include "Time.h"

namespace System {
	TimePoint::TimePoint(const std::chrono::steady_clock::time_point& time_point)
		:
		time_point_(time_point)
	{
	}

	Elapse TimePoint::operator-(const TimePoint& rhs) const {
		return Elapse(std::chrono::duration_cast<std::chrono::nanoseconds>(time_point_ - rhs.time_point_).count());
	}

	TimePoint TimePoint::Current() {
		return TimePoint(std::chrono::steady_clock::now());
	}

	Elapse operator-(const TimePoint& lhs, const TimePoint& rhs) {
		return lhs.operator-(rhs);
	}
}
