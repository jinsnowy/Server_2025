#pragma once

namespace Sql {
	class Conn;
	class Stmt;
	class Agent final {
	public:
		Agent(std::shared_ptr<Conn> conn);
		~Agent();

		Stmt CreateStmt();

	private:
		std::shared_ptr<Conn> _conn;
	};
}