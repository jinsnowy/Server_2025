#pragma once

namespace System {
	class SpinWait {
	public:
		static constexpr int32_t kMaxCount = 100;

		SpinWait() 
			: 
			count_(0) {
		}

		void Wait() {
			if (count_ < kMaxCount) {
				++count_;
			} else {
				std::this_thread::yield();
				count_ = 0;
			}
		}

		void Reset() {
			count_ = 0;
		}

	private:
		int32_t count_;
	};

	class SpinLock final {
	public:
		SpinLock() 
			: 
			flag_(false) {
		}
		
		void lock() {
			while (flag_.exchange(true)) {
				spin_wait_.Wait();
			}
		}

		bool try_lock() {
			bool expected = flag_.load(std::memory_order_acquire);
			return flag_.compare_exchange_strong(expected, true);
		}

		void unlock() {
			flag_.store(false, std::memory_order_release);
			spin_wait_.Reset();
		}

	private:
		std::atomic<bool> flag_;
		SpinWait spin_wait_;
	};
}
