#pragma once

namespace Container {

	template<typename TKey, typename TValue>
	class LruCache {
	public:
		LruCache(size_t max_size) : max_size_(max_size) {}

		bool Put(const TKey& key, TValue&& value) {
			auto it = lookup_table_.find(key);
			if (it != lookup_table_.end()) {
				return false; // Key already exists
			}

			lru_key_values_.emplace_front(key, std::move(value));
			auto new_list_iter = lru_key_values_.begin();
			lookup_table_[key] = new_list_iter;

			Trim();
			return true;
		}

		void Update(const TKey& key, TValue&& value) {
			auto it = lookup_table_.find(key);
			if (it != lookup_table_.end()) {
				auto list_iter = it->second;
				lru_key_values_.erase(list_iter);
				lookup_table_.erase(it);
			}

			lru_key_values_.emplace_front(key, std::move(value));
			auto new_list_iter = lru_key_values_.begin();
			lookup_table_[key] = new_list_iter;

			Trim();
		}

		bool Pop(const TKey& key, TValue& out_value) {
			auto it = lookup_table_.find(key);
			if (it == lookup_table_.end()) {
				return false; // Key not found
			}
			auto list_iter = it->second;
			out_value = std::move(list_iter->second);
			lru_key_values_.erase(list_iter);
			lookup_table_.erase(it);
			return true;
		}

		bool Get (const TKey& key, TValue& out_value) {
			auto it = lookup_table_.find(key);
			if (it == lookup_table_.end()) {
				return false; // Key not found
			}
			auto list_iter = it->second;
			out_value = std::move(list_iter->second);
			// Move the accessed item to the front of the list
			lru_key_values_.splice(lru_key_values_.begin(), lru_key_values_, list_iter);
			return true;
		}

	private:
		size_t max_size_;

		using ListIter = typename std::list<std::pair<TKey, TValue>>::iterator;
		
		std::unordered_map<TKey, ListIter> lookup_table_;
		std::list<std::pair<TKey, TValue>> lru_key_values_;

		void Trim() {
			if (lru_key_values_.size() > max_size_) {
				auto last_iter = lru_key_values_.end();
				--last_iter; // Get the last element
				lookup_table_.erase(last_iter->first); // Remove from hashmap
				lru_key_values_.pop_back(); // Remove from list
			}
		}
	};
}
