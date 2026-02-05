#include "file.h"

File File::Open(FIL& file)
{
    return File(file);
}

File File::Empty()
{
    return File();
}

bool File::IsEmpty()
{
    return _is_empty;
}

FRESULT File::Seek(const FSIZE_t offset)
{
    if (_is_empty)
    {
        return FR_NO_FILE;
    }

    return f_lseek(&_file, offset);
}

FRESULT File::Rewind()
{
    if (_is_empty)
    {
        return FR_NO_FILE;
    }

    return f_rewind(&_file);
}

TCHAR* File::Gets(TCHAR* buff, const int32_t len)
{
    if (_is_empty)
    {
        return nullptr;
    }

    return f_gets(buff, len, &_file);
}

int32_t File::Putc(const TCHAR c)
{
    if (_is_empty)
    {
        return -1;
    }

    return f_putc(c, &_file);
}

int32_t File::Puts(const TCHAR* str)
{
    if (_is_empty)
    {
        return -1;
    }

    return f_puts(str, &_file);
}

int32_t File::Puts(const std::string& str)
{
    if (_is_empty)
    {
        return -1;
    }

    return f_puts(str.c_str(), &_file);
}

FRESULT File::Close()
{
    if (_is_empty)
    {
        return FR_NO_FILE;
    }

    return f_close(&_file);
}

FSIZE_t File::GetSize()
{
    if (_is_empty)
    {
        return 0;
    }

    return f_size(&_file);
}