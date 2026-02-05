#pragma once

#include <stm32g0xx_hal.h>

#include "main.h"

class LED
{
    GPIO_TypeDef* const _port;
    const uint16_t _pin;

    bool _is_on = false;
    bool _is_continuous = false;
    uint32_t _remaining_ms = 0;

public:
    LED(GPIO_TypeDef* const port, const uint16_t pin) : _port(port), _pin(pin)
    {

    }

    void On(const uint32_t on_ms);

    void OnContinuous();

    void Off();

    void Update(const uint32_t delay_ms);
};