#include "stdafx.h"
#include "Core/ThirdParty/Sql.h"
#include "Core/System/String.h"

namespace Sql {

	thread_local std::string Constant::kLastErrorMessage;

	std::string GetErrorMessage(SQLRETURN ret, SQLSMALLINT handle_type, SQLHANDLE handle) {
		if (ret == SQL_SUCCESS) {
			return "success";
		}

		SQLSMALLINT index = 1;
		SQLWCHAR sqlState[MAX_PATH] = {};
		SQLINTEGER nativeErr = 0;
		SQLWCHAR errMsg[MAX_PATH] = {};
		SQLSMALLINT msgLen = 0;
		SQLRETURN errorRet = 0;

		std::wstring error_message;

		while (index < 5) {
			errorRet = ::SQLGetDiagRecW(handle_type, handle, index, sqlState, &nativeErr, errMsg, _countof(errMsg), &msgLen);

			if (errorRet == SQL_NO_DATA)
				break;

			if (errorRet == SQL_SUCCESS || errorRet == SQL_SUCCESS_WITH_INFO) {
				error_message += errMsg;
			}

			++index;
		}

		Constant::kLastErrorMessage = System::String::Convert(error_message);
		return Constant::kLastErrorMessage;
	}
}