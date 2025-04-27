#pragma once

#include "Core/System/Singleton.h"
#include "Core/Concurrency/MPSC.h"

namespace Log {

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
    void Log(Loglevel loglevel, const char* function, int32_t line, const char* format, Args&& ...args) {
        LogInternal(loglevel, function, line, std::vformat(format, std::make_format_args(args...)));
    }

    template<typename ...Args>
    void Log(Loglevel logLevel, const char* function, int32_t line, const wchar_t* format, Args&& ...args) {
        LogInternal(logLevel, function, line, std::vformat(std::wstring_view(format), std::make_wformat_args(args...)));
    }

    void Log(Loglevel loglevel, const char* function, int32_t line, const std::string_view& message) {
        LogInternal(loglevel, function, line, message);
    }

    void Log(Loglevel logLevel, const char* function, int32_t line, const std::wstring_view& message) {
        LogInternal(logLevel, function, line, message);
    }

private:
    friend class System::Singleton<Logger>;
    bool is_running_ = true;
    Concurrency::MPSCQueue<std::string> log_queue_;

    Logger();

    void Run();

    std::string log_file_path_;
    std::thread log_thread_;
    std::ofstream log_file_;

    void LogInternal(Loglevel loglevel, const char* function, int32_t line, const std::string_view& message);
    void LogInternal(Loglevel loglevel, const char* function, int32_t line, const std::wstring_view& message);
};
}

#define LOG_DEBUG(...) Log::Logger::GetInstance().Log(Log::Loglevel::kDebug, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) Log::Logger::GetInstance().Log(Log::Loglevel::kInfo, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) Log::Logger::GetInstance().Log(Log::Loglevel::kWarning, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Log::Logger::GetInstance().Log(Log::Loglevel::kError, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) Log::Logger::GetInstance().Log(Log::Loglevel::kFatal, __FUNCTION__, __LINE__, __VA_ARGS__)