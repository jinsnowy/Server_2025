#pragma once

#include "Core/Json/JsonFwd.h"

namespace Json {

	class JSection {
	public:
		JSection();
		JSection(std::unique_ptr<Json> json);
		~JSection();

		JSection operator[](const char* key);
		const JSection operator[](const char* key) const;

		template<typename T>
		std::optional<T> GetValue(const char* key) const {
			static_assert(false, "not implemented for this type");
			return {};
		}

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



	private:
		std::unique_ptr<Json> json_;
	};

	class JDocument {
	public:
		JDocument(std::unique_ptr<Json> root);
		~JDocument();

		static std::optional<JDocument> TryParse(const std::string& json_str);

		JSection operator[](const char* key);
		const JSection operator[](const char* key) const;

	private:
		std::unique_ptr<Json> root_;
	};
}


