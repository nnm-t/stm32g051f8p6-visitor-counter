#pragma once

#include <stm32g0xx_hal.h>

#include "main.h"
#include "date_time.h"

class PCF8563
{
    static constexpr const uint8_t i2c_address = 0x51;
    static constexpr const uint32_t i2c_timeout = 100;

    static constexpr const uint16_t register_size = 1;

    static constexpr const uint8_t register_control_status_1 = 0x00;
    static constexpr const uint8_t register_control_status_2 = 0x01;
    static constexpr const uint8_t register_vl_seconds = 0x02;
    static constexpr const uint8_t register_minutes = 0x03;
    static constexpr const uint8_t register_hours = 0x04;
    static constexpr const uint8_t register_days = 0x05;
    static constexpr const uint8_t register_weekdays = 0x06;
    static constexpr const uint8_t register_century_months = 0x07;
    static constexpr const uint8_t register_years = 0x08;
    
    static constexpr const uint8_t control_status_1_test1 = 0x80;
    static constexpr const uint8_t control_status_1_stop = 0x20;
    static constexpr const uint8_t control_status_1_testc = 0x08;

    I2C_HandleTypeDef* const _i2c_handle;

    bool IsRunning();

    static const uint8_t BcdToDec(const uint8_t value);

    static const uint8_t DecToBcd(const uint8_t value);

public:
    PCF8563(I2C_HandleTypeDef* const i2c_handle) : _i2c_handle(i2c_handle)
    {

    }

    bool Begin();

    bool WasVoltageLow();

    DateTime Now();

    bool Adjust(const DateTime& date_time);
};