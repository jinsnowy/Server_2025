#pragma once

#include "Core/System/Elapse.h"
#include "Core/System/DateTime.h"

namespace System {
	class Tick {
	public:
		static Tick Current();

		constexpr Tick()
			:
			time_point_{}
		{
		}

		Tick(const Tick&) = default;
		Tick& operator=(const Tick&) = default;

		Elapse operator-(const Tick& rhs) const;

		bool operator==(const Tick& rhs) const {
			return time_point_ == rhs.time_point_;
		}

		bool operator!=(const Tick& rhs) const {
			return !(*this == rhs);
		}

		bool operator<(const Tick& rhs) const {
			return time_point_ < rhs.time_point_;
		}

		bool operator<=(const Tick& rhs) const {
			return time_point_ <= rhs.time_point_;
		}
		
		bool operator>(const Tick& rhs) const {
			return time_point_ > rhs.time_point_;
		}

		bool operator>=(const Tick& rhs) const {
			return time_point_ >= rhs.time_point_;
		}
	
		// to epoch milliseconds
		int64_t GetEpocMilliseconds () const {
			return std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch()).count();
		}

		int64_t GetEpocMicroseconds() const {
			return std::chrono::duration_cast<std::chrono::microseconds>(time_point_.time_since_epoch()).count();
		}

		Tick AddMilliseconds(int64_t milliseconds) const {
			return Tick(time_point_ + std::chrono::milliseconds(milliseconds));
		}

		Tick AddSeconds(int64_t seconds) const {
			return Tick(time_point_ + std::chrono::seconds(seconds));
		}

		Tick AddSeconds(float seconds) const {
			return Tick(time_point_ + std::chrono::microseconds(static_cast<int64_t>(seconds * 1000000)));
		}

		static Tick FromEpocMilliseconds(int64_t milliseconds) {
			return Tick(std::chrono::steady_clock::time_point(std::chrono::milliseconds(milliseconds)));
		}

		static Tick FromEpocMicroseconds(int64_t microseconds) {
			return Tick(std::chrono::steady_clock::time_point(std::chrono::microseconds(microseconds)));
		}

		const std::chrono::steady_clock::time_point& internal_time_point() const {
			return time_point_;
		}

	private:
		std::chrono::steady_clock::time_point time_point_;

		Tick(const std::chrono::steady_clock::time_point& Tick);
	};
}

