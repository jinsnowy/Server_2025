#pragma once

namespace System {
	class Elapse {
	public:
		constexpr Elapse()
			:
			value_(0LL)
		{
		}

		constexpr Elapse(int64_t nanoseconds)
			:
			value_(nanoseconds)
		{
		}

		constexpr int64_t AsNanoSecs() const { return value_; }
		constexpr int64_t AsMicroSecs() const { return value_ / 1'000LL; }
		constexpr int64_t AsMilliSecs() const { return value_ / 1'000'000LL; }
		constexpr int64_t AsSecs() const { return value_ / 1'000'000'000LL; }



	private:
		int64_t value_;
	};
}