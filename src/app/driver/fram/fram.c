/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    fram.c
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  FRAM
 *
 * @brief   Driver for the FRAM module
 *
 *
 */

/*========== Includes =======================================================*/
#include "fram.h"

#include "version_cfg.h"

#include "crc.h"
#include "diag.h"
#include "fassert.h"
#include "fstd_types.h"
#include "io.h"
#include "mcu.h"
#include "spi.h"
#include "utils.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** delay in &micro;s after writing the FRAM */
#define FRAM_DELAY_AFTER_WRITE_ENABLE_US (5u)

/** control commands for the FRAM */
/**@{*/
#define FRAM_WRITE_COMMAND        (0x02u)
#define FRAM_READ_COMMAND         (0x03u)
#define FRAM_WRITE_ENABLE_COMMAND (0x06u)
/**@}*/

#define FRAM_MAX_ADDRESS         (0x03FFFFu) /**< maximal memory FRAM address */
#define FRAM_ADDRESS_UPPER_PART  (0x030000u) /**< upper part of the FRAM memory address */
#define FRAM_ADDRESS_MIDDLE_PART (0x00FF00u) /**< middle part of the FRAM memory address */
#define FRAM_ADDRESS_LOWER_PART  (0x0000FFu) /**< lower part of the FRAM memory address */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void FRAM_Initialize(void) {
    uint32_t address = 0u;

    /* Reset error flag at startup */
    (void)DIAG_Handler(DIAG_ID_FRAM_READ_CRC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    /* find address of all variables in FRAM  by parsing length of data*/
    for (uint8_t i = 0u; i < (uint8_t)FRAM_BLOCK_MAX; i++) {
        (fram_databaseHeader[i]).address = address;
        address += (fram_databaseHeader[i]).datalength + FRAM_CRC_HEADER_SIZE;
    }

    /* ASSERT that size of variables does not exceed FRAM size */
    FAS_ASSERT(!(address > FRAM_MAX_ADDRESS));

    /* Read FRAM version struct - Set CRC error flag if this fails */
    if (FRAM_ACCESS_OK != FRAM_ReadData(FRAM_BLOCK_ID_VERSION)) {
        (void)DIAG_Handler(DIAG_ID_FRAM_READ_CRC_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }
}

extern STD_RETURN_TYPE_e FRAM_ReinitializeAllEntries(void) {
    STD_RETURN_TYPE_e retVal = STD_OK;

    /* Reset FRAM version struct information */
    fram_version.project = FRAM_PROJECT_ID_FOXBMS_BASELINE;
    fram_version.major   = ver_foxbmsVersionInformation.major;
    fram_version.minor   = ver_foxbmsVersionInformation.minor;
    fram_version.patch   = ver_foxbmsVersionInformation.patch;

    for (uint8_t i = 0u; i < (uint8_t)FRAM_BLOCK_MAX; i++) {
        if (FRAM_WriteData((FRAM_BLOCK_ID_e)i) != FRAM_ACCESS_OK) {
            retVal = STD_NOT_OK;
        }
    }
    return retVal;
}

extern FRAM_RETURN_TYPE_e FRAM_WriteData(FRAM_BLOCK_ID_e blockId) {
    FAS_ASSERT(blockId < FRAM_BLOCK_MAX);

    uint16_t read             = 0u;
    uint64_t crc              = 0u;
    FRAM_RETURN_TYPE_e retVal = FRAM_ACCESS_OK;

    /* FRAM must use SW Chip Select configuration*/
    FAS_ASSERT(spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE);

    uint32_t address = (fram_databaseHeader[blockId]).address;
    uint32_t size    = (fram_databaseHeader[blockId]).datalength;
    uint8_t *pWrite  = (uint8_t *)(fram_databaseHeader[blockId].blockptr);

    STD_RETURN_TYPE_e crcRetVal = CRC_CalculateCrc(&crc, pWrite, size);

    if (crcRetVal == STD_OK) {
        STD_RETURN_TYPE_e spiRetVal = SPI_Lock(SPI_GetSpiIndex(spi_framInterface.pNode));

        if (spiRetVal == STD_OK) {
            /* send write enable command */
            IO_PinReset(spi_framInterface.pGioPort, spi_framInterface.csPin);
            uint16_t write = FRAM_WRITE_ENABLE_COMMAND;
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
            IO_PinSet(spi_framInterface.pGioPort, spi_framInterface.csPin);
            MCU_Delay_us(FRAM_DELAY_AFTER_WRITE_ENABLE_US);

            /* send data to write */
            /* set chip select low to start transmission */
            IO_PinReset(spi_framInterface.pGioPort, spi_framInterface.csPin);

            /* send write command */
            write = FRAM_WRITE_COMMAND;
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

            /* send in this order: upper, middle and lower part of address */
            write = (uint16_t)((address & FRAM_ADDRESS_UPPER_PART) >> UTIL_SHIFT_TWO_BYTES);
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
            write = (uint16_t)((address & FRAM_ADDRESS_MIDDLE_PART) >> UTIL_SHIFT_ONE_BYTE);
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
            write = (uint16_t)((address & FRAM_ADDRESS_LOWER_PART) >> UTIL_SHIFT_ZERO_BYTES);
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

            /* send CRC */
            pWrite = (uint8_t *)(&crc);
            for (uint8_t i = 0u; i < FRAM_CRC_HEADER_SIZE; i++) {
                write = (uint16_t)(*pWrite);
                SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
                pWrite++;
            }

            pWrite = (uint8_t *)(fram_databaseHeader[blockId].blockptr);

            /* send data */
            while (size > 0u) {
                write = (uint16_t)(*pWrite);
                SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
                pWrite++;
                size--;
            }

            /* set chip select high to start transmission */
            IO_PinSet(spi_framInterface.pGioPort, spi_framInterface.csPin);

            SPI_Unlock(SPI_GetSpiIndex(spi_framInterface.pNode));
        } else {
            retVal = FRAM_ACCESS_SPI_BUSY;
        }
    } else {
        retVal = FRAM_ACCESS_CRC_BUSY;
    }
    return retVal;
}

extern FRAM_RETURN_TYPE_e FRAM_ReadData(FRAM_BLOCK_ID_e blockId) {
    FAS_ASSERT(blockId < FRAM_BLOCK_MAX);

    uint16_t read             = 0u;
    FRAM_RETURN_TYPE_e retVal = FRAM_ACCESS_OK;

    /* FRAM must use SW Chip Select configuration*/
    FAS_ASSERT(spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE);

    STD_RETURN_TYPE_e spiRetVal = SPI_Lock(SPI_GetSpiIndex(spi_framInterface.pNode));

    if (spiRetVal == STD_OK) {
        uint32_t address = (fram_databaseHeader[blockId]).address;
        uint32_t size    = (fram_databaseHeader[blockId]).datalength;

        /* get data to be read */
        /* set chip select low to start transmission */
        IO_PinReset(spi_framInterface.pGioPort, spi_framInterface.csPin);

        /* send write command */
        uint16_t write = FRAM_READ_COMMAND;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send upper part of address */
        write = (address & 0x3F0000u) >> 16u;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send middle part of address */
        write = (address & 0x00FF00u) >> 8u;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send lower part of address */
        write = address & 0x0000FFu;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* read CRC */
        uint64_t crcRead = 0u;
        uint8_t *pRead   = (uint8_t *)(&crcRead);
        for (uint8_t i = 0u; i < FRAM_CRC_HEADER_SIZE; i++) {
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
            *pRead = (uint8_t)read;
            pRead++;
        }

        pRead = (uint8_t *)(fram_databaseHeader[blockId].blockptr);

        /* read data */
        write = 0;
        while (size > 0u) {
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
            *pRead = read & (uint8_t)UINT8_MAX;
            pRead++;
            size--;
        }

        /* set chip select high to start transmission */
        IO_PinSet(spi_framInterface.pGioPort, spi_framInterface.csPin);

        SPI_Unlock(SPI_GetSpiIndex(spi_framInterface.pNode));

        pRead                       = (uint8_t *)(fram_databaseHeader[blockId].blockptr);
        size                        = (fram_databaseHeader[blockId]).datalength;
        uint64_t crcCalculated      = 0u;
        STD_RETURN_TYPE_e crcRetVal = CRC_CalculateCrc(&crcCalculated, pRead, size);

        if (crcRetVal == STD_OK) {
            if (crcRead != crcCalculated) {
                (void)DIAG_Handler(DIAG_ID_FRAM_READ_CRC_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, blockId);
                retVal = FRAM_ACCESS_CRC_ERROR;
            }
        } else {
            retVal = FRAM_ACCESS_CRC_BUSY;
        }
    } else {
        retVal = FRAM_ACCESS_SPI_BUSY;
    }

    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
