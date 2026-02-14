#pragma once

#include <string>

#include <stm32g0xx_hal.h>

#include "main.h"
#include "app_fatfs.h"

class File
{
    FIL* _file;

    File(FIL* file) : _file(file)
    {

    }

    File() : _file(nullptr)
    {

    }

public:
    static File Open(const TCHAR* path, const BYTE mode);

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