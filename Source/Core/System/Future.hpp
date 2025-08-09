#pragma once

#include "Core/System/Future.h"
#include "Core/System/Thenable.h"
#include "Core/System/FutureController.h"

namespace System {
	template<typename T>
	inline Detail::FutureController<T> Future<T>::GetController(const void* signature) {
		return Detail::FutureController<T>(state_, signature);
	}

	inline Detail::FutureController<void> Future<void>::GetController(const void* signature) {
		return Detail::FutureController<void>(state_, signature);
	}
}