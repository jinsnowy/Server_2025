#pragma once

#include "Core/System/FutureState.h"

namespace System {
namespace Detail {
	template<typename T>
	class Thenable;

	template<typename T>
	class FutureController {
	public:
		FutureController(std::shared_ptr<FutureState<T>>& state, const void* signature)
			:
			state_(state),
			signature_(signature)
		{
		}

		template<typename F>
		decltype(auto) Then(F&& func);

		template<typename F, typename = std::enable_if_t<!std::is_void_v<T>>>
		decltype(auto) ThenPost(F&& func);

	private:
		std::shared_ptr<FutureState<T>>& state_;
		const void* signature_ = nullptr;
	};
}
} // namespace System

