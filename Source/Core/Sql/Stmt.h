#pragma once

#include "Core/Sql/Param.h"
#include "Core/Sql/Column.h"

namespace Sql  {
	class Stmt final {
	public:
		static const Stmt kInvalidStmt;

		static constexpr size_t kMaxParamCount = 128;
		static constexpr size_t kMaxColumnCount = 128;

		static Stmt Invalid();

		Stmt(SQLHSTMT stmt);
		~Stmt();

		bool Execute(const wchar_t* sp_name);

		bool Success() const { return GetResult() == 0; }

		std::wstring error_message() const;

		std::string GetLastErrorMessage();

		bool FetchResult();

		bool FetchEnd();

		void Reset();

		int32_t GetResult() const;

		int64_t GetAffectedRowCount();

		std::vector<Param>& GetParams() { return _params; }

		std::vector<Column>& GetColumns() { return _columns; }

		SQLHSTMT GetHandle();

		void BindInParam(const bool& data);
		void BindInParam(const uint8_t& data);
		void BindInParam(const int8_t& data);
		void BindInParam(const uint16_t& data);
		void BindInParam(const int16_t& data);
		void BindInParam(const uint32_t& data);
		void BindInParam(const int32_t& data);
		void BindInParam(const uint64_t& data);
		void BindInParam(const int64_t& data);
		void BindInParam(const float& data);
		void BindInParam(const double& data);
		void BindInParam(const System::Time& data);
		void BindInParam(const ByteArray& data);
		void BindInParam(const CharArray& data);
		void BindInParam(const WCharArray& data);
		void BindInParam(ByteArray&& data);
		void BindInParam(CharArray&& data);
		void BindInParam(WCharArray&& data);

		void BindOutParam(bool* data);
		void BindOutParam(uint8_t* data);
		void BindOutParam(int8_t* data);
		void BindOutParam(uint16_t* data);
		void BindOutParam(int16_t* data);
		void BindOutParam(uint32_t* data);
		void BindOutParam(int32_t* data);
		void BindOutParam(uint64_t* data);
		void BindOutParam(int64_t* data);
		void BindOutParam(float* data);
		void BindOutParam(double* data);
		void BindOutParam(System::Time* data);
		void BindOutParam(ByteArray* data);
		void BindOutParam(CharArray* data);
		void BindOutParam(WCharArray* data);

		void BindColumn(bool* data);
		void BindColumn(uint8_t* data);
		void BindColumn(int8_t* data);
		void BindColumn(uint16_t* data);
		void BindColumn(int16_t* data);
		void BindColumn(uint32_t* data);
		void BindColumn(int32_t* data);
		void BindColumn(uint64_t* data);
		void BindColumn(int64_t* data);
		void BindColumn(float* data);
		void BindColumn(double* data);
		void BindColumn(System::Time* data);
		void BindColumn(ByteArray* data);
		void BindColumn(CharArray* data);
		void BindColumn(WCharArray* data);

		NO_COPY_AND_ASSIGN(Stmt);

	protected:
		bool		   _eof = false;
		bool		   _fetch_ready = false;
		int32_t		   _return_value;
		SQLHSTMT	   _stmt;
		std::wstring   _error_message;
		std::vector<Param>  _params;
		std::vector<Column> _columns;

	private:
		void LogParamDesc(const wchar_t* sp_name);
		void BindReturnValue();
		void UpdateOnFetchResult();
		void BindParamInternal(const size_t index, const Param& param, SQLSMALLINT inputOutputType, SQLSMALLINT decimalDigits = 0);
		void BindColumnInternal(const size_t index, Column& column);
	};
}