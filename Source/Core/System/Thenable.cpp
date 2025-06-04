#include "stdafx.h"
#include "Thenable.h"

namespace System {
	void FutureBase::OnException(const std::exception& e)
	{
		if (exception_callback_) {
			exception_callback_(e);
		}
		else {
			LOG_ERROR("Unhandled exception in FutureBase: {}", e.what());
			throw;
		}
	}
}
