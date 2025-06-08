#pragma once

#include "Core/System/Elapse.h"
#include "Core/System/DateTime.h"

namespace System {
	class TimePoint {
	public:
		static TimePoint Current();

		constexpr TimePoint()
			:
			time_point_{}
		{
		}

		TimePoint(const TimePoint&) = default;
		TimePoint& operator=(const TimePoint&) = default;

		Elapse operator-(const TimePoint& rhs) const;
	
		DateTime ToDateTime() const;

		const std::chrono::steady_clock::time_point& internal_time_point() const {
			return time_point_;
		}

	private:
		friend Elapse operator-(const TimePoint& lhs, const TimePoint& rhs);

		std::chrono::steady_clock::time_point time_point_;

		TimePoint(const std::chrono::steady_clock::time_point& timepoint);
	};

	Elapse operator-(const TimePoint& lhs, const TimePoint& rhs);
}

