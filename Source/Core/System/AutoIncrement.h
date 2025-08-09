#pragma once

namespace System {
	template<typename T, typename I, typename = std::enable_if_t<std::is_integral_v<I>>>
	class AutoIncrement {
	public:
		constexpr AutoIncrement() = default;
		constexpr AutoIncrement(const AutoIncrement&) = delete;
		constexpr AutoIncrement& operator=(const AutoIncrement&) = delete;
		
		static constexpr I Next() {
			return value_.fetch_add(1, std::memory_order_relaxed);
		}
		
		static constexpr I Current() {
			return value_.load(std::memory_order_relaxed);
		}

	private:
		static std::atomic<I> value_;
	};

	template<typename T, typename I, typename Enable>
	std::atomic<I> AutoIncrement<T, I, Enable>::value_= {1};
}