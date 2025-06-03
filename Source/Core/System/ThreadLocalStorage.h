#pragma once

namespace System {
	template<typename T>
	class ThreadLocalStorage {
	public:
		ThreadLocalStorage() = default;
		~ThreadLocalStorage() = default;

		static T& GetInstance() {
			thread_local T instance;
			return instance;
		}

		ThreadLocalStorage(const ThreadLocalStorage&) = delete;
		ThreadLocalStorage& operator=(const ThreadLocalStorage&) = delete;

		ThreadLocalStorage(ThreadLocalStorage&&) = delete;
		ThreadLocalStorage& operator=(ThreadLocalStorage&&) = delete;
	};

	template<typename T>
	class ThreadLocalStorageShared : public std::enable_shared_from_this<T> {
	public:
		ThreadLocalStorageShared() = default;
		~ThreadLocalStorageShared() = default;

		static T& GetInstance() {
			thread_local SharedInstance instance;
			return *instance;
		}

		ThreadLocalStorageShared(const ThreadLocalStorageShared&) = delete;
		ThreadLocalStorageShared& operator=(const ThreadLocalStorageShared&) = delete;

		ThreadLocalStorageShared(ThreadLocalStorageShared&&) = delete;
		ThreadLocalStorageShared& operator=(ThreadLocalStorageShared&&) = delete;

	private:
		struct SharedInstance {
			std::shared_ptr<T> instance;
			SharedInstance() : instance(std::make_shared<T>()) {}

			T& operator*() {
				return *instance;
			}
		};
	};
}

