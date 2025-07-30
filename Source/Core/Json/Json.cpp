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

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

	int main() {
		// Create an input file stream
		std::ifstream inputFile("data.json");

		// Check if the file is open
		if (!inputFile.is_open()) {
			std::cerr << "Error: Could not open the file!" << std::endl;
			return 1;
		}

		// Parse the JSON file into a nlohmann::json object
		nlohmann::json jsonData;
		try {
			inputFile >> jsonData;
		}
		catch (const nlohmann::json::parse_error& e) {
			std::cerr << "Parse error: " << e.what() << std::endl;
			return 1;
		}

		// Close the file
		inputFile.close();

		// Access and print the JSON data
		std::cout << "Parsed JSON data: " << jsonData.dump(4) << std::endl;

		return 0;
	}

	std::optional<JDocument> JDocument::TryParse(std::ifstream& file) {
		try {
			nlohmann::json jsonData;
			file >> jsonData;
			return std::make_optional<JDocument>(jsonData);
		}
		catch (const std::exception&) {
			return std::nullopt;
		}
	}

	JDocument::~JDocument() = default;

	JDocument JDocument::Parse(const std::string& json_str)
	{
		return JDocument(nlohmann::json::parse(json_str));
	}

	JDocument JDocument::Parse(std::ifstream& file)
	{
		nlohmann::json jsonData;
		file >> jsonData;
		return JDocument(jsonData);
	}

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
