#pragma once

namespace System {
	class Duration;
	class PeriodicTimer {
	public:
		class Handle {
		public:
			Handle() = default;
			~Handle();
			Handle(const Handle&) = delete;
			Handle& operator=(const Handle&) = delete;
			
			Handle(Handle&& other) noexcept : handle_(other.handle_) {
				other.handle_ = nullptr;
			}
			
			Handle& operator=(Handle&& other) noexcept {
				if (this != &other) {
					handle_ = other.handle_;
					other.handle_ = nullptr;
				}
				return *this;
			}
			
			explicit Handle(void* handle) : handle_(handle) {}
			bool IsValid() const { return handle_ != nullptr; }
			void Cancel();
		
		private:
			void* handle_ = nullptr;

			void Release();
		};

		static Handle Schedule(const System::Duration& period, std::function<void(PeriodicTimer::Handle&)> body, bool first_launch = true);
	};
}


