#pragma once

#include "Core/System/Channel.h"
#include "Core/Container/SList.h"

namespace System {
	class Actor;
	class Dispatcher final : public System::Singleton<Dispatcher> {
	public:
		Dispatcher(Protection);
		~Dispatcher();

		Channel CreateChannel();

		bool TryPop(Channel& channel);
		void Schedule(const Channel& channel);

	private:
		static constexpr size_t MaxChannelsPerThread = 128;
		std::atomic<size_t> counter_{ 0 };
		std::vector<Channel> channels_;
		SList<Channel> ready_channels_;
	};
} // namespace System
