#include "stdafx.h"
#include "Conn.h"
#include "Core/Sql/Pool.h"
#include "Core/System/Time.h"
#include "Core/Sql/Stmt.h"

namespace Sql  {
	Conn::Conn()
		:
		_connection(SQL_NULL_HANDLE),
		_statement(SQL_NULL_HANDLE)
	{
	}

	Conn::~Conn() {
		Clear();
	}

	bool Conn::Connect(SQLHDBC henv, LPCWSTR connectionString) {
		SQLRETURN ret = ::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] alloc connection handle failed : %s", GetErrorMessage(ret, SQL_HANDLE_DBC, _connection));
			return false;
		}

		uint64_t auto_commit_value = SQL_AUTOCOMMIT_ON;
		ret = ::SQLSetConnectAttrW(_connection, SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>(auto_commit_value), SQL_IS_UINTEGER);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] set auto commit failed : %s", GetErrorMessage(ret, SQL_HANDLE_DBC, _connection));
			return false;
		}

		ret = ::SQLSetConnectAttrW(_connection, SQL_ATTR_LOGIN_TIMEOUT, reinterpret_cast<SQLPOINTER>(10), 0);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] set login timeout failed : %s", GetErrorMessage(ret, SQL_HANDLE_DBC, _connection));
			return false;
		}

		ret = ::SQLSetConnectAttrW(_connection, SQL_ATTR_CONNECTION_TIMEOUT, reinterpret_cast<SQLPOINTER>(10), 0);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] set connection timeout failed : %s", GetErrorMessage(ret, SQL_HANDLE_DBC, _connection));
			return false;
		}

		WCHAR stringBuffer[MAX_PATH] = {};
		::wcscpy_s(stringBuffer, connectionString);

		WCHAR resultString[MAX_PATH] = {};
		SQLSMALLINT resultStringLen = 0;

		ret = ::SQLDriverConnectW(_connection, NULL, (SQLWCHAR*)stringBuffer, _countof(stringBuffer), (SQLWCHAR*)resultString, _countof(resultString), &resultStringLen, SQL_DRIVER_NOPROMPT);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] connect failed : %s", GetErrorMessage(ret, SQL_HANDLE_DBC, _connection));
			return false;
		}

		ret = ::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] cannot create statement handle : %s", GetErrorMessage(ret, SQL_HANDLE_STMT, _statement));
			return false;
		}

		return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
	}

	void Conn::Clear() {
		if (_statement != SQL_NULL_HANDLE) {
			::SQLFreeStmt(_statement, SQL_UNBIND);
			::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
			::SQLFreeStmt(_statement, SQL_CLOSE);
			::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
			_statement = SQL_NULL_HANDLE;
		}

		if (_connection != SQL_NULL_HANDLE) {
			::SQLDisconnect(_connection);
			::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
			_connection = SQL_NULL_HANDLE;
		}
	}

	bool Conn::IsConnected() const {
		SQLINTEGER conn_status = 0;
		SQLRETURN ret = ::SQLGetConnectAttr(_connection, SQL_ATTR_CONNECTION_DEAD, &conn_status, SQL_IS_INTEGER, NULL);
		switch (ret) {
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			return conn_status != SQL_CD_FALSE;
		}
		return true;
	}

	Stmt Conn::CreateStmt() {
		return Stmt(_statement);
	}
}

