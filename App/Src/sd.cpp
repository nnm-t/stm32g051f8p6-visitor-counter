#include "sd.h"

FRESULT SD::Mount()
{
    if (_is_mounted)
    {
        return FR_DISK_ERR;
    }

    const FRESULT res = f_mount(&_fs, "", 0);

    if (res == FR_OK)
    {
        _is_mounted = true;
    }

    return res;
}

FRESULT SD::Unmount()
{
    if (!_is_mounted)
    {
        return FR_DISK_ERR;
    }

    const FRESULT res = f_mount(nullptr, "", 1);

    if (res == FR_OK)
    {
        _is_mounted = false;
    }

    return res;
}

File SD::Open(const TCHAR* path, const FileMode mode)
{
    if (!_is_mounted)
    {
        return File::Empty();
    }

    FIL file;
    const FRESULT res = f_open(&file, path, static_cast<BYTE>(mode));

    if (res == FR_OK)
    {
        return File::Open(file);
    }

    return File::Empty();
}