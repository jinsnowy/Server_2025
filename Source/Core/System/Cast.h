#pragma once

#include "Core/System/String.h"
#include "Core/System/DateTime.h"
#include "Core/System/Duration.h"
#include "Core/System/Time.h"

namespace System {

	template<typename CastFrom, typename CastTo>
	static CastTo Cast(const CastFrom& value) {
		return {};
	}


	template<>
	inline std::string Cast(const std::wstring& value) {
		return String::Convert(value);
	}

	template<>
	inline std::wstring Cast(const std::string& value) {
		return String::Convert(value);
	}

	template<>
	inline std::string Cast(const std::wstring_view& value) {
		return String::Convert(value);
	}

	template<>
	inline std::wstring Cast(const std::string_view& value) {
		return String::Convert(value);
	}
}
