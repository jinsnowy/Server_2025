#pragma once

namespace System {
	class SingletonServiceInterface {
	public:
		virtual ~SingletonServiceInterface() = default;
	};

	class ThreadLocalServiceInterface {
	public:
		virtual ~ThreadLocalServiceInterface() = default;
	};

	class TransientServiceInterface {
	public:
		virtual ~TransientServiceInterface() = default;
	};
}
