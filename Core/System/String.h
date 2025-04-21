#pragma once

namespace System {
    class String final {
    public:
        static std::string Convert(const std::wstring& wstr);
        static std::wstring Convert(const std::string& str);
    };
}
