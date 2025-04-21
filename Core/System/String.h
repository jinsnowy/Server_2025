#pragma once

namespace System {
    class String final {
    public:
        static std::string Convert(const std::wstring_view& wstr);
        static std::wstring Convert(const std::string_view& str);
    };
}
