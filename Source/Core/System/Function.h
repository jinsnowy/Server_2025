#pragma once

#include "Core/System/Detail/AnyInvocable.h"

namespace System {
	template<typename Signautre>
	using Function = Detail::AnyInvocable<Signautre>;
}