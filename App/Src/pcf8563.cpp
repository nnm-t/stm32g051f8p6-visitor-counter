#include "pcf8563.h"

const uint8_t PCF8563::BcdToDec(const uint8_t value)
{
    return (((value & 0xF0) >> 4) * 10) + (value & 0x0F);
}

const uint8_t PCF8563::DecToBcd(const uint8_t value)
{
    return (((value / 10) % 10) << 4) + (value % 10);
}

bool PCF8563::IsRunning()
{
    // Control Status 1, Control Status 2
    uint8_t control_status[2];
    const HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(_i2c_handle, i2c_address, register_control_status_1, register_size, control_status, sizeof(control_status), i2c_timeout);

    if (ret != HAL_OK)
    {
        return false;
    }

    return !(control_status[0] & control_status_1_stop);
}

bool PCF8563::Begin()
{
    return IsRunning();
}

bool PCF8563::WasVoltageLow()
{
    uint8_t value[1];
    const HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(_i2c_handle, i2c_address, register_vl_seconds, register_size, value, sizeof(value), i2c_timeout);

    if (ret != HAL_OK)
    {
        return false;
    }

    return (value[0] & 0x80) >> 7;
}

DateTime PCF8563::Now()
{
    uint8_t value[7];
    const HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(_i2c_handle, i2c_address, register_vl_seconds, register_size, value, sizeof(value), i2c_timeout);

    if (ret != HAL_OK)
    {
        return DateTime();
    }

    // BCD形式
    const uint8_t seconds = BcdToDec(value[0] & 0x7F);
    const uint8_t minutes = BcdToDec(value[1] & 0x7F);
    const uint8_t hours = BcdToDec(value[2] & 0x3F);
    const uint8_t days = BcdToDec(value[3] & 0x3F);
    const Weekday weekdays = static_cast<Weekday>(value[4]);
    const uint8_t months = BcdToDec(value[5] & 0x1F);
    const uint16_t years = BcdToDec(value[6]) + 2000;

    return DateTime(years, months, weekdays, days, hours, minutes, seconds);
}

bool PCF8563::Adjust(const DateTime& date_time)
{
    uint8_t value[7];

    value[0] = DecToBcd(date_time.GetSeconds());
    value[1] = DecToBcd(date_time.GetMinutes());
    value[2] = DecToBcd(date_time.GetHours());
    value[4] = DecToBcd(static_cast<uint8_t>(date_time.GetWeekdays()));
    value[5] = DecToBcd(date_time.GetMonths());
    value[6] = DecToBcd(date_time.GetYears() - 2000);

    const HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(_i2c_handle, i2c_address, register_vl_seconds, register_size, value, sizeof(value), i2c_timeout);

    if (ret != HAL_OK)
    {
        return false;
    }

    return true;
}