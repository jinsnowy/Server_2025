#include "stdafx.h"
#include "SectionRepository.h"
#include "Section.h"
#include "Core/System/SingletonActor.h"
#include "Model/UniqueId.h"
#include "../Session/WorldSession.h"
#include "Core/System/PeriodicTimer.h"
#include "Server/GameObject/Pc.h"

namespace Server {
	class SectionRepositoryImpl : public System::SingletonActor<SectionRepositoryImpl> {
	public:
		static constexpr System::Duration kSectionTickInterval = System::Duration::FromMilliseconds(10);

		SectionRepositoryImpl();
		~SectionRepositoryImpl() = default;

		std::shared_ptr<Section> GetSection(uint64_t section_id) {
			auto it = sections_.find(section_id);
			if (it != sections_.end()) {
				return it->second;
			}
			return nullptr;
		}

		void AddSection(std::shared_ptr<Section> section) {
			sections_[section->section_id()] = section;
		}

		void RemoveSection(uint64_t section_id) {
			auto iter = sections_.find(section_id);
			if (iter == sections_.end()) {
				return;
			}
			sections_.erase(iter);
		}

		void OnTick(float delta_time);

		System::Tick last_tick_;
		std::unordered_map<uint64_t/*section_id*/, std::shared_ptr<Section>> sections_;
		System::PeriodicTimer::Handle periodic_timer_handle_;
	};

	SectionRepositoryImpl::SectionRepositoryImpl()  {

		last_tick_ = System::Tick::Current();
		periodic_timer_handle_ = System::PeriodicTimer::Schedule(kSectionTickInterval, [](System::PeriodicTimer::Handle&) {
			auto& repo = SectionRepositoryImpl::GetInstance();
			const auto current_tick = System::Tick::Current();
			float delta_time = (current_tick - repo.last_tick_).AsSecs();
			repo.OnTick(delta_time);
			repo.last_tick_ = current_tick;
		});
	}

	void SectionRepositoryImpl::OnTick(float delta_time) {
		for (const auto& [_, section] : sections_) {
			if (section) {
				section->OnTick(delta_time);
			}
		}
	}

	namespace SectionRepository {
		System::Future<std::shared_ptr<Section>> EnterSection(int32_t map_uid, std::shared_ptr<WorldSession> session) {
			System::Future<std::shared_ptr<Section>> future;
			auto& repo = SectionRepositoryImpl::GetInstance();
			Ctrl(repo).Async([map_uid, session, future](SectionRepositoryImpl& repo) mutable -> std::shared_ptr<Section> {
				uint64_t section_id = Section::Id::Persistent(map_uid);

				auto section = repo.GetSection(section_id);
				if (!section) {
					section = std::make_shared<Section>(section_id);
					section->set_map_uid(map_uid);
					
					repo.AddSection(section);

					Ctrl(*section).Post([](Section& section) {
						section.OnCreated();
					});
				}

				// TODO : Check the section is already destroyed or full
				Ctrl(*session).Post([section, future](WorldSession& session) {
					session.OnSectionEntered(section);
					DEBUG_ASSERT(session.pc().character_id() != 0);
					Ctrl(*section).Post([session = System::Actor::GetShared(&session), future](Section& section) {
						section.EnterSession(session);
					});
				});

				future.SetResult(section);

				return section;
			});

			return future;
		}

		void LeaveSection(int32_t map_uid, std::shared_ptr<WorldSession> session) {
			auto& repo = SectionRepositoryImpl::GetInstance();
			Ctrl(repo).Post([map_uid, session](SectionRepositoryImpl& repo) {
				uint64_t section_id = Section::Id::Persistent(map_uid);
				auto section = repo.GetSection(section_id);
				if (section) {
					Ctrl(*section).Post([session](Section& section) {
						section.LeaveSession(session);
					});

					Ctrl(*session).Post([section](WorldSession& session) {
						session.OnSectionLeft(section);
					});

					// TODO : Check if the section is empty and remove it
					if (section->GetCount() <= 1) {
						repo.RemoveSection(section_id);

						Ctrl(*section).Post([](Section& section) {
							section.OnDestroyed();
						});
					}
				}
			});
		}

		System::Future<std::shared_ptr<Section>> FindSection(uint64_t section_id) {
			return Ctrl(SectionRepositoryImpl::GetInstance()).Async([section_id](SectionRepositoryImpl& repo) -> std::shared_ptr<Section> {
				auto section = repo.GetSection(section_id);
				if (section) {
					return section;
				}
				return nullptr;
			});
		}

		const System::Tick& GetLastTick() {
			return SectionRepositoryImpl::GetInstance().last_tick_;
		}
	}
}

