#include "stdafx.h"
#include "Core/ThirdParty/Sql.h"
#include "Pool.h"
#include "Conn.h"
#include "Agent.h"
#include "Pool.h"

namespace Sql  {
	struct Pool::Internal {
		SQLHENV environment;
	};

	Pool::Pool()
		:
		_internal(std::make_unique<Pool::Internal>())
	{
	}

	Pool::~Pool() {
		Clear();
	}

	bool Pool::Connect(const wchar_t* connectionString, uint32_t connectionCount) {
		_connStr = connectionString;

		SQLRETURN ret = SQL_SUCCESS;
		if (ret = ::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_internal->environment); ret != SQL_SUCCESS) {
			LOG_ERROR("[ODBC] pool cannot allocate environment handle: {}", GetErrorMessage(ret, SQL_HANDLE_ENV, _internal->environment));
			return false;
		}

		if (ret = ::SQLSetEnvAttr(_internal->environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0); ret != SQL_SUCCESS) {
			LOG_ERROR("[ODBC] pool cannot set environment attribute: {}", GetErrorMessage(ret, SQL_HANDLE_ENV, _internal->environment));
			return false;
		}

		for (uint32_t i = 0; i < connectionCount; ++i) {
			auto conn = std::make_shared<Conn>();
			if (conn->Connect(_internal->environment, _connStr.c_str()) == false) {
				return false;
			}
			connections_.Push(conn);
		}
		return true;
	}

	void Pool::Clear() {
		if (_internal->environment != SQL_NULL_HANDLE) {
			::SQLFreeHandle(SQL_HANDLE_ENV, _internal->environment);
			_internal->environment = SQL_NULL_HANDLE;
		}

		std::shared_ptr<Conn> conn;
		while (connections_.TryPop(conn)) {
			conn->Clear();
		}
	}

	std::shared_ptr<Conn> Pool::Dequeue() {
		std::shared_ptr<Conn> conn;
		if (connections_.TryPop(conn)) {
			return conn;
		}

		conn = std::make_shared<Conn>();
		for (int32_t i = 0; i < kMaxTryCount; ++i) {
			if (conn->Connect(_internal->environment, _connStr.c_str()) == true) {
				return conn;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(kWaitMilliseconds));
		}

		LOG_ERROR("[POOL] cannot get connection");

		return nullptr;
	}


	void Pool::Enqueue(std::shared_ptr<Conn>&& conn) {
		connections_.Push(std::move(conn));
	}
}