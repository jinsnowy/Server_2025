#pragma once

namespace Sql {
	class Conn;
	class Stmt;
	class Agent final {
	public:
		Agent(std::shared_ptr<Conn> conn);
		~Agent();

		Stmt CreateStmt();

		NO_COPY_AND_ASSIGN(Agent);

	private:
		std::shared_ptr<Conn> _conn;
	};
}