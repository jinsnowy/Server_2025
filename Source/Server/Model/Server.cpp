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
		stmt.BindInParam(created_at);
		stmt.BindOutParam(&server_id);
		if (stmt.Execute(L"usp_UpsertServer") == false) {
			return false;
		}
		return true;
	}

	std::vector<Server> Server::LoadByType(Sql::Agent& agent, ServerType server_type) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(static_cast<uint8_t>(server_type));
		if (stmt.Execute(L"usp_SelectServerByServerType") == false) {
			LOG_ERROR("Failed to load servers of type: {}", System::Enums::ToString(server_type));
			return std::vector<Server>();
		}

		std::vector<Server> servers;
		Server server;
		stmt.BindColumn(&server.server_id);
		stmt.BindColumn(reinterpret_cast<uint8_t*>(&server.server_type));
		Sql::WCharArray server_address(256);
		stmt.BindColumn(&server_address);
		Sql::WCharArray level_map_name(256);
		stmt.BindColumn(&level_map_name);
		stmt.BindColumn(&server.created_at);
		stmt.BindColumn(&server.last_ping_time);

		while (stmt.FetchResult()) {
			server.server_address = server_address.ToString();
			server.level_map_name = level_map_name.ToString();
			servers.push_back(server);
		}

		return servers;
	}

	std::optional<Server> Server::LoadByServerId(Sql::Agent& agent, int32_t server_id)
	{
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(server_id);
		if (stmt.Execute(L"usp_SelectServerByServerId") == false) {
			LOG_ERROR("Failed to load servers of server_id: {}", server_id);
			return std::nullopt;
		}

		Server server;
		stmt.BindColumn(&server.server_id);
		stmt.BindColumn(reinterpret_cast<uint8_t*>(&server.server_type));
		Sql::WCharArray server_address(256);
		stmt.BindColumn(&server_address);
		Sql::WCharArray level_map_name(256);
		stmt.BindColumn(&level_map_name);
		stmt.BindColumn(&server.created_at);
		stmt.BindColumn(&server.last_ping_time);

		if (stmt.FetchResult()) {
			server.server_address = server_address.ToString();
			server.level_map_name = level_map_name.ToString();
			return server;
		}

		return std::nullopt;
	}

} // namespace Server