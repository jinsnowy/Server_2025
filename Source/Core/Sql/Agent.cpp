#include "stdafx.h"
#include "Agent.h"
#include "Stmt.h"
#include "Conn.h"
#include "Pool.h"
#include "Database.h"

namespace Sql {
	Agent::Agent(std::shared_ptr<Conn> conn) 
		:
		_conn(conn)
	{
	}

	Agent::~Agent() {
	
	}
	
	Stmt Agent::CreateStmt() {
		if (_conn == nullptr) {
			return Stmt::Invalid();
		}
		return _conn->CreateStmt();
	}
}