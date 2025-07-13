#pragma once

namespace Sql {
	class Conn;
	class Stmt;
	class Pool;
	class Agent final {
	public:
		Agent(std::shared_ptr<Conn> conn, std::shared_ptr<Pool> pool);
		~Agent();

		Stmt CreateStmt();

		NO_COPY_AND_ASSIGN(Agent);

	private:
		std::shared_ptr<Conn> _conn;
		std::shared_ptr<Pool> _pool;
	};
}