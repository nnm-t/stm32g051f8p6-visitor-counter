#include "fatfs_sd.h"

#define bool            BYTE
#define TRUE            1
#define FALSE           0

volatile uint16_t timer1;
volatile uint16_t timer2;

static volatile DSTATUS status = STA_NOINIT;
static uint8_t card_type;
static uint8_t power_flag = 0;

static void SPI_cs_select()
{
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void SPI_cs_deselect()
{
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void SPI_tx_byte(uint8_t data)
{
    while(!__HAL_SPI_GET_FLAG(HSPI_SDCARD, SPI_FLAG_TXE))
    {

    }

    HAL_SPI_Transmit(HSPI_SDCARD, &data, sizeof(data), SPI_TIMEOUT);
}

static void SPI_tx_buffer(uint8_t* buffer, uint16_t len)
{
    while(!__HAL_SPI_GET_FLAG(HSPI_SDCARD, SPI_FLAG_TXE))
    {

    }

    HAL_SPI_Transmit(HSPI_SDCARD, buffer, len, SPI_TIMEOUT);
}

static uint8_t SPI_rx_byte()
{
    while(!__HAL_SPI_GET_FLAG(HSPI_SDCARD, SPI_FLAG_TXE))
    {

    }

    uint8_t dummy = 0;
    uint8_t data;
    HAL_SPI_TransmitReceive(HSPI_SDCARD, &dummy, &data, 1, SPI_TIMEOUT);

    return data;
}

static void SPI_rx_byte_ptr(uint8_t* buff)
{
    *buff = SPI_rx_byte();
}

static uint8_t SD_ready_wait()
{
    timer2 = 500;

    uint8_t res;
    do
    {
        res = SPI_rx_byte();
    } while ((res != 0xFF) && timer2);

    return res;
}

static void SD_power_on()
{
    SPI_cs_deselect();

    for (int i= 0; i < 10; i++)
    {
        SPI_tx_byte(0xFF);
    }

    SPI_cs_select();

    uint8_t args[6] = { CMD0, 0, 0, 0, 0, 0x95 };

    SPI_tx_buffer(args, sizeof(args));

    uint32_t count = 0x1FFFF;
    while ((SPI_rx_byte() != 0x01) && count)
    {
        count--;
    }

    SPI_cs_deselect();
    SPI_tx_byte(0xFF);

    power_flag = 1;
}

static void SD_power_off()
{
    power_flag = 0;
}

static uint8_t SD_check_power()
{
    return power_flag;
}

static bool SD_rx_data_block(BYTE* buff, UINT len)
{
    timer1 = 200;

    uint8_t token;
    do
    {
        token = SPI_rx_byte();
    } while ((token == 0xFF) && timer1);

    if (token != 0xFE)
    {
        return FALSE;
    }

    while (len--)
    {
        SPI_rx_byte_ptr(buff++);
    }

    SPI_rx_byte();
    SPI_rx_byte();

    return TRUE;
}

#if _USE_WRITE == 1
static bool SD_tx_data_block(const uint8_t* buff, BYTE token)
{
    if (SD_ready_wait() != 0xFF)
    {
        return FALSE;
    }

    SPI_tx_byte(token);

    uint8_t resp = 0xFF;
    if (token != 0xFD)
    {
        SPI_tx_buffer((uint8_t*)buff, 512);

        SPI_rx_byte();
        SPI_rx_byte();

        uint8_t i = 0;
        while (i <= 64)
        {
            resp = SPI_rx_byte();

            if ((resp & 0x1F) == 0x05)
            {
                break;
            }

            i++;
        }

        timer1 = 200;
        while ((SPI_rx_byte() == 0) && timer1)
        {

        }
    }

    if ((resp & 0x1F) == 0x05)
    {
        return TRUE;
    }

    return FALSE;
}
#endif

static BYTE SD_send_cmd(BYTE cmd, uint32_t arg)
{
    if (SD_ready_wait() != 0xFF)
    {
        return 0xFF;
    }

    SPI_tx_byte(cmd);
    SPI_tx_byte((uint8_t)(arg >> 24));
    SPI_tx_byte((uint8_t)(arg >> 16));
    SPI_tx_byte((uint8_t)(arg >> 8));
    SPI_tx_byte((uint8_t)arg);

    uint8_t crc;
    if (cmd == CMD0)
    {
        crc = 0x95;
    }
    else if (cmd == CMD8)
    {
        crc = 0x87;
    }
    else
    {
        crc = 0x01;
    }

    SPI_tx_byte(crc);

    if (cmd == CMD12)
    {
        SPI_rx_byte();
    }

    uint8_t n = 10;
    uint8_t res;
    do
    {
        res = SPI_rx_byte();
    } while ((res & 0x80) && --n);

    return res;
}

DSTATUS SD_disk_initialize(BYTE drv)
{

    if (drv)
    {
        return STA_NOINIT;
    }

    if (status & STA_NODISK)
    {
        return status;
    }

    SD_power_on();

    SPI_cs_select();

    uint8_t type = 0;

    if (SD_send_cmd(CMD0, 0 == 1))
    {
        timer1 = 1000;

        if (SD_send_cmd(CMD8, 0x1AA) == 1)
        {
            uint8_t ocr[4];
            for (uint8_t n = 0; n < 4; n++)
            {
                ocr[n] = SPI_rx_byte();
            }

            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            {
                do
                {
                    if (SD_send_cmd(CMD55, 0) <= 1 && SD_send_cmd(CMD41, 1UL << 30) == 0)
                    {
                        break;
                    }
                } while (timer1);

                if (timer1 && SD_send_cmd(CMD58, 0) == 0)
                {
                    for (uint8_t n = 0; n < 4; n++)
                    {
                        ocr[n] = SPI_rx_byte();
                    }

                    type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        }
        else
        {
            type = (SD_send_cmd(CMD55, 0) <= 1 && SD_send_cmd(CMD41, 0) <= 1) ? CT_SD1 : CT_MMC;

            do
            {
                if (type == CT_SD1)
                {
                    if (SD_send_cmd(CMD55, 0) <= 1 && SD_send_cmd(CMD41, 0) == 0)
                    {
                        break;
                    }
                }
                else
                {
                    if (SD_send_cmd(CMD1, 0) == 0)
                    {
                        break;
                    }
                }
            } while (timer1);

            if (!timer1 || SD_send_cmd(CMD16, 512) != 0)
            {
                type = 0;
            }
        }
    }

    card_type = type;

    SPI_cs_deselect();
    SPI_rx_byte();

    if (type)
    {
        status &= ~STA_NOINIT;
    }
    else
    {
        SD_power_off();
    }

    return status;
}

DSTATUS SD_disk_status(BYTE drv)
{
    if (drv)
    {
        return STA_NOINIT;
    }

    return status;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv || !count)
    {
        return RES_PARERR;
    }

    if (status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    if (!(card_type & CT_BLOCK))
    {
        sector *= 512;
    }

    SPI_cs_select();

    if (count == 1)
    {
        if ((SD_send_cmd(CMD17, sector) == 0) && SD_rx_data_block(buff, 512))
        {
            count = 0;
        }
    }
    else
    {
        if (SD_send_cmd(CMD18, sector) == 0)
        {
            do
            {
                if (!SD_rx_data_block(buff, 512))
                {
                    break;
                }

                buff += 512;
            } while (--count);

            SD_send_cmd(CMD12, 0);
        }
    }

    SPI_cs_deselect();
    SPI_rx_byte();

    return count ? RES_ERROR : RES_OK;
}

#if _USE_WRITE == 1
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv || !count)
    {
        return RES_PARERR;
    }

    if (status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    if (status & STA_PROTECT)
    {
        return RES_WRPRT;
    }

    if (!(card_type & CT_BLOCK))
    {
        sector *= 512;
    }

    SPI_cs_select();

    if (count == 1)
    {
        if ((SD_send_cmd(CMD24, sector) == 0) && SD_tx_data_block(buff, 0xFE))
        {
            count = 0;
        }
    }
    else
    {
        if (card_type & CT_SD1)
        {
            SD_send_cmd(CMD55, 0);
            SD_send_cmd(CMD23, count);
        }

        if (SD_send_cmd(CMD25, sector) == 0)
        {
            do
            {
                if (!SD_tx_data_block(buff, 0xFC))
                {
                    break;
                }

                buff += 512;
            } while (--count);

            if (!SD_tx_data_block(0, 0xFD))
            {
                count = 1;
            }
        }
    }

    SPI_cs_deselect();
    SPI_rx_byte();

    return count ? RES_ERROR : RES_OK;
}
#endif

DRESULT SD_disk_ioctl(BYTE drv, BYTE ctrl, void* buff)
{
    if (drv)
    {
        return RES_PARERR;
    }

    DRESULT res = RES_ERROR;
    uint8_t* ptr = buff;

    if (ctrl == CTRL_POWER)
    {
        switch (*ptr)
        {
            case 0:
                SD_power_off();
                res = RES_OK;
                break;
            case 1:
                SD_power_on();
                res = RES_OK;
                break;
            case 2:
                *(ptr + 1) = SD_check_power();
                res = RES_OK;
                break;
            default:
                res = RES_PARERR;
                break;
        }
    }
    else
    {
        if (status & STA_NOINIT)
        {
            return RES_NOTRDY;
        }

        SPI_cs_select();

        uint8_t csd[16];
        switch (ctrl)
        {
            case GET_SECTOR_COUNT:
                if ((SD_send_cmd(CMD9, 0) == 0) && SD_rx_data_block(csd, 16))
                {
                    DWORD c_size = (DWORD)(csd[7] & 0x3F) << 16 | (WORD)csd[8] << 8 | csd[9];
                    *(DWORD*)buff = (c_size + 1) << 10;
                }
                else
                {
                    uint8_t n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    WORD c_size = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10) + 1;
                    *(DWORD*)buff = (DWORD)c_size << (n - 9);
                }

                res = RES_OK;
                break;
            case GET_SECTOR_SIZE:
                *(WORD*)buff = 512;
                res = RES_OK;
                break;
            case CTRL_SYNC:
                if (SD_ready_wait() == 0xFF)
                {
                    res = RES_OK;
                }
                break;
            case MMC_GET_CSD:
                if (SD_send_cmd(CMD9, 0) == 0&& SD_rx_data_block(ptr, 16))
                {
                    res = RES_OK;
                }
                break;
            case MMC_GET_CID:
                if (SD_send_cmd(CMD10, 0) == 0&& SD_rx_data_block(ptr, 16))
                {
                    res = RES_OK;
                }
                break;
            case MMC_GET_OCR:
            if (SD_send_cmd(CMD58, 0) == 0)
            {
                for (uint8_t n = 0; n < 4; n++)
                {
                    *ptr++ = SPI_rx_byte();
                }

                res = RES_OK;
            }
                break;
        }

        SPI_cs_deselect();
        SPI_rx_byte();
    }

    return res;
}