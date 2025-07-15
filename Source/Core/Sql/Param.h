#pragma once

#include "Core/Sql/SqlTypes.h"

namespace System {
	class Time;
} // namespace System

namespace Sql {
	class Param {
	public:
		using Value = std::variant<
			bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double, 
			bool*, int8_t*, uint8_t*, int16_t*, uint16_t*, int32_t*, uint32_t*, int64_t*, uint64_t*, float*, double*,
			Timestamp, TimestampEx, char*, wchar_t*, CharArray, WCharArray, ByteArray>;

		Param() = default;

		template<typename T, typename = std::enable_if_t<!std::is_pointer_v<std::remove_cvref_t<T>>>>
		Param& Set(T&& value, SQLSMALLINT valueType, SQLSMALLINT paramType) {
			_value = std::forward<T>(value);
			_valueType = valueType;
			_paramType = paramType;
			_paramSize = sizeof(T);
			_valuePtr = reinterpret_cast<SQLPOINTER>(&_value._Storage());
			return *this;
		}

		template<typename T>
		Param& Set(T* value, SQLSMALLINT valueType, SQLSMALLINT paramType, SQLLEN paramSize, SQLLEN strLenOrIndex) {
			_value = value;
			_valueType = valueType;
			_paramType = paramType;
			_paramSize = paramSize;
			_valuePtr = value;
			_strLenOrIndex = value == nullptr ? SQL_NULL_DATA : strLenOrIndex;
			return *this;
		}

		template<typename T>
		Param& Set(T* valuePtr, SQLSMALLINT valueType, SQLSMALLINT paramType) {
			_value = {};
			_valueType = valueType;
			_paramType = paramType;
			_paramSize = 0;
			_valuePtr = valuePtr;
			_strLenOrIndex = 0;
			return *this;
		}

		template<typename T>
		T& Get() {
			return std::get<T>(_value);
		}

		template<typename T>
		const T& Get() const {
			return std::get<T>(_value);
		}

		Value& GetValue() {
			return _value;
		}

		const Value& GetValue() const {
			return _value;
		}
	private:
		friend class Stmt;
		Value		_value;
		SQLSMALLINT _valueType = 0;
		SQLSMALLINT _paramType = 0;
		SQLULEN		_paramSize = 0;
		SQLPOINTER	_valuePtr = nullptr;
		SQLULEN		_bufferLength = 0;
		SQLLEN		_strLenOrIndex = 0;
	};
}
