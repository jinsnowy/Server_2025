#include "stdafx.h"
#include "Section.h"
#include "../Session/WorldSession.h"

#include "Protobuf/Public/Types.h"

namespace Server {
	std::atomic<uint32_t> Section::Id::auto_increment_counter = 1;

	void Section::EnterSession(std::shared_ptr<WorldSession> session) {
		for (size_t i = 0; i < world_sessions_indexes_.size(); ++i) {
			bool is_occupied = world_sessions_indexes_[i].load();
			if (!is_occupied && world_sessions_indexes_[i].compare_exchange_strong(is_occupied, true)) {
				world_sessions_arr_[i] = session;
				++_session_count;

				LOG_INFO("Section::EnterSession session_id: {}, section_id: {}", session->session_id(), _section_id);
				return;
			}
		}
	}

	void Section::LeaveSession(std::shared_ptr<WorldSession> session) {
		for (size_t i = 0; i < world_sessions_indexes_.size(); ++i) {
			if (world_sessions_arr_[i] == session) {
				world_sessions_indexes_[i].store(false);
				world_sessions_arr_[i].reset();
				--_session_count;
				LOG_INFO("Section::LeaveSession session_id: {}, section_id: {}", session->session_id(), _section_id);
				return;
			}
		}
	}

	void Section::WriteTo(types::SectionInfo* out_section) const {
		out_section->set_id(_section_id);
		out_section->set_map_uid(_map_uid);
	}

	void Section::Multicast(const std::shared_ptr<const google::protobuf::Message>& message, const int64_t source_session_id) {
		DEBUG_ASSERT(IsSynchronized());
		for (const auto session : world_sessions_arr_) {
			if (session == nullptr) {
				continue;
			}
			if (session->session_id() != source_session_id) {
				session->Send(message);
			}
		}
	}

	void Section::ForEach(std::function<void(WorldSession&)> functor, const int64_t source_session_id) {
		auto shared_functor = std::make_shared<std::function<void(WorldSession&)>>(functor);
		for (const auto session : world_sessions_arr_) {
			if (session == nullptr) {
				continue;
			}
			if (session->session_id() != source_session_id) {
				Ctrl(*session).Post([shared_functor](WorldSession& session) {
					auto& functor = *shared_functor;
					functor(session);
				});
			}
		}
	}
}