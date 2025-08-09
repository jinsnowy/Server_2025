#pragma once

namespace System {
    enum class DayOfWeek : uint8_t {
		Sunday = 0,
		Monday,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday
	};

    class Time;
    class DateTime final {
    public:
        DateTime();
        DateTime(const Time& time);
        DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond);

        static DayOfWeek GetDayOfWeek(uint16_t year, uint8_t month, uint8_t day);
        static DateTime Now();
        static DateTime UtcNow();

        uint16_t year() const;
        uint8_t month() const;
        uint8_t day() const;
        DayOfWeek day_of_week() const;
        uint8_t hour() const;
        uint8_t minute() const;
        uint8_t second() const;
        uint16_t millisecond() const;

        std::string ToString() const;

        int64_t GetPosixMilliseconds() const;
        int64_t GetPosixSeconds() const;

    private:
        uint16_t year_;
        uint8_t month_;
        uint8_t day_;
        DayOfWeek day_of_week_;
        uint8_t hour_;
        uint8_t minute_;
        uint8_t second_;
        uint16_t millisecond_;
    };
}

