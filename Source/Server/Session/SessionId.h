#pragma once
namespace Server {
	enum class SessionType {
		kWorldSession,
		kLobbySession,
	};

	class SessionId {
	public:
		static int64_t Issue(SessionType session_type);

	private:
		static std::atomic<int32_t> session_id_counter_;
	};
}

