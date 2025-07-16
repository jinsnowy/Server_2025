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
		
		bool UpsertToDb(Sql::Agent& agent);
	};
}



