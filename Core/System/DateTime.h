#pragma once
namespace System {
    class DateTime final {
    public:
        DateTime();

        static DateTime Now();
        static DateTime UtcNow();

        uint16_t year() const;
        uint8_t month() const;
        uint8_t day() const;
        uint8_t week_day() const;
        uint8_t hour() const;
        uint8_t minute() const;
        uint8_t second() const;
        uint16_t millisecond() const;

        std::string ToString() const;

    private:
        time_t time_t_;
        uint16_t year_;
        uint8_t month_;
        uint8_t day_;
        uint8_t week_day_;
        uint8_t hour_;
        uint8_t minute_;
        uint8_t second_;
        uint16_t millisecond_;
    };
}

