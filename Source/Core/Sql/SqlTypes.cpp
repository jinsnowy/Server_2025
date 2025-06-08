#include "stdafx.h"
#include "SqlTypes.h"
#include "Core/System/Time.h"
#include "Core/System/String.h"

namespace Sql {

	static_assert(sizeof(TIMESTAMP_STRUCT) == sizeof(Timestamp));

	void TimestampEx::Update() {
		System::DateTime date_time(timestamp.year, timestamp.month, timestamp.day,
			timestamp.hour, timestamp.minute, timestamp.second,
			static_cast<uint16_t>(timestamp.fraction / 1'000'000));
		*out_time = System::Time(date_time);
	}

	CharArray::CharArray(size_t length) 
		:
		_buffer(new char[length + 1]),
		_length(length) {
		memset(_buffer, '\0', length + 1);
	}

	CharArray::CharArray(size_t length, const std::string_view& str)
		: 
		_buffer(new char[length + 1]),
		_length(length) {
		memcpy_s(_buffer, _length, str.data(), str.size());
		_buffer[std::min(str.size(), length)] = '\0';
	}

	CharArray::~CharArray() {
		if (_buffer != nullptr) {
			delete [] _buffer;
		}
	}

	void CharArray::Set(const std::string_view& str) {
		memcpy_s(_buffer, _length, str.data(), str.size());
		_length = std::min(_length, str.size());
		_buffer[_length] = '\0';
	}

	CharArray::CharArray(CharArray&& rhs) noexcept
		:
		_buffer(rhs._buffer),
		_length(rhs._length) {
		rhs._buffer = nullptr;
		rhs._length = 0;
	}

	CharArray& CharArray::operator=(CharArray&& rhs) noexcept {
		_buffer = rhs._buffer;
		_length = rhs._length;
		rhs._buffer = nullptr;
		rhs._length = 0;
		return *this;
	}

	std::string CharArray::ToString() const {
		return _buffer;
	}

	size_t CharArray::GetLength() const {
		return _length;
	}

	char* CharArray::GetBuffer() {
		return _buffer;
	}

	const char* CharArray::GetBuffer() const {
		return _buffer;
	}

	WCharArray::WCharArray(const size_t length)
		:
		_buffer(new wchar_t[length + 1]),
		_length(length) {
		wmemset(_buffer, L'\0', length + 1);
	}

	WCharArray::WCharArray(const size_t length, const std::string_view& str)
		:
		WCharArray(length, System::String::Convert(str)) {
	}

	WCharArray::WCharArray(const size_t length, const std::wstring_view& str)
		:
		_buffer(new wchar_t[length + 1]),
		_length(length) {
		wmemcpy_s(_buffer, _length, str.data(), str.size());
		_buffer[std::min(str.size(), length)] = L'\0';
	}

	WCharArray::~WCharArray() {
		if (_buffer != nullptr) {
			delete [] _buffer;
		}
	}

	void WCharArray::Set(const std::string_view& str) {
		Set(System::String::Convert(str));
	}

	void WCharArray::Set(const std::wstring_view& str) {
		wmemcpy_s(_buffer, _length, str.data(), str.size());
		_length = std::min(_length, str.size());
		_buffer[_length] = L'\0';
	}

	std::wstring WCharArray::ToWString() const {
		return _buffer;
	}

	std::string WCharArray::ToString() const {
		return System::String::Convert(_buffer);
	}

	size_t WCharArray::GetLength() const {
		return _length;
	}

	wchar_t* WCharArray::GetBuffer() {
		return _buffer;
	}

	const wchar_t* WCharArray::GetBuffer() const {
		return _buffer;
	}

	WCharArray::WCharArray(WCharArray&& rhs) noexcept
		:
		_buffer(rhs._buffer),
		_length(rhs._length) {
		rhs._buffer = nullptr;
		rhs._length = 0;
	}

	WCharArray& WCharArray::operator=(WCharArray&& rhs) noexcept {
		_buffer = rhs._buffer;
		_length = rhs._length;
		rhs._buffer = nullptr;
		rhs._length = 0;
		return *this;
	}

	ByteArray::ByteArray(size_t length)
		:
		_buffer(length, 0)
	{
	}

	size_t ByteArray::GetLength() const {
		return _buffer.size();
	}

	const uint8_t* ByteArray::GetBuffer() const {
		return _buffer.data();
	}

	uint8_t* ByteArray::GetBuffer() {
		return _buffer.data();
	}

	ByteArray::ByteArray(ByteArray&& rhs) noexcept
		:
		_buffer(std::move(rhs._buffer)) {
	}

	ByteArray& ByteArray::operator=(ByteArray&& rhs) noexcept {
		_buffer = std::move(rhs._buffer);
		return *this;
	}

}