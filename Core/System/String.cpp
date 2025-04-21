#include "stdafx.h"
#include "String.h"
#include <boost/locale/encoding.hpp>

namespace System
{
	std::string String::Convert(const std::wstring& wstr) {
		return boost::locale::conv::utf_to_utf<char>(wstr);
	}

	std::wstring String::Convert(const std::string& str) {
		return boost::locale::conv::utf_to_utf<wchar_t>(str);
	}
}