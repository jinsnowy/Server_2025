#pragma once

namespace Sql {
	class Agent;
} // namespace Sql


namespace Server::Model {
	enum class ServerType {
		kUnknown = 0,
		kDedicatedServer,
		kWorldServer,
		kLobbyServer,
	};

	struct Server {
		int32_t server_id = 0;
		ServerType server_type = ServerType::kUnknown;
		std::string server_address;
		std::string level_map_name;
		System::Time created_at;
		System::Time last_ping_time;

		bool UpsertToDb(Sql::Agent& agent);
	
		static std::vector<Server> LoadByType(Sql::Agent& agent, ServerType server_type);
		static std::optional<Server> LoadByServerId(Sql::Agent& agent, int32_t server_id);
	};
}



