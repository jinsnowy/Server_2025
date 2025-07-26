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
		constexpr float AsMilliSecs() const { return static_cast<float>(value_) / 1'000'000.0f; }
		constexpr float AsSecs() const { return static_cast<float>(value_) / 1000000000.0f; }

	private:
		int64_t value_;
	};
}