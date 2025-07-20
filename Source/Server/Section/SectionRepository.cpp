#include "stdafx.h"
#include "SectionRepository.h"
#include "Section.h"
#include "Core/System/SingletonActor.h"
#include "Model/UniqueId.h"
#include "../Session/WorldSession.h"

namespace Server {
	class SectionRepositoryImpl : public System::SingletonActor<SectionRepositoryImpl> {
	public:
		SectionRepositoryImpl() = default;
		~SectionRepositoryImpl() = default;

		std::shared_ptr<Section> GetSection(uint64_t section_id) {
			auto it = sections_.find(section_id);
			if (it != sections_.end()) {
				return it->second;
			}
			return nullptr;
		}

		bool AddSection(std::shared_ptr<Section> section) {
			return sections_.emplace(section->section_id(), section).second;
		}

		void RemoveSection(uint64_t section_id) {
			sections_.erase(section_id);
		}

	private:
		std::unordered_map<uint64_t/*section_id*/, std::shared_ptr<Section>> sections_;
	};

	namespace SectionRepository {
		System::Future<std::shared_ptr<Section>> EnterSection(int32_t map_uid, std::shared_ptr<WorldSession> session) {
			auto& repo = SectionRepositoryImpl::GetInstance();
			return Ctrl(repo).Async([map_uid, session](SectionRepositoryImpl& repo) mutable {
				uint64_t section_id = Section::Id::Persistent(map_uid);

				auto section = repo.GetSection(section_id);
				if (!section) {
					section = std::make_shared<Section>(section_id);
					section->set_map_uid(map_uid);
					repo.AddSection(section);
				}

				section->EnterSession(session);
				Ctrl(*session).Post([section](WorldSession& session) {
					session.OnSectionEntered(section);
				});

				return section;
			});
		}

		void LeaveSection(int32_t map_uid, std::shared_ptr<WorldSession> session) {
			auto& repo = SectionRepositoryImpl::GetInstance();
			Ctrl(repo).Post([map_uid, session](SectionRepositoryImpl& repo) {

				uint64_t section_id = Section::Id::Persistent(map_uid);

				auto section = repo.GetSection(section_id);
				if (section) {
					section->LeaveSession(session);
					if (section->IsEmpty()) {
						// Optionally remove the section if no sessions are left
						repo.RemoveSection(section_id);
					}
					Ctrl(*session).Post([section](WorldSession& session) {
						session.OnSectionLeft(section);
					});
				}
			});
		}
	}
}

