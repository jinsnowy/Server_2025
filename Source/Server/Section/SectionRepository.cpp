#include "stdafx.h"
#include "SectionRepository.h"
#include "Section.h"
#include "Core/System/SingletonActor.h"
#include "Model/UniqueId.h"

namespace Server {
	class SectionRepositoryImpl : public System::SingletonActor<SectionRepositoryImpl> {
	public:
		SectionRepositoryImpl() = default;
		~SectionRepositoryImpl() = default;

		std::shared_ptr<Section> GetSection(int32_t map_uid) {
			auto it = sections_.find(map_uid);
			if (it != sections_.end()) {
				return it->second;
			}
			return nullptr;
		}

		bool AddSection(int32_t map_uid, std::shared_ptr<Section> section) {
			return sections_.emplace(map_uid, section).second;
		}

	private:
		std::unordered_map<int32_t/*map_uid*/, std::shared_ptr<Section>> sections_;
	};

	System::Future<std::shared_ptr<Section>> SectionRepository::BeginEnter(int32_t map_uid) {

		System::Future<std::shared_ptr<Section>> future;

		auto& repo = SectionRepositoryImpl::GetInstance();
		Ctrl(repo).Post([future, map_uid](SectionRepositoryImpl& repo) mutable {
			auto section = repo.GetSection(map_uid);
			if (section) {
				future.SetResult(section);
				return;
			}

			// Create a new section
			int64_t section_id = Server::UniqueId::Issue(0); // Assuming server_id is 0 for this example
			auto new_section = std::make_shared<Section>(section_id);
			new_section->set_map_uid(map_uid);
			repo.AddSection(map_uid, new_section);

			future.SetResult(new_section);
		});

		return future;
	}
}

