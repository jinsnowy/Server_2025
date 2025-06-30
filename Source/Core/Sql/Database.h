#pragma once

#include "Core/Sql/Agent.h"

namespace Sql  {
	class Conn;
	class Pool;

	class Database final {
	public:
		Database(std::string db_name);

		~Database();

		bool Initialize(const std::wstring& db_dsn, uint32_t db_pool_size);

		Pool& GetPool() {
			return *_conn_pool;
		}

		Agent GetAgent();

		void Destroy();

	private:
		std::string _db_name;
		std::unique_ptr<Pool> _conn_pool;
	};
}
