#include "stdafx.h"
#include "Server.h"

#include "Core/Sql/Database.h"
#include "../Database/DB.h"

namespace Server::Model {
	
	bool Server::UpsertToDb(Sql::Agent& agent) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(static_cast<uint8_t>(server_type));
		stmt.BindInParam(Sql::WCharArray(128, server_address.c_str()));
		stmt.BindInParam(Sql::WCharArray(128, level_map_name.c_str()));
		stmt.BindInParam(System::Time::UtcNow());
		stmt.BindOutParam(&server_id);
		if (stmt.Execute(L"usp_UpsertServer") == false) {
			return false;
		}
		return true;
	}
} // namespace Server