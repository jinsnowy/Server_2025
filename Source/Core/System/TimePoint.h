#pragma once

namespace System {
	class TimePoint {
	public:
		class Duration {
		public:
			Duration(std::chrono::steady_clock::duration duration)
				:
				duration_(duration)
			{
			}

			int64_t milliseconds() const {
				return std::chrono::duration_cast<std::chrono::milliseconds>(duration_).count();
			}

		private:
			std::chrono::steady_clock::duration duration_;
		};

		static TimePoint Current();

		friend TimePoint::Duration operator-(const TimePoint& lhs, const TimePoint& rhs);

		TimePoint(const std::chrono::steady_clock::time_point& timepoint);
		
		TimePoint::Duration operator-(const TimePoint& rhs) const {
			return TimePoint::Duration(time_point_ - rhs.time_point_);
		}

		System::DateTime ToDateTime() const;

		const std::chrono::steady_clock::time_point& internal_time_point() const {
			return time_point_;
		}

	private:
		std::chrono::steady_clock::time_point time_point_;
	};


	TimePoint::Duration operator-(const TimePoint& lhs, const TimePoint& rhs) {
		return TimePoint::Duration(lhs.time_point_ - rhs.time_point_);
	}
}

