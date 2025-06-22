#include "stdafx.h"
#include "Json.h"
#include "Core/ThirdParty/Json.h"

namespace Json {
	JSection::JSection()
		:
		json_(nullptr)
	{
	}

	JSection::JSection(std::unique_ptr<Json> json)
		:
		json_(std::move(json))
	{
	}

	JSection::~JSection() = default;

	JSection JSection::operator[](const char* key) {
		auto iter = json_->find(key);
		if (iter == json_->end()) {
			return JSection();
		}

		return JSection(std::make_unique<Json>(*iter));
	}

	const JSection JSection::operator[](const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end()) {
			return JSection();
		}

		return JSection(std::make_unique<Json>(*iter));
	}

	template<>
	std::optional<int8_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int8_t>();
	}

	template<>
	std::optional<int16_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int16_t>();
	}

	template<>
	std::optional<int32_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int32_t>();
	}

	template<>
	std::optional<int64_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int64_t>();
	}

	template<>
	std::optional<uint8_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint8_t>();
	}

	template<>
	std::optional<uint16_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint16_t>();
	}

	template<>
	std::optional<uint32_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint32_t>();
	}

	template<>
	std::optional<uint64_t> JSection::GetValue(const char* key) const {
		auto iter = json_->find(key);
		if (iter == json_->end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint64_t>();
	}

	JDocument::JDocument(std::unique_ptr<Json> root)
		:
		root_(std::move(root))
	{
	}

	std::optional<JDocument> JDocument::TryParse(const std::string& json_str)  {
		try {
			auto json = nlohmann::json::parse(json_str);
			return std::make_optional<JDocument>(std::make_unique<Json>(std::move(json)));
		}
		catch (const std::exception&) {
			return std::nullopt;
		}
	}

	JDocument::~JDocument() = default;

	JSection JDocument::operator[](const char* key) {
		auto iter = root_->find(key);
		if (iter == root_->end()) {
			return JSection();
		}

		return JSection(std::make_unique<Json>(*iter));
	}

	const JSection JDocument::operator[](const char* key) const {
		auto iter = root_->find(key);
		if (iter == root_->end()) {
			return JSection();
		}

		return JSection(std::make_unique<Json>(*iter));
	}
}
