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
 * @file    test_n775.c
 * @author  foxBMS Team
 * @date    2020-06-10 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the n775.c module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe.h"
#include "Mockdatabase.h"
#include "Mockfassert.h"
#include "Mocknxp_afe_dma.h"
#include "Mockos.h"
#include "Mockspi.h"

#include "n775.h"
#include "spi_cfg-helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for NXP MC33775A communication, Tx part */
static const spiDAT1_t spi_kNxp775DataConfigTx[BS_NR_OF_STRINGS] = {
    {
        .CS_HOLD = TRUE,                              /*!< The HW chip select signal is deactivated */
        .WDEL    = TRUE,                              /*!< No delay will be inserted */
        .DFSEL   = SPI_FMT_0,                         /*!< Data word format select */
        .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, /* CS2 enabled */
    },
    {
        .CS_HOLD = TRUE,                              /*!< The HW chip select signal is deactivated */
        .WDEL    = TRUE,                              /*!< No delay will be inserted */
        .DFSEL   = SPI_FMT_0,                         /*!< Data word format select */
        .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, /*!< CS2 enabled */
    },
    {
        .CS_HOLD = TRUE,                              /*!< The HW chip select signal is deactivated */
        .WDEL    = TRUE,                              /*!< No delay will be inserted */
        .DFSEL   = SPI_FMT_0,                         /*!< Data word format select */
        .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, /* CS2 enabled */
    },
};

/** SPI interface configuration for N775 communication Tx part */
SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceTx[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kNxp775DataConfigTx[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kNxp775DataConfigTx[1u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kNxp775DataConfigTx[2u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testDummy(void) {
}
