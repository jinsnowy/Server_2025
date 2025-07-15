#include "stdafx.h"
#include "Stmt.h"
#include "Param.h"
#include "Core/System/Time.h"

namespace Sql {
	//SQLRETURN SQLBindInParameter(
	//	SQLHSTMT        StatementHandle,
	//	SQLUSMALLINT    ParameterNumber,
	//	SQLSMALLINT     InputOutputType,
	//	SQLSMALLINT     ValueType,
	//	SQLSMALLINT     ParameterType,
	//	SQLULEN         ColumnSize,
	//	SQLSMALLINT     DecimalDigits,
	//	SQLPOINTER      ParameterValuePtr,
	//	SQLLEN          BufferLength,
	//	SQLLEN* StrLen_or_IndPtr
	//);
	void Stmt::BindParamInternal(const size_t index, const Param& param, SQLSMALLINT inputOutputType, SQLSMALLINT decimalDigits) {
		SQLRETURN ret = ::SQLBindParameter(
			_stmt,
			static_cast<SQLUSMALLINT>(index),
			inputOutputType,
			param._valueType,
			param._paramType,
			param._paramSize,
			decimalDigits,
			param._valuePtr,
			param._bufferLength,
			const_cast<SQLLEN*>(&param._strLenOrIndex));
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] bind param failed: {}", GetErrorMessage(ret, SQL_HANDLE_STMT, _stmt));
		}
	}

	void Stmt::BindColumnInternal(const size_t index, Column& column) {
		SQLRETURN ret = ::SQLBindCol(_stmt, static_cast<uint16_t>(index), column._targetType, column._targetValue, column._bufferLength, &column._strLenOrIndex);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			LOG_ERROR("[Sql] bind column failed: {}", GetErrorMessage(ret, SQL_HANDLE_STMT, _stmt));
		}
	}

	void Stmt::BindInParam(const bool& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_BIT, SQL_BIT);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const uint8_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_UTINYINT, SQL_TINYINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const int8_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_STINYINT, SQL_TINYINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const uint16_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_USHORT, SQL_SMALLINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const int16_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_SSHORT, SQL_SMALLINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const uint32_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_ULONG, SQL_INTEGER);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const int32_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_SLONG, SQL_INTEGER);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const uint64_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_UBIGINT, SQL_BIGINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const int64_t& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_SBIGINT, SQL_BIGINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const float& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_FLOAT, SQL_REAL);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const double& data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_DOUBLE, SQL_DOUBLE);
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const System::Time& time) {
		const System::DateTime datetime = System::DateTime(time);
		const uint32_t nanoseconds = datetime.millisecond() * 1'000'000;
		Timestamp timestamp = 
		{
			.year = static_cast<SQLSMALLINT>(datetime.year()),
			.month = static_cast<SQLUSMALLINT>(datetime.month()),
			.day = static_cast<SQLUSMALLINT>(datetime.day()),
			.hour = static_cast<SQLUSMALLINT>(datetime.hour()),
			.minute = static_cast<SQLUSMALLINT>(datetime.minute()),
			.second = static_cast<SQLUSMALLINT>(datetime.second()),
			.fraction = static_cast<SQLUINTEGER>(nanoseconds)
		};
		auto& param = _params.emplace_back();
		param.Set(timestamp, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP);
		param._paramSize = SQL_TIMESTAMP_LEN + 1;
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT, 7);
	}

	void Stmt::BindInParam(const ByteArray& data) {
		const SQLULEN strlen = data.GetLength();
		auto& param = _params.emplace_back();
		param._valuePtr = std::get<ByteArray>(param._value).GetBuffer();
		param._valueType = SQL_C_BINARY;
		param._paramType = strlen > Constant::kMaxVarBinary ? SQL_LONGVARBINARY : SQL_VARBINARY;
		param._paramSize = strlen;
		param._strLenOrIndex = strlen;
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const CharArray& data) {
		const SQLULEN strlen = data.GetLength() + 1;
		auto& param = _params.emplace_back();
		param._valuePtr = const_cast<char*>(data.GetBuffer());
		param._valueType = SQL_C_CHAR;
		param._paramType = strlen > Constant::kMaxVarchar ? SQL_LONGVARCHAR : SQL_VARCHAR;
		param._paramSize = strlen;
		param._strLenOrIndex = SQL_NTSL;
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(const WCharArray& data) {
		const SQLULEN strlen = data.GetLength() + 1;
		auto& param = _params.emplace_back();
		param._valuePtr = const_cast<wchar_t*>(data.GetBuffer());
		param._valueType = SQL_C_WCHAR;
		param._paramType = strlen > Constant::kMaxVarchar ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
		param._paramSize = strlen;
		param._strLenOrIndex = SQL_NTSL;
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(ByteArray&& data) {
		const SQLULEN strlen = data.GetLength();
		auto& param = _params.emplace_back();
		param._value = std::move(data);
		param._valuePtr = std::get<ByteArray>(param._value).GetBuffer();
		param._valueType = SQL_C_BINARY;
		param._paramType = strlen > Constant::kMaxVarBinary ? SQL_LONGVARBINARY : SQL_VARBINARY;
		param._paramSize = strlen;
		param._strLenOrIndex = strlen;
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(CharArray&& data) {
		const SQLULEN strlen = data.GetLength() + 1;
		auto& param = _params.emplace_back();
		param._value = std::move(data);
		param._valuePtr = std::get<CharArray>(param._value).GetBuffer();
		param._valueType = SQL_C_CHAR;
		param._paramType = strlen > Constant::kMaxVarchar ? SQL_LONGVARCHAR : SQL_VARCHAR;
		param._paramSize = strlen;
		param._strLenOrIndex = SQL_NTSL;
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindInParam(WCharArray&& data) {
		const SQLULEN strlen = data.GetLength() + 1;
		auto& param = _params.emplace_back();
		param._value = std::move(data);
		param._valuePtr = std::get<WCharArray>(param._value).GetBuffer();
		param._valueType = SQL_C_WCHAR;
		param._paramType = strlen > Constant::kMaxVarchar ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
		param._paramSize = strlen;
		param._strLenOrIndex = SQL_NTSL;
		BindParamInternal(_params.size(), param, SQL_PARAM_INPUT);
	}

	void Stmt::BindOutParam(bool* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_BIT, SQL_BIT);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(uint8_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_UTINYINT, SQL_TINYINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(int8_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_STINYINT, SQL_TINYINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(uint16_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_USHORT, SQL_SMALLINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(int16_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_SSHORT, SQL_SMALLINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(uint32_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_ULONG, SQL_INTEGER);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(int32_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_SLONG, SQL_INTEGER);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(uint64_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_UBIGINT, SQL_BIGINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(int64_t* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_SBIGINT, SQL_BIGINT);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(float* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_FLOAT, SQL_REAL);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(double* data) {
		auto& param = _params.emplace_back();
		param.Set(data, SQL_C_DOUBLE, SQL_DOUBLE);
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(System::Time* data) {
		auto& param = _params.emplace_back();
		param._value.emplace<TimestampEx>();
		TimestampEx& value_holder = std::get<TimestampEx>(param._value);
		value_holder.out_time = data;
		param._valuePtr = &value_holder;
		param._valueType = SQL_C_TIMESTAMP;
		param._paramType = SQL_TIMESTAMP;
		param._paramSize = SQL_TIMESTAMP_LEN + 1;
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT, 7);
	}

	void Stmt::BindOutParam(ByteArray* data) {
		auto& param = _params.emplace_back();
		param._value = data->GetBuffer();
		param._valuePtr = data->GetBuffer();
		param._valueType = SQL_C_BINARY;
		param._paramType = data->GetLength() > Constant::kMaxVarBinary ? SQL_LONGVARBINARY : SQL_VARBINARY;
		param._paramSize = data->GetLength();
		param._strLenOrIndex = data->GetLength();
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(CharArray* data) {
		auto& param = _params.emplace_back();
		param._value = data->GetBuffer();
		param._valuePtr = data->GetBuffer();
		param._valueType = SQL_C_CHAR;
		param._paramType = data->GetLength() > Constant::kMaxVarchar ? SQL_LONGVARCHAR : SQL_VARCHAR;
		param._paramSize = data->GetLength();
		param._bufferLength = data->GetLength() + 1; // +1 for null terminator
		param._strLenOrIndex = data->GetLength();
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindOutParam(WCharArray* data) {
		auto& param = _params.emplace_back();
		param._value = data->GetBuffer();
		param._valuePtr = data->GetBuffer();
		param._valueType = SQL_C_WCHAR;
		param._paramType = data->GetLength() > Constant::kMaxVarchar ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
		param._paramSize = data->GetLength();
		param._bufferLength = 2 * (data->GetLength() + 1); // WCHAR is 2 bytes
		param._strLenOrIndex = data->GetLength();
		BindParamInternal(_params.size(), param, SQL_PARAM_OUTPUT);
	}

	void Stmt::BindColumn(bool* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_BIT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(uint8_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_TINYINT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(int8_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_TINYINT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(uint16_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_USHORT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(int16_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_SHORT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(uint32_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_ULONG);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(int32_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_LONG);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(uint64_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_UBIGINT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(int64_t* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_SBIGINT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(float* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_FLOAT);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(double* data) {
		auto& column = _columns.emplace_back();
		column.Set(data, SQL_C_DOUBLE);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(System::Time* data) {
		auto& column = _columns.emplace_back();
		column._value.emplace<TimestampEx>();
		column._targetType = SQL_C_TIMESTAMP;
		column._targetValue = reinterpret_cast<SQLPOINTER>(&(std::get<TimestampEx>(column._value).timestamp));
		column._bufferLength = sizeof(TIMESTAMP_STRUCT);
		std::get<TimestampEx>(column._value).out_time = data;
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(ByteArray* data) {
		auto& column = _columns.emplace_back();
		column.Set(data->GetBuffer(), SQL_C_BINARY, data->GetLength());
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(CharArray* data) {
		auto& column = _columns.emplace_back();
		column.Set(data->GetBuffer(), SQL_C_CHAR, data->GetLength() + 1);
		BindColumnInternal(_columns.size(), column);
	}

	void Stmt::BindColumn(WCharArray* data) {
		auto& column = _columns.emplace_back();
		column.Set(data->GetBuffer(), SQL_C_WCHAR, 2 * (data->GetLength() + 1));
		BindColumnInternal(_columns.size(), column);
	}
}