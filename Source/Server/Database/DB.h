#pragma once


namespace Sql{
	class Database;
}

namespace Server {
	struct DBConfig {
		std::wstring lobby_db_dsn;
		std::wstring game_db_dsn;
	};

	class DB : public System::Singleton<DB> {
	public:
		DB();
		~DB();

		void Initialize(DBConfig config);
		void Shutdown();

		static Sql::Database& GetLobbyDB() {
			return *GetInstance().lobby_db;
		}

		static Sql::Database& GetGameDB() {
			return *GetInstance().game_db;
		}

		std::unique_ptr<Sql::Database> lobby_db;
		std::unique_ptr<Sql::Database> game_db;
	};

}

#define LOBBYDB DB::GetLobbyDB()
#define GAMEDB DB::GetGameDB()

