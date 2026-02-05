#include "date_time.h"

const uint16_t DateTime::GetYears() const
{
    return _years;
}

const uint8_t DateTime::GetMonths() const
{
    return _months;
}

const Weekday DateTime::GetWeekdays() const
{
    return _weekdays;
}

const uint8_t DateTime::GetDays() const
{
    return _days;
}

const uint8_t DateTime::GetHours() const
{
    return _hours;
}

const uint8_t DateTime::GetMinutes() const
{
    return _minutes;
}

const uint8_t DateTime::GetSeconds() const
{
    return _seconds;
}