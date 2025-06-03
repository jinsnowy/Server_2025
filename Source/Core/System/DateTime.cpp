#include "stdafx.h"
#include "DateTime.h"

namespace System {

    DateTime::DateTime() 
        :
        year_(1970),
        month_(1),
        day_(1),
        hour_(0),
        day_of_week_(DayOfWeek::Thursday),
        minute_(0),
        second_(0),
        millisecond_(0) {
    }

    DateTime::DateTime(const TimePoint& time_point) {
        const int64_t& posix_milliseconds = 
            std::chrono::duration_cast<std::chrono::milliseconds>(time_point.internal_time_point().time_since_epoch()).count();
		
        time_t time_t_value = static_cast<time_t>(posix_milliseconds / 1000LL); // Convert milliseconds to seconds
		tm time_info;
		localtime_s(&time_info, &time_t_value);

		year_ = time_info.tm_year + 1900; 
		month_ = time_info.tm_mon + 1;
		day_ = time_info.tm_mday;
		hour_ = time_info.tm_hour;
		minute_ = time_info.tm_min;
		second_ = time_info.tm_sec;
		day_of_week_ = static_cast<DayOfWeek>(time_info.tm_wday);
		millisecond_ = posix_milliseconds % 1000;
    }

    DateTime DateTime::Now() {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        
        DateTime date_time;
        tm time_info;
        localtime_s(&time_info, &now_time);

        date_time.year_ = time_info.tm_year + 1900; 
        date_time.month_ = time_info.tm_mon + 1;
        date_time.day_ = time_info.tm_mday;
        date_time.hour_ = time_info.tm_hour;
        date_time.minute_ = time_info.tm_min;
        date_time.second_ = time_info.tm_sec;
        date_time.day_of_week_ = static_cast<DayOfWeek>(time_info.tm_wday);
        date_time.millisecond_ = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

        return date_time;
    }

    DateTime DateTime::UtcNow() {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        DateTime date_time;
        tm time_info;
        gmtime_s(&time_info, &now_time);

        date_time.year_ = time_info.tm_year + 1900;
        date_time.month_ = time_info.tm_mon + 1;
        date_time.day_ = time_info.tm_mday;
        date_time.hour_ = time_info.tm_hour;
        date_time.minute_ = time_info.tm_min;
        date_time.second_ = time_info.tm_sec;
        date_time.day_of_week_ = static_cast<DayOfWeek>(time_info.tm_wday);
        date_time.millisecond_ = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

        return date_time;
    }

    uint16_t DateTime::year() const {
        return year_;
    }

    uint8_t DateTime::month() const {
        return month_;
    }

    uint8_t DateTime::day() const {
        return day_;
    }

    DayOfWeek DateTime::day_of_week() const {
        return day_of_week_;
    }

    uint8_t DateTime::hour() const {
        return hour_;
    }

    uint8_t DateTime::minute() const {
        return minute_;
    }

    uint8_t DateTime::second() const {
        return second_;
    }

    uint16_t DateTime::millisecond() const {
        return millisecond_;
    }

    std::string DateTime::ToString() const {
        return FORMAT("{:04d}-{:02d}-{:02d}_{:02d}:{:02d}:{:02d}.{:03d}", year(), month(), day(), hour(), minute(), second(), millisecond());
    }
}
