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
 * @file    test_ltc_6806.c
 * @author  foxBMS Team
 * @date    2020-07-13 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the ltc.c module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe_plausibility.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockdma.h"
#include "Mockfassert.h"
#include "Mockio.h"
#include "Mockltc_afe_dma.h"
#include "Mockltc_pec.h"
#include "Mockos.h"
#include "Mockpex.h"
#include "Mockspi.h"

#include "ltc_6806_cfg.h"

#include "ltc.h"
#include "spi_cfg-helper.h"

TEST_FILE("ltc_6806.c")

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for LTC communication */
static const spiDAT1_t spi_kLtcDataConfig[BS_NR_OF_STRINGS] = {
    {
        /* struct is implemented in the TI HAL and uses uppercase true and false */
        .CS_HOLD = TRUE,                              /* If true, HW chip select kept active between words */
        .WDEL    = FALSE,                             /* Activation of delay between words */
        .DFSEL   = SPI_FMT_0,                         /* Data word format selection */
        .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, /* CS2 enabled */
    },
    {
        /* struct is implemented in the TI HAL and uses uppercase true and false */
        .CS_HOLD = TRUE,                              /* If true, HW chip select kept active */
        .WDEL    = FALSE,                             /* Activation of delay between words */
        .DFSEL   = SPI_FMT_0,                         /* Data word format selection */
        .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, /* CS2 enabled */
    },
    {
        /* struct is implemented in the TI HAL and uses uppercase true and false */
        .CS_HOLD = TRUE,                              /* If true, HW chip select kept active */
        .WDEL    = FALSE,                             /* Activation of delay between words */
        .DFSEL   = SPI_FMT_0,                         /* Data word format selection */
        .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, /* CS2 enabled */
    },
};

/**
 * SPI interface configuration for LTC communication
 * This is a list of structs because of multistring
 */
SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kLtcDataConfig[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kLtcDataConfig[1u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
    {
        .pConfig  = &spi_kLtcDataConfig[2u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

SPI_INTERFACE_CONFIG_s spi_ltcInterfaceSecondary[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kLtcDataConfig[0u],
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
void testLTC_SetFirstMeasurementCycleFinished(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    static LTC_STATE_s test_ltc_state     = {0};
    test_ltc_state.first_measurement_made = false;
    TEST_LTC_SetFirstMeasurementCycleFinished(&test_ltc_state);
    TEST_ASSERT_EQUAL_UINT8(true, test_ltc_state.first_measurement_made);
}
