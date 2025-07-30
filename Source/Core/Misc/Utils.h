#pragma once

namespace Misc {
	namespace Detail {
		template<typename T>
		struct AutoIncrement {
			static std::atomic<uint64_t> counter;
		};

		template<typename T>
		std::atomic<uint64_t> AutoIncrement<T>::counter{ 1 };
	}

	template<typename T>
	static inline uint64_t AutoIncrement() {
		return Detail::AutoIncrement<T>::counter.fetch_add(1);
	}
}
