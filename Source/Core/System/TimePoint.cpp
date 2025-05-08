#include "stdafx.h"
#include "TimePoint.h"

namespace System {
	TimePoint::TimePoint(const std::chrono::steady_clock::time_point& time_point)
		:
		time_point_(time_point)
	{
	}

	System::DateTime TimePoint::ToDateTime() const
	{
		return System::DateTime();
	}

	TimePoint TimePoint::Current() {
		return TimePoint(std::chrono::steady_clock::now());
	}


}
