#include "file.h"

File File::Open(const TCHAR* path, const BYTE mode)
{
    FIL* file = new FIL();
    FRESULT res = f_open(file, path, mode);

    if (res == FR_OK)
    {
        return File(file);
    }

    return File();
}

File File::Empty()
{
    return File();
}

bool File::IsEmpty()
{
    return _file == nullptr;
}

FRESULT File::Seek(const FSIZE_t offset)
{
    if (IsEmpty())
    {
        return FR_NO_FILE;
    }

    return f_lseek(_file, offset);
}

FRESULT File::Rewind()
{
    if (IsEmpty())
    {
        return FR_NO_FILE;
    }

    return f_rewind(_file);
}

TCHAR* File::Gets(TCHAR* buff, const int32_t len)
{
    if (IsEmpty())
    {
        return nullptr;
    }

    return f_gets(buff, len, _file);
}

int32_t File::Putc(const TCHAR c)
{
    if (IsEmpty())
    {
        return -1;
    }

    return f_putc(c, _file);
}

int32_t File::Puts(const TCHAR* str)
{
    if (IsEmpty())
    {
        return -1;
    }

    return f_puts(str, _file);
}

int32_t File::Puts(const std::string& str)
{
    if (IsEmpty())
    {
        return -1;
    }

    return f_puts(str.c_str(), _file);
}

FRESULT File::Close()
{
    if (IsEmpty())
    {
        return FR_NO_FILE;
    }


    const FRESULT res = f_close(_file);

    delete _file;
    _file = nullptr;

    return res;
}

FSIZE_t File::GetSize()
{
    if (IsEmpty())
    {
        return 0;
    }

    return f_size(_file);
}