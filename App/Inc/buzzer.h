#pragma once

#include <stm32g0xx_hal.h>

#include "main.h"

class Buzzer
{
    TIM_HandleTypeDef* const _timer_handler;
    const uint32_t _timer_channel;
    const uint32_t _beep_length_ms;

    bool _is_beep = false;
    uint32_t _beep_ms = 0;

public:
    Buzzer(TIM_HandleTypeDef* const timer_handler, const uint32_t timer_channel, const uint32_t beep_length_ms) : _timer_handler(timer_handler), _timer_channel(timer_channel), _beep_length_ms(beep_length_ms)
    {

    }

    void Begin();

    void Update(const uint32_t delay_ms);

    bool Beep();
};