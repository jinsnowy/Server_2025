#pragma once

namespace System {
namespace Detail {
	template<typename T>
	struct SharedPtrWrapper {
		using InnerType = void;
		static constexpr bool value = false;
	};

	template<typename T>
	struct SharedPtrWrapper<std::shared_ptr<T>> {
		using InnerType = T;
		static constexpr bool value = true;
	};

	template<typename T>
	static constexpr bool IsSharedPtrWrapperV = SharedPtrWrapper<T>::value;
}
}
