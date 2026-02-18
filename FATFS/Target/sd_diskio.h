#pragma once

#include "diskio.h"
#include "fatfs_sd.h"
#include "ff_gen_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const Diskio_drvTypeDef SD_Driver;

DSTATUS SD_disk_status(BYTE drv);
DSTATUS SD_disk_initialize(BYTE drv);
DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);

#ifdef __cplusplus
}
#endif