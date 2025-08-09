#pragma once

#include "Core/System/FuncTraits.h"

namespace System {
	namespace Detail {
		template<typename F>
		using AsyncResult = std::remove_cvref_t<FuncReturnT<F>>;
	}
}