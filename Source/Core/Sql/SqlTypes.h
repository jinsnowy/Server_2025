#pragma once

#include "Core/ThirdParty/Sql.h"

namespace Sql {
	struct Timestamp {
		int16_t    year;
		uint16_t   month;
		uint16_t   day;
		uint16_t   hour;
		uint16_t   minute;
		uint16_t   second;
		uint32_t    fraction;
	};

	struct TimestampEx {
		Timestamp timestamp = {};
		System::Time* out_time = nullptr;
		void Update();
	};

	class CharArray {
	public:
		CharArray(size_t length);
		CharArray(size_t length, const std::string_view& str);
		~CharArray();

		void Set(const std::string_view& str);

		std::string ToString() const;
		size_t GetLength() const;
		char* GetBuffer();
		const char* GetBuffer() const;

		CharArray(CharArray&& rhs) noexcept;
		CharArray& operator=(CharArray&& rhs) noexcept;

		NO_COPY_AND_ASSIGN(CharArray);

	private:
		char* _buffer;
		size_t _length;
	};

	class WCharArray {
	public:
		WCharArray(const size_t length);
		WCharArray(const size_t length, const std::string_view& str);
		WCharArray(const size_t length, const std::wstring_view& str);
		~WCharArray();

		void Set(const std::string_view& str);
		void Set(const std::wstring_view& str);

		std::wstring ToWString() const;
		std::string ToString() const;
		size_t GetLength() const;
		wchar_t* GetBuffer();
		const wchar_t* GetBuffer() const;

		WCharArray(WCharArray&& rhs) noexcept;
		WCharArray& operator=(WCharArray&& rhs) noexcept;

		NO_COPY_AND_ASSIGN(WCharArray);

	private:
		wchar_t* _buffer = nullptr;
		size_t _length = 0;
	};

	class ByteArray {
	public:
		ByteArray(size_t length);

		size_t GetLength() const;
		const uint8_t* GetBuffer() const;
		uint8_t* GetBuffer();

		NO_COPY_AND_ASSIGN(ByteArray);

		ByteArray(ByteArray&& rhs) noexcept;
		ByteArray& operator=(ByteArray&& rhs) noexcept;

	private:
		std::vector<uint8_t> _buffer;
	};
}
