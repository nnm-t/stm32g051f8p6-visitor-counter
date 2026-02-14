#include "main.h"
#include "app_main.h"
#include "visitor_counter.h"

namespace {
    constexpr const uint8_t sw1_exti_line = 1;
    constexpr const uint8_t sw2_exti_line = 2;

    VisitorCounter visitor_counter;
}

void app_setup(void)
{
    visitor_counter.Begin();
}

void app_loop(void)
{
    visitor_counter.Update();
}

void app_gpio_rising(const uint16_t gpio_pin)
{
    switch (gpio_pin)
    {
        case sw1_exti_line:
            visitor_counter.SW1Pressed();
        break;
        case sw2_exti_line:
            visitor_counter.SW2Pressed();
        break;
    }
}

void app_uart_rx_complete(UART_HandleTypeDef* const huart)
{
    visitor_counter.UARTRXComplete(huart);
}