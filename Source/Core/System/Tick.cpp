#include "stdafx.h"
#include "Tick.h"
#include "Time.h"

namespace System {
	Tick::Tick(const std::chrono::steady_clock::time_point& time_point)
		:
		time_point_(time_point)
	{
	}

	Elapse Tick::operator-(const Tick& rhs) const {
		return Elapse(std::chrono::duration_cast<std::chrono::nanoseconds>(time_point_ - rhs.time_point_).count());
	}

	Tick Tick::Current() {
		return Tick(std::chrono::steady_clock::now());
	}
}
