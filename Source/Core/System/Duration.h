#pragma once

namespace System {
	class Duration {
	public:
		constexpr Duration() : _milliseconds(0) {}
		constexpr explicit Duration(const int64_t& milliseconds) : _milliseconds(milliseconds) {}

		static constexpr Duration FromMilliseconds(int64_t milliseconds) { return Duration(milliseconds); }
		static constexpr Duration FromSeconds(int64_t seconds) { return Duration(seconds * 1'000LL); }
		static constexpr Duration FromMinutes(int64_t minutes) { return Duration(minutes * 60'000LL); }
		static constexpr Duration FromHours(int64_t hours) { return Duration(hours * 3'600'000LL); }
		static constexpr Duration FromDays(int64_t days) { return Duration(days * 86'400'000LL); }

		int64_t Milliseconds() const { return _milliseconds; }
		int64_t Seconds() const { return _milliseconds / 1000LL; }
		int64_t Minutes() const { return _milliseconds / 60'000LL; }
		int64_t Hours() const { return _milliseconds / 3'600'000LL; }
		int64_t Days() const { return _milliseconds / 86'400'000LL; }
		int64_t Weeks() const { return _milliseconds / 604'800'000LL; }
		int64_t Years() const { return _milliseconds / 31'536'000'000LL; }

		Duration operator+(const Duration& other) const { return Duration(_milliseconds + other._milliseconds); }
		Duration operator-(const Duration& other) const { return Duration(_milliseconds - other._milliseconds); }
		Duration operator*(int64_t factor) const { return Duration(_milliseconds * factor); }
		Duration operator/(int64_t factor) const { return Duration(_milliseconds / factor); }

		Duration& operator+=(const Duration& other) { _milliseconds += other._milliseconds; return *this; }
		Duration& operator-=(const Duration& other) { _milliseconds -= other._milliseconds; return *this; }
		Duration& operator*=(int64_t factor) { _milliseconds *= factor; return *this; }
		Duration& operator/=(int64_t factor) { _milliseconds /= factor; return *this; }

		std::strong_ordering operator<=>(const Duration& other) const { return _milliseconds <=> other._milliseconds; }
		std::weak_ordering operator<=>(int64_t milliseconds) const { return _milliseconds <=> milliseconds; }
		bool operator==(const Duration& other) const { return _milliseconds == other._milliseconds; }

		operator int64_t() const { return _milliseconds; }

	private:
		int64_t _milliseconds;

		static constexpr int64_t kMaxSeconds = std::numeric_limits<int64_t>::max() / 1'000LL;
		static constexpr int64_t kMaxMinutes = std::numeric_limits<int64_t>::max() / 60'000LL;
		static constexpr int64_t kMaxHours = std::numeric_limits<int64_t>::max() / 360'000LL;
		static constexpr int64_t kMaxDays = std::numeric_limits<int64_t>::max() / 86'400'000LL;
		static constexpr int64_t kMaxWeeks = std::numeric_limits<int64_t>::max() / 604'800'000LL;
		static constexpr int64_t kMaxYears = std::numeric_limits<int64_t>::max() / 31'536'000'000LL;
	};
}