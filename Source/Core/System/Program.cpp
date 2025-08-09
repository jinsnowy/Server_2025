#include "stdafx.h"
#include "Program.h"

namespace System {
	static void SignalHandler(int signum);

	class ProgramImpl : public System::Singleton<ProgramImpl> {
	public:
		ProgramImpl(Protection) {
			std::signal(SIGINT, SignalHandler);
			std::signal(SIGTERM, SignalHandler);
			std::signal(SIGABRT, SignalHandler);
			std::signal(SIGSEGV, SignalHandler);
		}

		void Wait() {
			std::unique_lock<std::mutex> lock(mutex_);
			condition_.wait(lock, [this] { return signal_received_ && signal_ != Program::Signal::kNone; });
			signal_received_ = false;
		}

		void SignalNow(Program::Signal signal) {
			std::lock_guard<std::mutex> lock(mutex_);
			signal_ = signal;
			signal_received_ = true;
			condition_.notify_all();

			LOG_INFO("Program received signal ... {}", static_cast<int>(signal_));
		}

		std::mutex mutex_;
		std::condition_variable condition_;
		bool signal_received_ = false;
		Program::Signal signal_ = Program::Signal::kNone;
	};

	static void SignalHandler(int signum) {
		switch (signum) {
			case SIGINT:
				ProgramImpl::GetInstance().SignalNow(Program::Signal::kKeyboardInterrupt);
				break;
			case SIGTERM:
				ProgramImpl::GetInstance().SignalNow(Program::Signal::kTerminate);
				break;
			case SIGABRT:
				ProgramImpl::GetInstance().SignalNow(Program::Signal::kAbort);
				break;
			case SIGSEGV:
				ProgramImpl::GetInstance().SignalNow(Program::Signal::kSegmentationFault);
				break;
			case SIGILL:
				ProgramImpl::GetInstance().SignalNow(Program::Signal::kIllegalInstruction);
				break;
			default:
				break;
		}
	}


	void Program::Wait() {
		ProgramImpl::GetInstance().Wait();
	}

	void Program::SignalNow(Signal signal) {
		ProgramImpl::GetInstance().SignalNow(signal);
	}
}