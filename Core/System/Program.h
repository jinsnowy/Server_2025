#pragma once

namespace System {
	class Program {
	public:
		enum class Signal {
			kNone = 0,
			kKeyboardInterrupt = 1,
			kTerminate = 2,
			kAbort = 3,
			kSegmentationFault = 4,
			kIllegalInstruction = 5
		};

		static void Wait();
		static void SignalNow(Signal signal);
	};
}

