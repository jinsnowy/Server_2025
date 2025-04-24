#pragma once

namespace System {

class Diagnostics final {
public:
    static std::string GetCurrentWorkingDirectory();
    static std::string GetProgramName();
};
}
