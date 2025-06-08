#pragma once

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

namespace Sql {
	class Constant {
	public:
		static thread_local std::string kLastErrorMessage;
		static constexpr size_t kMaxVarchar = 4000;
		static constexpr size_t kMaxVarBinary = 8000;
	};

	std::string GetErrorMessage(SQLRETURN ret, SQLSMALLINT handle_type, SQLHANDLE handle);
}