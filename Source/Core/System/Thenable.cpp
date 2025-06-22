#include "stdafx.h"
#include "Thenable.h"

namespace System::Detail {
	void FutureBase::OnException(const std::exception& e) const {
		if (exception_callback_) {
			exception_callback_(e);
		}
		else {
			LOG_ERROR("Unhandled exception in FutureBase: {}", e.what());
			DEBUG_BREAK;
		}
	}

	void FutureBase::OnExceptionNoThrow(const std::exception& e) const {
		if (exception_callback_) {
			exception_callback_(e);
		}
	}
}
