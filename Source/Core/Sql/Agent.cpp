#include "stdafx.h"
#include "Agent.h"
#include "Stmt.h"
#include "Conn.h"
#include "Pool.h"
#include "Database.h"

namespace Sql {
	Agent::Agent(std::shared_ptr<Conn> conn, std::shared_ptr<Pool> pool)
		:
		_conn(conn),
		_pool(pool)
	{
	}

	Agent::~Agent() {
		if (_conn != nullptr && _pool != nullptr) {
			_pool->Enqueue(std::move(_conn));
		}
	}
	
	Stmt Agent::CreateStmt() {
		if (_conn == nullptr) {
			return Stmt::Invalid();
		}
		return _conn->CreateStmt();
	}
}