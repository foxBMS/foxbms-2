/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    fram.c
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  FRAM
 *
 * @brief   Driver for the FRAM module
 *
 *
 */

/*========== Includes =======================================================*/
#include "fram.h"

#include "io.h"
#include "mcu.h"
#include "os.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/** delay in &micro;s after writing the FRAM */
#define FRAM_DELAY_AFTER_WRITE_ENABLE_US (5U)

/** control commands for the FRAM */
/**@{*/
#define FRAM_WRITECOMMAND       (0x02u)
#define FRAM_READCOMMAND        (0x03u)
#define FRAM_WRITEENABLECOMMAND (0x06u)
/**@}*/

/** maximal memory address of the FRAM */
#define FRAM_MAX_ADDRESS (0x3FFFFu)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void FRAM_Initialize(void) {
    uint32_t address = 0;

    /* find address of all variables in FRAM  by parsing length of data*/
    for (uint16_t i = 0u; i < FRAM_BLOCK_MAX; i++) {
        (&fram_base_header[0u] + i)->address = address;
        address += (&fram_base_header[0u] + i)->datalength;
    }

    /* ASSERT that size of variables does not exceed FRAM size */
    FAS_ASSERT(!(address > FRAM_MAX_ADDRESS));
}

extern STD_RETURN_TYPE_e FRAM_Write(FRAM_BLOCK_ID_e blockId) {
    uint8_t *wrt_ptr         = NULL_PTR;
    uint32_t address         = 0;
    uint16_t write           = 0;
    uint16_t read            = 0;
    uint16_t size            = 0;
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    /* FRAM must use SW Chip Select configuration*/
    FAS_ASSERT(spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE);

    retVal = SPI_Lock(SPI_GetSpiIndex(spi_framInterface.pNode));

    if (retVal == STD_OK) {
        address = (&fram_base_header[0] + blockId)->address;

        wrt_ptr = (uint8_t *)((&fram_base_header[0] + blockId)->blockptr);
        size    = (&fram_base_header[0] + blockId)->datalength;

        /* send write enable command */
        IO_PinReset(spi_framInterface.pGioPort, spi_framInterface.csPin);
        write = FRAM_WRITEENABLECOMMAND;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
        IO_PinSet(spi_framInterface.pGioPort, spi_framInterface.csPin);
        MCU_delay_us(FRAM_DELAY_AFTER_WRITE_ENABLE_US);

        /* send data to write */
        /* set chip select low to start transmission */
        IO_PinReset(spi_framInterface.pGioPort, spi_framInterface.csPin);

        /* send write command */
        write = FRAM_WRITECOMMAND;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send upper part of address */
        write = (address & 0x3F0000u) >> 16u;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send middle part of address */
        write = (address & 0xFF00u) >> 8u;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send lower part of address */
        write = address & 0xFFu;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        while (size > 0u) {
            write = (uint16_t)(*wrt_ptr);
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
            wrt_ptr++;
            size--;
        }

        /* set chip select high to start transmission */
        IO_PinSet(spi_framInterface.pGioPort, spi_framInterface.csPin);

        SPI_Unlock(SPI_GetSpiIndex(spi_framInterface.pNode));
    }

    return retVal;
}

extern STD_RETURN_TYPE_e FRAM_Read(FRAM_BLOCK_ID_e blockId) {
    uint8_t *rd_ptr          = NULL_PTR;
    uint32_t address         = 0;
    uint16_t write           = 0;
    uint16_t read            = 0;
    uint16_t size            = 0;
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    /* FRAM must use SW Chip Select configuration*/
    FAS_ASSERT(spi_framInterface.csType == SPI_CHIP_SELECT_SOFTWARE);

    retVal = SPI_Lock(SPI_GetSpiIndex(spi_framInterface.pNode));

    if (retVal == STD_OK) {
        address = (&fram_base_header[0] + blockId)->address;

        rd_ptr = (uint8_t *)((&fram_base_header[0] + blockId)->blockptr);
        size   = (&fram_base_header[0] + blockId)->datalength;

        /* get data to be read */
        /* set chip select low to start transmission */
        IO_PinReset(spi_framInterface.pGioPort, spi_framInterface.csPin);

        /* send write command */
        write = FRAM_READCOMMAND;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send upper part of address */
        write = (address & 0x3F0000u) >> 16u;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send middle part of address */
        write = (address & 0xFF00u) >> 8u;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        /* send lower part of address */
        write = address & 0xFFu;
        SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);

        write = 0;
        while (size > 0u) {
            SPI_FramTransmitReceiveData(&spi_framInterface, &write, &read, 1u);
            *rd_ptr = read;
            rd_ptr++;
            size--;
        }

        /* set chip select high to start transmission */
        IO_PinSet(spi_framInterface.pGioPort, spi_framInterface.csPin);

        SPI_Unlock(SPI_GetSpiIndex(spi_framInterface.pNode));
    }

    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
