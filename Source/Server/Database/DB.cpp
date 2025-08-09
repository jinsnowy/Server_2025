#include "stdafx.h"
#include "DB.h"
#include "Core/Sql/Database.h"

namespace Server {
	DB::DB(Protection)
	{
	}

	DB::~DB() {
	}

	void DB::Initialize(DBConfig config)
	{
		DB::GetInstance().lobby_db = std::make_unique<Sql::Database>("LobbyDB");
		DB::GetInstance().game_db = std::make_unique<Sql::Database>("GameDB");

		if (DB::GetInstance().lobby_db->Initialize(config.lobby_db_dsn, 4) == false) {
			LOG_ERROR(L"Failed to initialize LobbyDB with DSN: {}", config.lobby_db_dsn);
			throw std::runtime_error("Failed to initialize LobbyDB");
		}

	/*	if (DB::GetInstance().game_db->Initialize(config.game_db_dsn, 16) == false) {
			LOG_ERROR(L"Failed to initialize GameDB with DSN: {}", config.game_db_dsn);
			throw std::runtime_error("Failed to initialize GameDB");
		}*/
	}

	void DB::Shutdown()
	{
		lobby_db.reset();
		game_db.reset();
	}

} // namespace Server