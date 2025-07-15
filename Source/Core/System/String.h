#pragma once

namespace System {
    class String final {
    public:
        static std::string Convert(const std::wstring_view& wstr);
        static std::wstring Convert(const std::string_view& str);

        static std::string Trim(const std::string_view& str) {
            auto start = str.find_first_not_of(" \t\n\r");
            auto end = str.find_last_not_of(" \t\n\r");
            if (start == std::string_view::npos) {
                start = 0;
            }
            if (end == std::string_view::npos) {
                end = str.length() - 1;
			}

            return std::string(str.substr(start, end - start + 1));
		}

        static std::wstring Trim(const std::wstring_view& wstr) {
            auto start = wstr.find_first_not_of(L" \t\n\r");
            auto end = wstr.find_last_not_of(L" \t\n\r");
            if (start == std::wstring_view::npos) {
                start = 0;
            }
            if (end == std::wstring_view::npos) {
                end = wstr.length() - 1;
            }
            return std::wstring(wstr.substr(start, end - start + 1));
        }

        static std::string ToLower(const std::string_view& str) {
            std::string lower_str(str);
            for (auto& c : lower_str) {
                c = static_cast<char>(std::tolower(c));
            }
            return lower_str;
		}

        static std::wstring ToLower(const std::wstring_view& wstr) {
            std::wstring lower_wstr(wstr);
            for (auto& c : lower_wstr) {
                c = static_cast<wchar_t>(std::tolower(c));
            }
            return lower_wstr;
        }

        static std::string ToUpper(const std::string_view& str) {
            std::string upper_str(str);
            for (auto& c : upper_str) {
                c = static_cast<char>(std::toupper(c));
            }
            return upper_str;
        }

        static std::wstring ToUpper(const std::wstring_view& wstr) {
            std::wstring upper_wstr(wstr);
            for (auto& c : upper_wstr) {
                c = static_cast<wchar_t>(std::toupper(c));
            }
            return upper_wstr;
		}
    };


}
