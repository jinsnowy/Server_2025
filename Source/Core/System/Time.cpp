#include "stdafx.h"
#include "Time.h"
#include "DateTime.h"

namespace System {

	const Time Time::kMinValue = Time(std::chrono::system_clock::time_point::min());
	const Time Time::kMaxValue = Time(std::chrono::system_clock::time_point::max());

	Time Time::UtcNow() {
		return Time(std::chrono::system_clock::now());
	}

	Time Time::Now() {
		auto now = std::chrono::system_clock::now();
		auto local = std::chrono::zoned_time{ std::chrono::current_zone(), now };
		return Time(local.get_sys_time());
	}

	Time::Time(const DateTime& rhs)
		:
		time_point_(std::chrono::system_clock::from_time_t(rhs.GetPosixSeconds())) {
		time_point_ += std::chrono::milliseconds(rhs.millisecond());
	}

	int64_t Time::GetPosixTimeMilliSeconds() const {
		return std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch()).count();
	}

	int64_t Time::GetPosixTimeSeconds() const {
		return std::chrono::duration_cast<std::chrono::seconds>(time_point_.time_since_epoch()).count();
	}

	time_t Time::ToTimeT() const {
		return std::chrono::system_clock::to_time_t(time_point_);
	}

	Time Time::FromTimeT(const time_t& rhs) {
		return Time(std::chrono::system_clock::from_time_t(rhs));
	}

	Time Time::operator+(const Duration& rhs) const {
		return time_point_ + std::chrono::milliseconds(rhs.Milliseconds());
	}

	Time Time::operator-(const Duration& rhs) const {
		return time_point_ - std::chrono::milliseconds(rhs.Milliseconds());
	}

	Time& Time::operator+=(const Duration& rhs) {
		time_point_ += std::chrono::milliseconds(rhs.Milliseconds());
		return *this;
	}

	Time& Time::operator-=(const Duration& rhs) {
		time_point_ -= std::chrono::milliseconds(rhs.Milliseconds());
		return *this;
	}

	Duration Time::operator-(const Time& rhs) const {
		return Duration(std::chrono::duration_cast<std::chrono::milliseconds>(time_point_ - rhs.time_point_).count());
	}

	Duration operator-(const Time& lhs, const Time& rhs) {
		return Duration(std::chrono::duration_cast<std::chrono::milliseconds>(lhs.time_point_ - rhs.time_point_).count());
	}

	Time operator+(const Time& lhs, const Duration& rhs) {
		return lhs.operator+(rhs);
	}

	Time operator-(const Time& lhs, const Duration& rhs) {
		return lhs.operator-(rhs);
	}

	std::string Time::ToString() const {
		return DateTime(*this).ToString();
	}

}
