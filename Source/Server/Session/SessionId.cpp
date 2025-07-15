#include "stdafx.h"
#include "SessionId.h"

namespace Server {
	std::atomic<int32_t> SessionId::session_id_counter_{ 1 };

	int64_t SessionId::Issue(SessionType session_type) {
		return (static_cast<int64_t>(session_type) << 32LL) | (session_id_counter_++ );
	}
}