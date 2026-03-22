#include "fatfs_sd.h"

#include <string.h>

static void SPI_cs_select()
{
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET);
}

static void SPI_cs_deselect()
{
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);
}

static void SPI_tx_byte(uint8_t data)
{
    HAL_SPI_Transmit(HSPI_SDCARD, &data, 1, 100);
}

static void SPI_tx_buffer(uint8_t* buffer, uint16_t len)
{
    HAL_SPI_Transmit(HSPI_SDCARD, buffer, len, 100);
}

static uint8_t SPI_rx_byte()
{
    uint8_t dummy = 0xFF;
    uint8_t data = 0x00;
    HAL_SPI_TransmitReceive(HSPI_SDCARD, &dummy, &data, 1, 100);

    return data;
}

static void SPI_rx_buffer(uint8_t* buffer, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        buffer[i] = SPI_rx_byte();
    }
}

static SD_Status SD_ready_wait()
{
    uint32_t timeout = HAL_GetTick() + 500;
    uint8_t res;

    do
    {
        res = SPI_rx_byte();
        if (res == 0xFF)
        {
            return SD_OK;
        }

    } while (HAL_GetTick() < timeout);

    return SD_ERROR;
}

static BYTE SD_send_cmd(BYTE cmd, uint32_t arg)
{
    SD_ready_wait();
    SPI_tx_byte(cmd);
    SPI_tx_byte((uint8_t)(arg >> 24));
    SPI_tx_byte((uint8_t)(arg >> 16));
    SPI_tx_byte((uint8_t)(arg >> 8));
    SPI_tx_byte((uint8_t)arg);

    uint8_t crc = 0x01;
    if (cmd == CMD0)
    {
        crc = 0x95;
    }
    else if (cmd == CMD8)
    {
        crc = 0x87;
    }

    SPI_tx_byte(crc);

    uint8_t retry = 0xFF;
    uint8_t res = 0xFF;
    do
    {
        res = SPI_rx_byte();
    } while ((res & 0x80) && --retry);

    return res;
}

static uint8_t sdhc = 0;
uint8_t card_initialized = 0;

uint8_t sd_is_sdhc(void)
{
    return sdhc;
}

SD_Status SD_spi_init(void)
{
    SPI_cs_deselect();
    // 74クロック以上のダミークロックを送信
    for (uint8_t i = 0; i < 10; i++)
    {
        SPI_tx_byte(0xFF);
    }

    BYTE cmd0_res = 0xFF;
    uint32_t retry = HAL_GetTick() + 1000;
    do
    {
        SPI_cs_select();
        // ソフトウェアリセット
        cmd0_res = SD_send_cmd(CMD0, 0);
        SPI_cs_deselect();
        SPI_tx_byte(0xFF);
    }
    while (cmd0_res != 0x01 && HAL_GetTick() < retry);
    
    if (cmd0_res != 0x01)
    {
        return SD_ERROR;
    }

    BYTE cmd8_res = 0xFF;
    uint8_t r7[4];

    SPI_cs_select();
    // カード電圧範囲等取得
    cmd8_res = SD_send_cmd(CMD8, 0x1AA);
    for (uint8_t i = 0; i < 4; i++)
    {
        r7[i] = SPI_rx_byte();
    }
    SPI_cs_deselect();
    SPI_tx_byte(0xFF);

    sdhc = 0;
    retry = HAL_GetTick() + 1000;
    if (cmd8_res == 0x01 && r7[2] == 0x01 && r7[3] == 0xAA)
    {
        SPI_cs_select();
        // 先にOCR取得
        SD_send_cmd(CMD58, 0);
        uint8_t ocr[4];
        for (uint8_t i = 0; i < 4; i++)
        {
            ocr[i] = SPI_rx_byte();
        }
        SPI_cs_deselect();
        SPI_tx_byte(0xFF);

        // ACMD41のArgumentにOCRの値を乗せる
        DWORD acmd41_arg = 0x40000000 | (ocr[3] << 16) | (ocr[2] << 8);
        BYTE acmd41_res = 0xFF;
        // ループして応答を待つ
        do
        {
            SPI_cs_select();
            // ACMD41の前にCMD55を送る
            SD_send_cmd(CMD55, 0);
            SPI_cs_deselect();
            SPI_tx_byte(0xFF);

            SPI_cs_select();
            // カード初期化
            acmd41_res = SD_send_cmd(CMD41, acmd41_arg);
            SPI_cs_deselect();
            SPI_tx_byte(0xFF);
        } while (acmd41_res != 0x00 && HAL_GetTick() < retry);

        if (acmd41_res != 0x00)
        {
            // タイムアウト
            return SD_ERROR;
        }

        SPI_cs_select();
        // OCR取得
        SD_send_cmd(CMD58, 0);
        for (uint8_t i = 0; i < 4; i++)
        {
            ocr[i] = SPI_rx_byte();
        }
        SPI_cs_deselect();
        SPI_tx_byte(0xFF);

        if (ocr[0] & 0x40)
        {
            sdhc = 1;
        }
    }
    else
    {
        BYTE acmd41_res = 0xFF;
        do
        {
            SPI_cs_select();
            SD_send_cmd(CMD55, 0);
            acmd41_res = SD_send_cmd(CMD41, 0x00);
            SPI_cs_deselect();
            SPI_tx_byte(0xFF);
        }
        while (acmd41_res != 0x00 && HAL_GetTick() < retry);
    }

    card_initialized = 1;
    return SD_OK;
}

SD_Status SD_disk_read_blocks(BYTE* buff, DWORD sector, UINT count)
{
    if (!count)
    {
        return SD_ERROR;
    }

    if (count == 1)
    {
        if (!sdhc)
        {
            sector *= 512;
        }

        SPI_cs_select();
        // todo: CMD17で読み出す先頭セクタの値がおかしい (ビットずれではない)
        BYTE cmd17_res = SD_send_cmd(CMD17, sector);
        if (cmd17_res != 0x00)
        {
            SPI_cs_deselect();
            return SD_ERROR;
        }

        uint8_t token;
        uint32_t timeout = HAL_GetTick() + 200;
        do
        {
            token = SPI_rx_byte();
            if (token == 0xFE)
            {
                break;
            }
        }
        while (HAL_GetTick() < timeout);

        if (token != 0xFE)
        {
            SPI_cs_deselect();
            return SD_ERROR;
        }

        SPI_rx_buffer(buff, 512);
        SPI_rx_byte();
        SPI_rx_byte();

        SPI_cs_deselect();
        SPI_tx_byte(0xFF);

        return SD_OK;
    }

    return SD_disk_read_multi(buff, sector, count);
}

SD_Status SD_disk_read_multi(BYTE* buff, DWORD sector, UINT count)
{
    if (!count)
    {
        return SD_ERROR;
    }

    if (!sdhc)
    {
        sector *= 512;
    }

    SPI_cs_select();
    BYTE cmd18_res = SD_send_cmd(CMD18, sector);
    if (cmd18_res != 0x00)
    {
        SPI_cs_deselect();
        return SD_ERROR;
    }

    while (count--)
    {
        uint8_t token;
        uint32_t timeout = HAL_GetTick() + 200;

        do
        {
            token = SPI_rx_byte();
            if (token == 0xFE)
            {
                break;
            }
        }
        while (HAL_GetTick() < timeout);

        if (token != 0xFE)
        {
            SPI_cs_deselect();
            return SD_ERROR;
        }

        SPI_rx_buffer(buff, 512);
        SPI_rx_byte();
        SPI_rx_byte();

        buff += 512;
    }

    SD_send_cmd(CMD12, 0);
    SPI_cs_deselect();
    SPI_tx_byte(0xFF);

    return SD_OK;
}

SD_Status SD_disk_write_blocks(const BYTE* buff, DWORD sector, UINT count)
{
    if (!count)
    {
        return SD_ERROR;
    }

    if (count == 1)
    {
        if (!sdhc)
        {
            sector *= 512;
        }

        SPI_cs_select();
        BYTE cmd24_res = SD_send_cmd(CMD24, sector);
        if (cmd24_res != 0x00)
        {
            SPI_cs_deselect();
            return SD_ERROR;
        }

        SPI_tx_byte(0xFE);
        SPI_tx_buffer((uint8_t*)buff, 512);
        SPI_tx_byte(0xFF);
        SPI_tx_byte(0xFF);

        const uint8_t response = SPI_rx_byte();
        if ((response & 0x1F) != 0x05)
        {
            SPI_cs_deselect();
            return SD_ERROR;
        }

        while (SPI_rx_byte() == 0);

        SPI_cs_deselect();
        SPI_tx_byte(0xFF);

        return SD_OK;
    }

    return SD_disk_write_multi(buff, sector, count);
}

SD_Status SD_disk_write_multi(const BYTE* buff, DWORD sector, UINT count)
{
    if (!count)
    {
        return SD_ERROR;
    }

    if (!sdhc)
    {
        sector *= 512;
    }

    SPI_cs_select();
    BYTE cmd25_res = SD_send_cmd(CMD25, sector);
    if (cmd25_res != 0x00)
    {
        SPI_cs_deselect();
        return SD_ERROR;
    }

    while (count--)
    {
        SPI_tx_byte(0xFC);
        SPI_tx_buffer((uint8_t*)buff, 512);
        SPI_tx_byte(0xFF);
        SPI_tx_byte(0xFF);

        const uint8_t response = SPI_rx_byte();
        if ((response & 0x1F) != 0x05)
        {
            SPI_cs_deselect();
            return SD_ERROR;
        }

        while (SPI_rx_byte() == 0);
        buff += 512;
    }

    SPI_tx_byte(0xFD);
    while (SPI_rx_byte() == 0);

    SPI_cs_deselect();
    SPI_tx_byte(0xFF);

    return SD_OK;
}
