#include "buzzer.h"

void Buzzer::Begin()
{
    HAL_TIM_PWM_Stop(_timer_handler, _timer_channel);
}

void Buzzer::Update(const uint32_t delay_ms)
{
    if (!_is_beep)
    {
        return;
    }

    if (_beep_ms >= _beep_length_ms)
    {
        HAL_TIM_PWM_Stop(_timer_handler, _timer_channel);
        _is_beep = false;
        return;
    }

    _beep_ms += delay_ms;
}

bool Buzzer::Beep()
{
    const HAL_StatusTypeDef ret = HAL_TIM_PWM_Start(_timer_handler, _timer_channel);
    _is_beep = true;
    _beep_ms = 0;

    if (ret != HAL_OK)
    {
        return false;
    }

    return true;
}