#include "led.h"

void LED::On(const uint32_t on_ms)
{
    _is_on = true;
    _is_continuous = false;
    _remaining_ms = on_ms;
    HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_SET);
}

void LED::OnContinuous()
{
    _is_on = true;
    _is_continuous = true;
    _remaining_ms = 0;
    HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_SET);
}

void LED::Off()
{
    _is_on = false;
    _is_continuous = false;
    _remaining_ms = 0;

    HAL_GPIO_WritePin(_port, _pin, GPIO_PIN_RESET);
}

void LED::Update(const uint32_t delay_ms)
{
    if (!_is_on)
    {
        return;
    }

    if (_is_continuous)
    {
        return;
    }

    _remaining_ms -= delay_ms;

    if (_remaining_ms <= 0)
    {
        // LED OFF
        Off();
    }
}