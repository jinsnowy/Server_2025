#pragma once

namespace System {

	class FutureNoResultException : public std::exception {
	public:
		FutureNoResultException() noexcept
			: std::exception("Future does not have a result") {
		}

		const char* what() const noexcept override {
			return "Future does not have a result";
		}
	};
}