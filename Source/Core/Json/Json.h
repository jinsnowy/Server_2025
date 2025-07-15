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

		template<typename T>
		std::optional<T> Value() const {
			try{
				return root_.get<T>();
			}
			catch (...) {
				return std::nullopt;
			}
		}

		template<typename T>
		std::optional<T> GetValue(const char* key) const {
			//static_assert(false, "not implemented for this type");
			return {};
		}

		template<typename T>
		T TryGetValue(const char* key, T default_value) const {
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
		std::optional<int8_t> GetValue(const char* key) const;
		template<>
		std::optional<int16_t> GetValue(const char* key) const;
		template<>
		std::optional<int32_t> GetValue(const char* key) const;
		template<>
		std::optional<int64_t> GetValue(const char* key) const;

		template<>
		std::optional<uint8_t> GetValue(const char* key) const;
		template<>
		std::optional<uint16_t> GetValue(const char* key) const;
		template<>
		std::optional<uint32_t> GetValue(const char* key) const;
		template<>
		std::optional<uint64_t> GetValue(const char* key) const;

		template<>
		std::optional<float> GetValue(const char* key) const;
		template<>
		std::optional<double> GetValue(const char* key) const;
		template<>
		std::optional<std::string> GetValue(const char* key) const;
		template<>
		std::optional<bool> GetValue(const char* key) const;
#pragma endregion

 #pragma region TryGetValue
		template<>
		int8_t TryGetValue(const char* key, int8_t default_value) const;
		template<>
		int16_t TryGetValue(const char* key, int16_t default_value) const;
		template<>
		int32_t TryGetValue(const char* key, int32_t default_value) const;
		template<>
		int64_t TryGetValue(const char* key, int64_t default_value) const;

		template<>
		uint8_t TryGetValue(const char* key, uint8_t default_value) const;
		template<>
		uint16_t TryGetValue(const char* key, uint16_t default_value) const;
		template<>
		uint32_t TryGetValue(const char* key, uint32_t default_value) const;
		template<>
		uint64_t TryGetValue(const char* key, uint64_t default_value) const;

		template<>
		float TryGetValue(const char* key, float default_value) const;
		template<>
		double TryGetValue(const char* key, double default_value) const;
		template<>
		std::string TryGetValue(const char* key, std::string default_value) const;
		std::string TryGetValue(const char* key, const std::string& default_value) const;
		template<>
		bool TryGetValue(const char* key, bool default_value) const;

#pragma endregion

	private:
		nlohmann::json root_;
	};

	class JDocument final {
	public:
		JDocument(const nlohmann::json& root);
		~JDocument();

		static std::optional<JDocument> TryParse(const std::string& json_str);

		JSection operator[](const char* key);
		const JSection operator[](const char* key) const;

		JSection GetSection(const char* key) const {
			return (*this)[key];
		}

		template<typename T>
		std::optional<T> GetValue(const char* key) const {
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


