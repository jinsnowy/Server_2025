#pragma once

#include "Core/ThirdParty/Json.h"

namespace Json {

	class JSection final {
	public:
		JSection();
		JSection(const nlohmann::json& root);
		~JSection();

		JSection operator[](const char* key);
		const JSection operator[](const char* key) const;

		JSection GetSection(const char* key) const {
			return (*this)[key];
		}

		std::optional<JSection> TryGetSection(const char* key) const {
			auto iter = root_.find(key);
			if (iter == root_.end()) {
				return std::nullopt;
			}
			return JSection(*iter);
		}

		template<typename T>
		T Value() const {
			return root_.get<T>();
		}

		template<typename T>
		T GetValue(const char* key) const {
			//static_assert(false, "not implemented for this type");
			return {};
		}

		bool IsScalar() const;
		bool IsArray() const;
		bool IsObject() const;

		class JValue {
		public:
			JValue(const nlohmann::json& value)
				: value_(value) {
			}

			template<typename T>
			T Value() const {
				return value_.get<T>();
			}

		private:
			nlohmann::json value_;
		};
		
		class ConstIterator {
		public:
			ConstIterator(nlohmann::json::const_iterator iter, nlohmann::json::const_iterator end)
				: iter_(iter), end_(end) {}
			
			ConstIterator& operator++() {
				++iter_;
				return *this;
			}
			
			bool operator!=(const ConstIterator& other) const {
				return iter_ != other.iter_;
			}
			
			JSection operator*() const {
				return JSection(*iter_);
			}
	
		private:
			nlohmann::json::const_iterator iter_;
			nlohmann::json::const_iterator end_;
		};

		class Iterator {
		public:
			Iterator(nlohmann::json::iterator iter, nlohmann::json::iterator end)
				: iter_(iter), end_(end) {}
			
			Iterator& operator++() {
				++iter_;
				return *this;
			}
			
			bool operator!=(const Iterator& other) const {
				return iter_ != other.iter_;
			}
			
			JSection operator*() const {
				return JSection(*iter_);
			}

		private:
			nlohmann::json::iterator iter_;
			nlohmann::json::iterator end_;
		};


		ConstIterator begin() const {
			return ConstIterator(root_.cbegin(), root_.cend());
		}

		ConstIterator end() const {
			return ConstIterator(root_.cend(), root_.cend());
		}

		Iterator begin() {
			return Iterator(root_.begin(), root_.end());
		}

		Iterator end() {
			return Iterator(root_.end(), root_.end());
		}

		bool ContainsKey(const char* key) const {
			return root_.contains(key);
		}

		std::unordered_map<std::string, JSection> GetSections() const;
		
		template<typename T>
		std::optional<std::vector<T>> GetValues() const {
			try {
				std::vector<T> values;
				if (!IsArray()) {
					return std::nullopt;
				}
				if (root_.empty() == 0) {
					return {};
				}
				values.reserve(root_.size());
				for (const auto& item : root_) {
					values.push_back(item.get<T>());
				}
				return values;
			}
			catch (...) {
				return std::nullopt;
			}
		}

#pragma region GetValue

		template<>
		int8_t GetValue(const char* key) const {
			return GetSection(key).Value<int8_t>();
		}

		template<>
		int16_t GetValue(const char* key) const {
			return GetSection(key).Value<int16_t>();
		}

		template<>
		int32_t GetValue(const char* key) const {
			return GetSection(key).Value<int32_t>();
		}

		template<>
		int64_t GetValue(const char* key) const {
			return GetSection(key).Value<int64_t>();
		}

		template<>
		uint8_t GetValue(const char* key) const {
			return GetSection(key).Value<uint8_t>();
		}

		template<>
		uint16_t GetValue(const char* key) const {
			return GetSection(key).Value<uint16_t>();
		}

		template<>
		uint32_t GetValue(const char* key) const {
			return GetSection(key).Value<uint32_t>();
		}

		template<>
		uint64_t GetValue(const char* key) const {
			return GetSection(key).Value<uint64_t>();
		}

		template<>
		float GetValue(const char* key) const {
			return GetSection(key).Value<float>();
		}

		template<>
		double GetValue(const char* key) const {
			return GetSection(key).Value<double>();
		}

		template<>
		std::string GetValue(const char* key) const {
			return GetSection(key).Value<std::string>();
		}

		template<>
		bool GetValue(const char* key) const {
			return GetSection(key).Value<bool>();
		}

		template<typename T>
		T TryGetValue(const char* key, T default_value) const {
			auto iter = root_.find(key);
			if (iter != root_.end()) {
				try {
					return iter->get<T>();
				}
				catch (const nlohmann::json::exception&) {
					return default_value;
				}
			}
			return {};
		}
#pragma endregion

	private:
		nlohmann::json root_;
	};

	class JDocument final {
	public:
		JDocument(const nlohmann::json& root);
		~JDocument();

		static std::optional<JDocument> TryParse(const std::string& json_str);
		static std::optional<JDocument> TryParse(std::ifstream& file);

		static JDocument Parse(const std::string& json_str);
		static JDocument Parse(std::ifstream& file);

		JSection operator[](const char* key);
		const JSection operator[](const char* key) const;

		JSection GetSection(const char* key) const {
			return (*this)[key];
		}

		JSection::ConstIterator begin() const {
			return JSection::ConstIterator(root_.cbegin(), root_.cend());
		}

		JSection::ConstIterator end() const {
			return JSection::ConstIterator(root_.cend(), root_.cend());
		}

		JSection::Iterator begin() {
			return JSection::Iterator(root_.begin(), root_.end());
		}

		JSection::Iterator end() {
			return JSection::Iterator(root_.end(), root_.end());
		}

		template<typename T>
		T GetValue(const char* key) const {
			return JSection(root_).GetValue<T>(key);
		}

		template<typename T>
		T TryGetValue(const char* key, T default_value) const {
			return JSection(root_).TryGetValue<T>(key, default_value);
		}

	private:
		nlohmann::json root_;
	};
}


