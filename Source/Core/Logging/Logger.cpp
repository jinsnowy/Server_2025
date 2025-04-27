#include "stdafx.h"
#include "Logger.h"
#include "Core/System/Diagnostics.h"
#include "Core/System/Scheduler.h"
#include "Core/System/String.h"
#include "Core/System/DateTime.h"
#include "Core/System/Path.h"

namespace Log {
    
    Logger::Logger() {
        try {
            std::string current_working_dir = System::Diagnostics::GetCurrentWorkingDirectory();
            std::string program_name = System::Path::GetFileNameWithoutExtension(System::Diagnostics::GetProgramName());
            auto today_date = System::DateTime::Now();
            std::string today_date_formatted = std::format("{:04}-{:02}-{:02}_{:02}-{:02}-{:02}",
                today_date.year(), today_date.month(), today_date.day(), today_date.hour(), today_date.minute(), today_date.second());
            std::filesystem::path log_file_path = std::filesystem::path(current_working_dir);
            std::wstring file_name = FORMAT(L"{}_{}.log", System::String::Convert(program_name), System::String::Convert(today_date_formatted));
            log_file_path.append(file_name);
            
            log_file_path_ = log_file_path.string();
            
            log_file_.open(log_file_path_, std::ios::out | std::ios::app);
            if (!log_file_.is_open()) {
                throw std::runtime_error("Failed to open log file: " + log_file_path_);
            }
            
            is_running_ = true;
            log_thread_ = std::thread(&Logger::Run, this);
        }
        catch (const std::exception& e) {
            std::cerr << "Logger initialization failed: " << e.what() << std::endl;
            throw;
        }
    }

    Logger::~Logger() {
        try {
            while (log_queue_.IsEmpty() == false) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            is_running_ = false;

            if (log_thread_.joinable()) {
                log_thread_.join();
            }
            
            if (log_file_.is_open()) {
                log_file_.close();
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Logger cleanup failed: " << e.what() << std::endl;
        }
    }

    void Logger::LogInternal(Loglevel loglevel, const char* function, int32_t line, const std::string_view& message) {
        std::string time_str = System::DateTime::Now().ToString();
        log_queue_.Push(std::format("{}[{}][{}][{}({})] {}", time_str, ToString(loglevel), System::Scheduler::ThreadId(), function, line, message));
    }

    void Logger::LogInternal(Loglevel loglevel, const char* function, int32_t line, const std::wstring_view& message) {
        std::string time_str = System::DateTime::Now().ToString();
        std::string converted_message = System::String::Convert(message);
        log_queue_.Push(std::format("{}[{}][{}][{}({})] {}", time_str, ToString(loglevel), System::Scheduler::ThreadId(), function, line,  std::move(converted_message)));
    }

    void Logger::Run() {
        std::string message;
        while (is_running_) {
            if (log_queue_.TryPop(message)) {
                log_file_ << std::move(message) << std::endl;
                fprintf_s(stdout, "%s\n", message.c_str());
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
}