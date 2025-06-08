#pragma once

#include "Core/ThirdParty/Sql.h"

namespace Sql  {
	class Stmt; 
	class Conn final {
	public:
		Conn();
		~Conn();

		bool  Connect(SQLHDBC henv, LPCWSTR connectionString);
		void  Clear();
		bool  IsConnected() const;
		Stmt  CreateStmt();
	
		NO_COPY_AND_ASSIGN(Conn);

	private:
		std::wstring _last_error_message;
		SQLHDBC _connection;
		SQLHSTMT _statement;
	};
}