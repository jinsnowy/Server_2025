#pragma once

namespace System {
	class Callable {
	public:
		Callable() = default;
		virtual ~Callable() = default;
		virtual void operator()() = 0;
	};

	struct CallableWrapper {
		std::unique_ptr<Callable> callable;
		void operator()() {
			(*callable)();
		}
	};
}