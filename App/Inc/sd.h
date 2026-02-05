#pragma once

#include <stm32g0xx_hal.h>

#include "main.h"
#include "app_fatfs.h"

#include "file.h"

enum class FileMode : BYTE
{
    Read = FA_READ,
    ReadWrite = FA_READ | FA_WRITE,
    CreateAlwaysWrite = FA_CREATE_ALWAYS | FA_WRITE,
    CreateAlwaysReadWrite = FA_CREATE_ALWAYS | FA_READ | FA_WRITE,
    OpenAlwaysWrite = FA_OPEN_ALWAYS | FA_WRITE,
    OpenAlwaysReadWrite = FA_OPEN_ALWAYS | FA_READ | FA_WRITE,
    OpenAppendWrite = FA_OPEN_APPEND | FA_WRITE,
    OpenAppendReadWrite = FA_OPEN_APPEND | FA_READ | FA_WRITE
};

class SD
{
    FATFS _fs;
    bool _is_mounted = false;

public:
    FRESULT Mount();

    FRESULT Unmount();
    
    File Open(const TCHAR* path, const FileMode mode);
};