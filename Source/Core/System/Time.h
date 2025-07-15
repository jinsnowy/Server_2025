#pragma once

#include "Core/System/Duration.h"

namespace System {
	class DateTime;
	class Time {
	public:
		static Time UtcNow();
		static Time Now();

		static const Time kMinValue;
		static const Time kMaxValue;

		static const Time Min() noexcept { return kMinValue; }
		static const Time Max() noexcept { return kMaxValue; }

		constexpr Time() : time_point_{} {} ;
		constexpr Time(const std::chrono::system_clock::time_point& time_point) : time_point_(time_point) {}
		Time(const DateTime& rhs);

		int64_t GetPosixTimeMicroSeconds() const;
		int64_t GetPosixTimeMilliSeconds() const;
		int64_t GetPosixTimeSeconds() const;

		static Time FromPosixTimeMicroSeconds(const int64_t& rhs);
		static Time FromPosixTimeMilliSeconds(const int64_t& rhs);
		static Time FromPosixTimeSeconds(const int64_t& rhs);

		time_t ToTimeT() const;
		static Time FromTimeT(const time_t& rhs);

		std::string ToString() const;

		Time operator+(const Duration& rhs) const;
		Time operator-(const Duration& rhs) const;
		Time& operator+=(const Duration& rhs);
		Time& operator-=(const Duration& rhs);
		Duration operator-(const Time& rhs) const;

		const std::chrono::system_clock::time_point& internal() const { time_point_; }

	private:
		friend class Duration;
		friend constexpr std::strong_ordering operator<=>(const Time& lhs, const Time& rhs) { return lhs.time_point_ <=> rhs.time_point_; }
		friend constexpr bool operator==(const Time& lhs, const Time& rhs) { return lhs.time_point_ == rhs.time_point_; }

		// 100 nanoseconds since 1601-01-01 00:00:00 UTC
		std::chrono::system_clock::time_point time_point_;
	};
}