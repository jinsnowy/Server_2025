#include "stdafx.h"
#include "String.h"
#include <boost/locale/encoding.hpp>

namespace System
{
	std::string String::Convert(const std::wstring_view& wstr) {
		return boost::locale::conv::utf_to_utf<char>(wstr.data());
	}

	std::wstring String::Convert(const std::string_view& str) {
		return boost::locale::conv::utf_to_utf<wchar_t>(str.data());
	}
}