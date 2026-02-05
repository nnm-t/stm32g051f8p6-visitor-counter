#pragma once

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <stm32g0xx_hal.h>

#include "main.h"

#include "pcf8563.h"
#include "buzzer.h"
#include "sd.h"
#include "led.h"

extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim14;
extern UART_HandleTypeDef huart1;

class VisitorCounter
{
    static constexpr const uint32_t delay_ms = 10;
    static constexpr const uint32_t sw_interval_ms = 1000;
    static constexpr const uint32_t sw_buzzer_ms = 50;
    static constexpr const uint32_t led_sw_on_ms = sw_interval_ms;
    static constexpr const size_t csv_line_len = 20;
    static constexpr const char* csv_write_sw1 = "1";
    static constexpr const char* csv_write_sw2 = "2";
    static constexpr const size_t uart_buffer_len = 16;
    static constexpr const size_t command_buffer_len = 128;
    static constexpr const char command_delimiter = '\n';
    static constexpr const char* command_set_time = "set";

    PCF8563 _pcf8563 = PCF8563(&hi2c2);
    Buzzer _buzzer = Buzzer(&htim14, TIM_CHANNEL_1, sw_buzzer_ms);
    SD _sd = SD();
    File _file = File::Empty();

    LED _led1 = LED(LED1_GPIO_Port, LED1_Pin);
    LED _led2 = LED(LED2_GPIO_Port, LED2_Pin);

    uint8_t _uart_buffer[uart_buffer_len] = { 0 };
    char _command_buffer[command_buffer_len] = { 0 };
    size_t _command_length = 0;
    bool _do_command = false;

    bool _time_set = false;
    bool _file_opened = false;
    bool _sw1_pressed = false;
    bool _sw2_pressed = false;

    uint32_t _sw1_pressed_ms = 0;
    uint32_t _sw2_pressed_ms = 0;

    void WriteCSVLine(const DateTime& date_time, const char* str);

    void ParseCommand();

public:
    VisitorCounter()
    {

    }

    void Begin();

    void Update();

    void SW1Pressed();

    void SW2Pressed();

    void UARTRXComplete(UART_HandleTypeDef* const huart);
};