#include "stdafx.h"
#include "Character.h"

namespace Server::Model {
	bool Character::UpsertToDb(Sql::Agent& agent) {
		Sql::WCharArray name_(128, character_name_.c_str());
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(account_id_);
		stmt.BindInParam(server_id_);
		stmt.BindInParam(name_);
		stmt.BindInParam(level_);
		stmt.BindInParam(exp_);
		stmt.BindInParam(last_played_);
		stmt.BindOutParam(&character_id_);
		if (stmt.Execute(L"usp_UpsertCharacter") == false) {
			LOG_ERROR("Failed to upsert character: name: {}, account_id: {}", character_name_, account_id_);
			return false;
		}
		return true;
	}

	std::vector<std::unique_ptr<Character>> Character::LoadByAccountIdAndServerId(Sql::Agent& agent, int64_t account_id, int32_t server_id) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(account_id);
		stmt.BindInParam(server_id);
		if (stmt.Execute(L"usp_SelectCharacterByAccountIdAndServerId") == false) {
			LOG_ERROR("Failed to load characters for account_id: {}", account_id);
			return {};
		}
		std::vector<std::unique_ptr<Character>> characters;
		Sql::WCharArray name_buffer(128);
		Character character;
		stmt.BindColumn(&character.character_id_);
		stmt.BindColumn(&character.account_id_);
		stmt.BindColumn(&character.server_id_);
		stmt.BindColumn(&name_buffer);
		stmt.BindColumn(&character.level_);
		stmt.BindColumn(&character.exp_);
		stmt.BindColumn(&character.last_played_);
		stmt.BindColumn(&character.created_at_);
		while (stmt.FetchResult()) {
			character.set_character_name(name_buffer.ToString());
			characters.push_back(std::make_unique<Character>(character));
		}
		return characters;
	}

	bool Character::IsCharacterExists(Sql::Agent& agent, const std::string& character_name) {
		Sql::WCharArray name_buffer(128, character_name.c_str());
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(name_buffer);
		return stmt.Execute(L"usp_CheckCharacterExistsByName") == false;
	}

	void Model::Character::WriteTo(types::CharacterInfo* out_character)
	{
		out_character->set_character_id(character_id_);
		out_character->set_account_id(account_id_);
		out_character->set_server_id(server_id_);
		out_character->set_character_name(character_name_);
		out_character->set_level(level_);
		out_character->set_exp(exp_);
		*out_character->mutable_last_played() = Protobuf::ToTimestamp(last_played_);
		*out_character->mutable_created_at() = Protobuf::ToTimestamp(created_at_);
	}


} // namespace Server::Model