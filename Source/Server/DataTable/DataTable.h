#pragma once

namespace Server::DataTable
{
	template<typename Key, typename Record>
	class DataTable : public System::Singleton<DataTable<Key, Record>> {
	public:
		DataTable(System::Singleton<DataTable<Key, Record>>::Protection) {
		}

		void Add(const Key& key, std::unique_ptr<Record> record) {
			records_.emplace(key, std::move(record));
		}

		void Remove(const Key& key) {
			records_.erase(key);
		}

		const Record* Find(const Key& key) const {
			auto it = records_.find(key);
			if (it != records_.end()) {
				return it->second.get();
			}
			return nullptr;
		}

		size_t Count() const {
			return records_.size();
		}

		bool IsEmpty() const {
			return records_.empty();
		}

		void Clear() {
			records_.clear();
		}

		const std::unordered_map<Key, std::unique_ptr<Record>>& Records() const {
			return records_;
		}

	private:
		std::unordered_map<Key, std::unique_ptr<Record>> records_;
	};

}

