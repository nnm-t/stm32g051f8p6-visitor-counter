#pragma once

#include <stm32g0xx_hal.h>

#include "main.h"

enum class Weekday : uint8_t
{
    Sunday = 0x00,
    Monday = 0x01,
    Tuesday = 0x02,
    Wednesday = 0x03,
    Thursday = 0x04,
    Friday = 0x05,
    Saturday = 0x06,
    Null = 0xFF
};

struct DateTime
{
private:
    const uint16_t _years;
    const uint8_t _months;
    const Weekday _weekdays;
    const uint8_t _days;
    const uint8_t _hours;
    const uint8_t _minutes;
    const uint8_t _seconds;

public:
    DateTime(const uint16_t years, const uint8_t months, const Weekday weekdays, const uint8_t days, const uint8_t hours, const uint8_t minutes, const uint8_t seconds) : _years(years), _months(months), _weekdays(weekdays), _days(days), _hours(hours), _minutes(minutes), _seconds(seconds)
    {

    }

    DateTime() : _years(0x00), _months(0x00), _weekdays(Weekday::Null), _days(0x00), _hours(0x00), _minutes(0x00), _seconds(0x00)
    {

    }

    const uint16_t GetYears() const;

    const uint8_t GetMonths() const;

    const Weekday GetWeekdays() const;

    const uint8_t GetDays() const;

    const uint8_t GetHours() const;

    const uint8_t GetMinutes() const;

    const uint8_t GetSeconds() const;

};