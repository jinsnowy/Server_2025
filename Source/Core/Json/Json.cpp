#include "stdafx.h"
#include "Json.h"
#include "Core/ThirdParty/Json.h"

namespace Json {
	JSection::JSection()
		:
		root_()
	{
	}

	JSection::JSection(const nlohmann::json& root)
		:
		root_(root)
	{
	}

	JSection::~JSection() = default;

	JSection JSection::operator[](const char* key) {
		auto iter = root_.find(key);
		if (iter == root_.end()) {
			return JSection();
		}

		return JSection(*iter);
	}

	const JSection JSection::operator[](const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end()) {
			return JSection();
		}

		return JSection(*iter);
	}

	bool JSection::IsScalar() const {
		return root_.is_primitive();
	}

	bool JSection::IsArray() const {
		return root_.is_array();
	}

	bool JSection::IsObject() const {
		return root_.is_object();
	}

	std::unordered_map<std::string, JSection> JSection::GetSections() const {
		if (!root_.is_object()) {
			return {};
		}

		std::unordered_map<std::string, JSection> sections;
		for (const auto& item : root_.items()) {
			sections[item.key()] = JSection(item.value());
		}

		return sections;
	}

	template<>
	std::optional<int8_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int8_t>();
	}

	template<>
	std::optional<int16_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int16_t>();
	}

	template<>
	std::optional<int32_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int32_t>();
	}

	template<>
	std::optional<int64_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_integer()) {
			return std::nullopt;
		}
		return iter->get<int64_t>();
	}

	template<>
	std::optional<uint8_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint8_t>();
	}

	template<>
	std::optional<uint16_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint16_t>();
	}

	template<>
	std::optional<uint32_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint32_t>();
	}

	template<>
	std::optional<uint64_t> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_unsigned()) {
			return std::nullopt;
		}
		return iter->get<uint64_t>();
	}

	template<>
	std::optional<float> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_float()) {
			return std::nullopt;
		}
		return iter->get<float>();
	}

	template<>
	std::optional<double> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_number_float()) {
			return std::nullopt;
		}
		return iter->get<double>();
	}

	template<>
	std::optional<std::string> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_string()) {
			return std::nullopt;
		}
		return iter->get<std::string>();
	}

	template<>
	std::optional<bool> JSection::GetValue(const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end() || !iter->is_boolean()) {
			return std::nullopt;
		}
		return iter->get<bool>();
	}

    template<>
    int8_t JSection::TryGetValue(const char* key, int8_t default_value) const {
        auto value = GetValue<int8_t>(key);
        return value.value_or(default_value);
    }

    template<>
    int16_t JSection::TryGetValue(const char* key, int16_t default_value) const {
        auto value = GetValue<int16_t>(key);
        return value.value_or(default_value);
    }

    template<>
    int32_t JSection::TryGetValue(const char* key, int32_t default_value) const {
        auto value = GetValue<int32_t>(key);
        return value.value_or(default_value);
    }

    template<>
    int64_t JSection::TryGetValue(const char* key, int64_t default_value) const {
        auto value = GetValue<int64_t>(key);
        return value.value_or(default_value);
    }

    template<>
    uint8_t JSection::TryGetValue(const char* key, uint8_t default_value) const {
        auto value = GetValue<uint8_t>(key);
        return value.value_or(default_value);
    }

    template<>
    uint16_t JSection::TryGetValue(const char* key, uint16_t default_value) const {
        auto value = GetValue<uint16_t>(key);
        return value.value_or(default_value);
    }

    template<>
    uint32_t JSection::TryGetValue(const char* key, uint32_t default_value) const {
        auto value = GetValue<uint32_t>(key);
        return value.value_or(default_value);
    }

    template<>
    uint64_t JSection::TryGetValue(const char* key, uint64_t default_value) const {
        auto value = GetValue<uint64_t>(key);
        return value.value_or(default_value);
    }

    template<>
    float JSection::TryGetValue(const char* key, float default_value) const {
        auto value = GetValue<float>(key);
        return value.value_or(default_value);
    }

    template<>
    double JSection::TryGetValue(const char* key, double default_value) const {
        auto value = GetValue<double>(key);
        return value.value_or(default_value);
    }

    template<>
    std::string JSection::TryGetValue(const char* key, std::string default_value) const {
        auto value = GetValue<std::string>(key);
        return value.value_or(default_value);
    }

	std::string JSection::TryGetValue(const char* key, const std::string& default_value) const {
		auto value = GetValue<std::string>(key);
		return value.value_or(default_value);
	}

    template<>
    bool JSection::TryGetValue(const char* key, bool default_value) const {
        auto value = GetValue<bool>(key);
        return value.value_or(default_value);
    }

	JDocument::JDocument(const nlohmann::json& root)
		:
		root_(root)
	{
	}

	std::optional<JDocument> JDocument::TryParse(const std::string& json_str)  {
		try {
			auto root = nlohmann::json::parse(json_str);
			return std::make_optional<JDocument>(root);
		}
		catch (const std::exception&) {
			return std::nullopt;
		}
	}

	JDocument::~JDocument() = default;

	JSection JDocument::operator[](const char* key) {
		auto iter = root_.find(key);
		if (iter == root_.end()) {
			return JSection();
		}

		return JSection(*iter);
	}

	const JSection JDocument::operator[](const char* key) const {
		auto iter = root_.find(key);
		if (iter == root_.end()) {
			return JSection();
		}

		return JSection(*iter);
	}
}
