#pragma once

#include <string>

#include <stm32g0xx_hal.h>

#include "main.h"
#include "app_fatfs.h"

class File
{
    FIL _file;
    bool _is_empty;

    File(FIL& file) : _file(file), _is_empty(false)
    {

    }

    File() : _file(FIL()), _is_empty(true)
    {

    }

public:
    static File Open(FIL& file);

    static File Empty();

    bool IsEmpty();

    FRESULT Seek(const FSIZE_t offset);

    FRESULT Rewind();

    TCHAR* Gets(TCHAR* buff, const int32_t len);

    int32_t Putc(const TCHAR c);

    int32_t Puts(const TCHAR* str);

    int32_t Puts(const std::string& str);

    FRESULT Close();

    FSIZE_t GetSize();
};