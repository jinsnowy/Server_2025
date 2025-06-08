#pragma once

#include "Core/Sql/SqlTypes.h"

namespace System {
	class Time;
} // namespace System

namespace Sql {
	class Column {
	public:
		using Value = std::variant<bool*, char*, wchar_t*, int8_t*, uint8_t*, int16_t*, uint16_t*, int32_t*, uint32_t*, int64_t*, uint64_t*, float*, double*, TimestampEx>;
		
		Column() = default;

		template<typename T>
		Column& Set(T* value, SQLSMALLINT valueType) {
			_value = value;
			_targetType = valueType;
			_targetValue = reinterpret_cast<SQLPOINTER>(value);
			_bufferLength = sizeof(T);
			return *this;
		}

		template<typename T>
		Column& Set(T* value, SQLSMALLINT valueType, size_t bufferLength) {
			_value = value;
			_targetType = valueType;
			_targetValue = reinterpret_cast<SQLPOINTER>(value);
			_bufferLength = bufferLength;
			return *this;
		}

		Value& GetValue() {
			return _value;
		}

		const Value& GetValue() const {
			return _value;
		}

		size_t GetLength() const {
			return _bufferLength;
		}

	private:
		friend class Stmt;

		Value _value;
		SQLSMALLINT _targetType = 0;
		SQLPOINTER _targetValue = nullptr;
		SQLLEN _bufferLength = 0;
		SQLLEN _strLenOrIndex = 0;
	};
}

