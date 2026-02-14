#include "visitor_counter.h"

void VisitorCounter::WriteCSVLine(const DateTime& date_time, const char* str)
{
    if (!_time_set)
    {
        return;
    }

    if (_file.IsEmpty())
    {
        return;
    }

    // CSV行テキスト生成
    char csv_line[csv_line_len];
    snprintf(csv_line, sizeof(csv_line), "%02d:%02d:%02d,%s\n", date_time.GetHours(), date_time.GetMinutes(), date_time.GetSeconds(), str);
    // ファイル書き出し
    _file.Puts(csv_line);
}

void VisitorCounter::ParseCommand()
{
    // 日付設定コマンド パース
    char command[16];
    struct tm t;
    int years = 0;
    int months = 0;

    const int32_t assign_args = sscanf(_command_buffer, "%s %d/%d/%d %d:%d:%d", command, &years, &months, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);

    // 年: 1900年基準、 月: 0～11で表現
    t.tm_year = years - 1900;
    t.tm_mon = months - 1;
    // 夏時間は無効
    t.tm_isdst = 0;

    if (assign_args == 1 && strcmp(command, command_get_time) == 0)
    {
        // 時刻取得コマンド
        DateTime date_time = _pcf8563.Now();

        char response[27];
        snprintf(response, sizeof(response), "%4d/%2d/%2d %2d:%2d:%2d\n", date_time.GetYears(), date_time.GetMonths(), date_time.GetDays(), date_time.GetHours(), date_time.GetMinutes(), date_time.GetSeconds());
        // 送信
        Print(response);

        return;
    }
    else if (assign_args != 7)
    {
        // 代入できた変数の個数が一致しない場合
        Print("Invalid arguments.\n");
        return;
    }

    if (strcmp(command, command_set_time) != 0)
    {
        // コマンド名不一致
        Print("Invalid command.\n");
        return;
    }

    // カレンダー時間を生成 (曜日を算出)
    if (mktime(&t) == -1)
    {
        // 生成に失敗
        return;
    }

    // 曜日
    const Weekday weekday = static_cast<Weekday>(t.tm_wday);
    // 時刻設定
    const DateTime date_time(years, months, weekday, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    _pcf8563.Adjust(date_time);

    // フラグ下がったか確認
    if (_pcf8563.WasVoltageLow())
    {
        return;
    }

    Print("RTC configure OK.\n");

    _time_set = true;
    // LED1 OFF
    _led1.Off();
}

void VisitorCounter::Print(const char* message)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), uart_timeout);
}

void VisitorCounter::Begin()
{
    _buzzer.Begin();

    Print("Startup...\n");

    // LED1 ON
    _led1.OnContinuous();
    // LED2 ON
    _led2.OnContinuous();

    // UART割り込み開始
    HAL_UART_Receive_IT(&huart1, _uart_buffer, 1);

    // PCF8563初期化
    _pcf8563.Begin();

    if (_pcf8563.WasVoltageLow())
    {
        // 時刻未設定
        Print("RTC is not configured time.\n");
        return;
    }

    _time_set = true;
    // LED1 OFF
    _led1.Off();

    // SDカード初期化
    const FRESULT sd_mount_result = _sd.Mount();

    if (sd_mount_result != FR_OK)
    {
        Print("TF card can't mount.\n");
        return;
    }

    // 時刻を取得
    DateTime date_time = _pcf8563.Now();
    // ファイル名を生成
    char file_name[21] = { 0 };
    snprintf(file_name, sizeof(file_name), "%04d_%02d_%02d.csv", date_time.GetYears(), date_time.GetMonths(), date_time.GetDays());
    // 日付の名前でファイルを開く (無ければ新規作成、末尾に追記)
    _file = _sd.Open(file_name, FileMode::OpenAppendReadWrite);

    if (_file.IsEmpty())
    {
        char message[50];
        snprintf(message, sizeof(message), "%s is empty. Create new file.\n", file_name);
        Print(message);
    }
    else
    {
        char message[38];
        snprintf(message, sizeof(message), "Write data to %s.\n", file_name);
        Print(message);
    }

    _file_opened = true;

    Print("TF card init completed.\n");

    // LED2 OFF
    _led2.Off();
}

void VisitorCounter::Update()
{
    // コマンド実行
    if (_do_command)
    {
        ParseCommand();

        // フラグ、バッファ等リセット
        _do_command = false;
        _command_length = 0;
        memset(_command_buffer, 0, sizeof(_command_buffer));
    }

    HAL_Delay(delay_ms);

    _buzzer.Update(delay_ms);

    _led1.Update(delay_ms);
    _led2.Update(delay_ms);

    // SW1押した直後
    if (_sw1_pressed && _sw1_pressed_ms <= 0)
    {
        Print("SW1 Pressed.\n");
        // LED1 ON
        _led1.On(led_sw_on_ms);

        // ブザー鳴動
        _buzzer.Beep();

        // 時刻を取得
        DateTime date_time = _pcf8563.Now();
        // CSV書き出し
        WriteCSVLine(date_time, csv_write_sw1);
    }

    if (_sw2_pressed && _sw2_pressed_ms == 0)
    {
        Print("SW2 Pressed.\n");
        // LED2 ON
        _led2.On(led_sw_on_ms);

        // ブザー鳴動
        _buzzer.Beep();

        // 時刻を取得
        DateTime date_time = _pcf8563.Now();
        // CSV書き出し
        WriteCSVLine(date_time, csv_write_sw2);
    }

    if (_sw1_pressed)
    {
        _sw1_pressed_ms += delay_ms;

        // sw_interval_ms 経過
        if (_sw1_pressed_ms >= sw_interval_ms)
        {
            _sw1_pressed = false;
        }
    }

    if (_sw2_pressed)
    {
        _sw2_pressed_ms += delay_ms;

        if (_sw2_pressed_ms >= sw_interval_ms)
        {
            _sw2_pressed = false;
        }
    }
}

void VisitorCounter::SW1Pressed()
{
    if (_sw1_pressed)
    {
        return;
    }

    _sw1_pressed = true;
    _sw1_pressed_ms = 0;
}

void VisitorCounter::SW2Pressed()
{
    if (_sw2_pressed)
    {
        return;
    }

    _sw2_pressed = true;
    _sw2_pressed_ms = 0;
}

void VisitorCounter::UARTRXComplete(UART_HandleTypeDef* const huart)
{
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

    if (huart != &huart1)
    {
        return;
    }

    if (_uart_buffer[0] == command_delimiter)
    {
        // 改行コード投入でコマンド実行
        _do_command = true;
    }
    else if (_command_length >= command_buffer_len - 1)
    {
        // _command_buffer を使い切った時
    }
    else
    {
        memcpy(_command_buffer + _command_length, _uart_buffer, 1);
        _command_length++;
    }

    HAL_UART_Receive_IT(&huart1, _uart_buffer, 1);
}