#pragma once

namespace Server {
	class UniqueId {
	public:
		static int64_t Issue(int16_t server_id);

	private:
		union Internal {
			struct Accessor {
				uint16_t server_id :16;
				uint32_t timestamp :24;
				uint32_t auto_increment :16;
			};
			int64_t unique_id;
			Internal() : unique_id(0) {}
			Internal(int64_t unique_id_in) : unique_id(unique_id_in) {}
			Internal(int16_t server_id, uint32_t timestamp, uint32_t auto_increment)
				: unique_id((static_cast<int64_t>(server_id) << 40) | (static_cast<int64_t>(timestamp) << 16) | auto_increment) {
			}
		};

		static std::atomic<uint32_t> auto_increment_counter_;
	};
}

