#include "stdafx.h"
#include "UniqueId.h"

namespace Server
{
	static constexpr uint32_t kMaxAutoIncrement = 0xFFFF; // 16 bits for auto_increment
	static constexpr uint32_t kMaxTimestamp = 0xFFFFFF; // 24 bits for timestamp
	std::atomic<uint32_t> UniqueId::auto_increment_counter_{ 0 };

	int64_t UniqueId::Issue(int16_t server_id) {
		uint32_t timestamp = static_cast<uint32_t>(System::Time::UtcNow().GetPosixTimeMilliSeconds()) & kMaxTimestamp;
		uint32_t auto_increment = auto_increment_counter_.fetch_add(1) & kMaxAutoIncrement; // Ensure auto_increment fits in 16 bits
		return Internal(server_id, timestamp, auto_increment).unique_id;
	}
}