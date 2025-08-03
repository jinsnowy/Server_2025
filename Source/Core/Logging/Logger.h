#pragma once

#include "Core/System/Singleton.h"
#include "Core/Container/MPSC.h"
#include "Core/System/Macro.h"

namespace Log {

namespace Detail {
    static constexpr std::string_view GetPrettyFileName(const char* file) {
        std::string_view file_view(file);
        size_t pos = file_view.find_last_of("/\\");
        if (pos == std::string_view::npos) {
            return file_view; // No path separator found, return the original string
        }
        return file_view.substr(file_view.find_last_of("/\\") + 1);
    }
} // namespace Detail

enum class Loglevel {
    kDebug,
    kInfo,
    kWarning,
    kError,
    kFatal
};

static constexpr const char* ToString(Loglevel loglevel) {
    switch (loglevel) {
        case Loglevel::kDebug:
            return "DEBUG";
        case Loglevel::kInfo:
            return "INFO";
        case Loglevel::kWarning:
            return "WARNING";
        case Loglevel::kError:
            return "ERROR";
        case Loglevel::kFatal:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

class Logger : public System::Singleton<Logger> {
public:
    ~Logger();

    template<typename ...Args>
    void Log(Loglevel loglevel, const std::string_view& fileline, const char* format, Args&& ...args) {
        LogInternal(loglevel, fileline, std::vformat(format, std::make_format_args(args...)));
    }

    template<typename ...Args>
    void Log(Loglevel logLevel, const std::string_view& fileline, const wchar_t* format, Args&& ...args) {
        LogInternal(logLevel, fileline, std::vformat(std::wstring_view(format), std::make_wformat_args(args...)));
    }

    void Log(Loglevel loglevel, const std::string_view& fileline, const std::string_view& message) {
        LogInternal(loglevel, fileline, message);
    }

    void Log(Loglevel logLevel, const std::string_view& fileline, const std::wstring_view& message) {
        LogInternal(logLevel, fileline, message);
    }

    static void Destroy();

private:
    friend class System::Singleton<Logger>;
    bool is_running_ = true;
    Container::MPSCQueue<std::string> log_queue_;

    Logger();

    void Run();

    std::string log_file_path_;
    std::thread log_thread_;
    std::ofstream log_file_;

    void LogInternal(Loglevel loglevel, const std::string_view& fileline, const std::string_view& message);
    void LogInternal(Loglevel loglevel, const std::string_view& fileline, const std::wstring_view& message);
};
}

#define __LOG_FILE_LINE__ Log::Detail::GetPrettyFileName(__FILELINE__)
#define LOG_DEBUG(...) Log::Logger::GetInstance().Log(Log::Loglevel::kDebug, __LOG_FILE_LINE__, __VA_ARGS__)
#define LOG_INFO(...) Log::Logger::GetInstance().Log(Log::Loglevel::kInfo, __LOG_FILE_LINE__, __VA_ARGS__)
#define LOG_WARNING(...) Log::Logger::GetInstance().Log(Log::Loglevel::kWarning, __LOG_FILE_LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Log::Logger::GetInstance().Log(Log::Loglevel::kError, __LOG_FILE_LINE__, __VA_ARGS__)
#define LOG_FATAL(...) Log::Logger::GetInstance().Log(Log::Loglevel::kFatal, __LOG_FILE_LINE__, __VA_ARGS__)