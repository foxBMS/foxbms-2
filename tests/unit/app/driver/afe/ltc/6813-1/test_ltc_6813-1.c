/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_ltc_6813-1.c
 * @author  foxBMS Team
 * @date    2020-03-30 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the ltc_6813-1.c module
 * @details TODO
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
#include "Mocktsi.h"

#include "ltc_6813-1_cfg.h"

#include "ltc.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("ltc_6813-1.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/6813-1/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common/config")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/
/* SPI data configuration struct for LTC communication */
static spiDAT1_t spi_kLtcDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /* The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /* No delay will be inserted */
    .DFSEL   = SPI_FMT_0, /* Data word format select */
    .CSNR    = 0x0,       /* Chip select (CS) number, 0x01h for CS[0] */
};

/* SPI interface configuration for LTC communication */
SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kLtcDataConfig,
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
    },
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testTEST_LTC_CheckReEntrance(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    LTC_STATE_s ltc_state = {0};
    uint8_t retVal        = 0;
    retVal                = TEST_LTC_CheckReEntrance(&ltc_state);
    TEST_ASSERT_EQUAL_UINT8(0, retVal);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    ltc_state.triggerentry = 1;
    retVal                 = TEST_LTC_CheckReEntrance(&ltc_state);
    TEST_ASSERT_EQUAL_UINT8(0xFF, retVal);
}

void testLTC_SetFirstMeasurementCycleFinished(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    static LTC_STATE_s test_ltc_state     = {0};
    test_ltc_state.first_measurement_made = false;
    TEST_LTC_SetFirstMeasurementCycleFinished(&test_ltc_state);
    TEST_ASSERT_EQUAL_UINT8(true, test_ltc_state.first_measurement_made);
}

void testLTC_ConvertMuxVoltagesToTemperatures(void) {
    TSI_GetTemperature_ExpectAndReturn(0, 0u);
    int16_t x = 0;
    x         = LTC_ConvertMuxVoltagesToTemperatures(0);
    TEST_ASSERT_EQUAL_INT16(0, x);
    TSI_GetTemperature_ExpectAndReturn(11, 11u);
    x = LTC_ConvertMuxVoltagesToTemperatures(11);
    TEST_ASSERT_EQUAL_INT16(11, x);
}
