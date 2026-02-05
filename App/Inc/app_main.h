#pragma once

#include <stm32g0xx_hal.h>

#ifdef __cplusplus
extern "C"
{
#endif

void app_setup(void);

void app_loop(void);

void app_gpio_rising(const uint16_t gpio_pin);

void app_uart_rx_complete(UART_HandleTypeDef* const huart);

#ifdef __cplusplus
}
#endif