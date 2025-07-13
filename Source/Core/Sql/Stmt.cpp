#include "stdafx.h"
#include "Stmt.h"
#include "Core/Misc/ApplyVariants.h"
#include "Core/System/String.h"

namespace Sql  {
	const Stmt Stmt::kInvalidStmt = Stmt(SQL_NULL_HANDLE);

	Stmt Stmt::Invalid() {
		return Stmt(SQL_NULL_HANDLE);
	}

	Stmt::Stmt(SQLHSTMT stmt)
		:
		_eof(false),
		_stmt(stmt),
		_return_value(0),
		_fetch_ready(false),
		_params{},
		_columns{}
	{
		_params.reserve(kMaxParamCount);
		_columns.reserve(kMaxColumnCount);
		::SQLFreeStmt(_stmt, SQL_UNBIND);
		::SQLFreeStmt(_stmt, SQL_RESET_PARAMS);
		::SQLFreeStmt(_stmt, SQL_CLOSE);
		BindReturnValue();
	}

	Stmt::~Stmt() = default;

	bool Stmt::Execute(const wchar_t* sp_name) {
		wchar_t buffer[512] = {};
		swprintf_s(buffer, L"{? = CALL %s", sp_name);

		const int32_t input_param_count = static_cast<int32_t>(_params.size()) - 1;
		if (input_param_count > 0) {
			wcscat_s(buffer, L"(");
			for (int32_t i = 0; i < input_param_count; ++i) {
				wcscat_s(buffer, L"?");
				if (i != input_param_count - 1) {
					wcscat_s(buffer, L", ");
				}
			}
			wcscat_s(buffer, L")");
		}
		wcscat_s(buffer, L"}");

		SQLRETURN ret = SQL_NO_DATA;

		ret = ::SQLPrepareW(_stmt, buffer, SQL_NTSL);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] prepare {} failed: {}", System::String::Convert(sp_name), GetErrorMessage(ret, SQL_HANDLE_STMT, _stmt));
			return false;
		}

		ret = ::SQLExecute(_stmt);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] execute {} failed: {}", System::String::Convert(sp_name), GetErrorMessage(ret, SQL_HANDLE_STMT, _stmt));
			LogParamDesc(buffer);
			return false;
		}
	
		_fetch_ready = false;

		return Success();
	}

	void Stmt::LogParamDesc(const wchar_t* sp_name) {
		SQLSMALLINT  sqlType = 0;
		SQLULEN      parameterSize = 0;
		SQLSMALLINT  decimalDegits = 0;
		SQLSMALLINT  nullable = 0;
		for (size_t idx = 0; idx < _params.size(); ++idx) {
			SQLDescribeParam(_stmt, static_cast<SQLUSMALLINT>(idx + 1), &sqlType, &parameterSize, &decimalDegits, &nullable);
			LOG_INFO(L"[Sql] ParamDesc sp:{}, param:{}, sqltype:{}, param_size:{},decimal_digits:{},nullable:{}", sp_name, idx + 2, sqlType, parameterSize, decimalDegits, nullable);
		}
	}

	std::string Stmt::GetLastErrorMessage() {
		return System::String::Convert(_error_message);
	}

	bool Stmt::FetchResult() {
		if (_fetch_ready == false) {
			SQLSMALLINT columnSize = static_cast<SQLSMALLINT>(_columns.size());
			SQLSMALLINT columnCount = 0;
			SQLRETURN ret = ::SQLNumResultCols(_stmt, &columnCount);
			if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
				LOG_ERROR("[Sql] get num result columns failed: {}", GetErrorMessage(ret, SQL_HANDLE_STMT, _stmt));
				return false;
			}

			if (columnCount != columnSize) {
				LOG_ERROR("[Sql] column count mismatch: {} != {}", columnCount, columnSize);
				DEBUG_BREAK;
				return false;
			}

			_fetch_ready = true;
		}

		SQLRETURN ret = ::SQLFetch(_stmt);
		switch (ret) {
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			UpdateOnFetchResult();
			return true;
		case SQL_NO_DATA_FOUND:
			_eof = true;
			return false;
		default:
			LOG_ERROR("[Sql] stmt fetch error : %s", GetErrorMessage(ret, SQL_HANDLE_STMT, _stmt));
			return false;
		}
	}

	bool Stmt::FetchEnd() {
		return _eof;
	}

	int64_t Stmt::GetAffectedRowCount() {
		SQLLEN rowCount = 0;
		SQLRETURN ret = ::SQLRowCount(_stmt, &rowCount);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
			return rowCount;
		}
		LOG_ERROR("[Sql] stmt get affected row count failed: {}", GetErrorMessage(ret, SQL_HANDLE_STMT, _stmt));
		return -1;
	}

	void Stmt::Reset() {
		::SQLFreeStmt(_stmt, SQL_UNBIND);
		::SQLFreeStmt(_stmt, SQL_RESET_PARAMS);
		::SQLFreeStmt(_stmt, SQL_CLOSE);
		_eof = false;
		_fetch_ready = false;
		_params.clear();
		_columns.clear();
		_return_value = 0;
		BindReturnValue();
	}

	int32_t Stmt::GetResult() const { return _return_value; }

	SQLHSTMT Stmt::GetHandle() {
		return _stmt;
	}

	std::wstring Stmt::error_message() const {
		return _error_message;
	}

	void Stmt::BindReturnValue() {
		BindOutParam(&_return_value);
	}

	void Stmt::UpdateOnFetchResult() {
		for (size_t idx = 1; idx < _params.size(); ++idx) {
			Misc::Apply(_params[idx].GetValue(),
				[](TimestampEx& t) {
					t.Update();
				},
				[](auto&) {}
			);
		}
		for (size_t idx = 0; idx < _columns.size(); ++idx) {
			Misc::Apply(_columns[idx].GetValue(), 
			[](TimestampEx& t) {
				t.Update();
			},
			[](auto&) {}
			);
		}
	}

}