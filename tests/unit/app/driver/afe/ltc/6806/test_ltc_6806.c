/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_ltc_6806.c
 * @author  foxBMS Team
 * @date    2020-07-13 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the LTC LTC6806 driver
 * @details Test functions:
 *          - testLTC_SetFirstMeasurementCycleFinished
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

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("ltc_6806.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/6806/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common/config")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/

/** SPI data configuration struct for LTC communication */
static spiDAT1_t spi_kLtcDataConfig[BS_NR_OF_STRINGS] = {
    {
        /* struct is implemented in the TI HAL and uses uppercase true and false */
        .CS_HOLD = TRUE,                              /* If true, HW chip select kept active between words */
        .WDEL    = FALSE,                             /* Activation of delay between words */
        .DFSEL   = SPI_FMT_0,                         /* Data word format selection */
        .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE, /* CS2 enabled */
    },
};

/**
 * SPI interface configuration for LTC communication
 * This is a list of structs because of multi-string
 */
SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kLtcDataConfig[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/** local definition of plausible cell voltage values for the LTC 6806 */
static const AFE_PLAUSIBILITY_VALUES_s ltc_plausibleCellVoltages6806 = {
    .maximumPlausibleVoltage_mV = 5000,
    .minimumPlausibleVoltage_mV = -5000,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testLTC_SaveVoltages(void) {
    STD_RETURN_TYPE_e cellVoltageMeasurementValid = STD_OK;

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
            AFE_PlausibilityCheckVoltageMeasurementRange_ExpectAndReturn(
                ltc_stateBase.ltcData.cellVoltage->cellVoltage_mV[0u][m][cb], ltc_plausibleCellVoltages6806, STD_OK);
            (void)cellVoltageMeasurementValid;
        }
    }

    DIAG_CheckEvent_ExpectAndReturn(
        cellVoltageMeasurementValid, DIAG_ID_AFE_CELL_VOLTAGE_MEAS_ERROR, DIAG_STRING, 0u, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(ltc_stateBase.ltcData.cellVoltage, STD_OK);
    LTC_SaveVoltages(&ltc_stateBase, 0u);
}

void testLTC_SaveTemperatures(void) {
    STD_RETURN_TYPE_e cellTemperatureMeasurementValid = STD_OK;

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
            AFE_PlausibilityCheckTempMinMax_ExpectAndReturn(
                ltc_stateBase.ltcData.cellTemperature->cellTemperature_ddegC[0u][m][ts], STD_OK);
            (void)cellTemperatureMeasurementValid;
        }
    }

    DIAG_CheckEvent_ExpectAndReturn(
        cellTemperatureMeasurementValid, DIAG_ID_AFE_CELL_TEMPERATURE_MEAS_ERROR, DIAG_STRING, 0u, STD_OK);
    DATA_Write1DataBlock_ExpectAndReturn(ltc_stateBase.ltcData.cellTemperature, STD_OK);
    LTC_SaveTemperatures(&ltc_stateBase, 0u);
}

void testLTC_SaveAllGpioMeasurement(void) {
    DATA_Write1DataBlock_ExpectAndReturn(ltc_stateBase.ltcData.allGpioVoltages, STD_OK);
    LTC_SaveAllGpioMeasurement(&ltc_stateBase);
}

void testLTC_CheckReEntrance(void) {
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

void testLTC_GetStateRequest(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    LTC_GetStateRequest(&ltc_stateBase);
}

void testLTC_GetState(void) {
    LTC_GetState(&ltc_stateBase);
}

void testLTC_IsFirstMeasurementCycleFinished(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    LTC_IsFirstMeasurementCycleFinished(&ltc_stateBase);
}

void testLTC_SetFirstMeasurementCycleFinished(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    static LTC_STATE_s test_ltc_state     = {0};
    test_ltc_state.first_measurement_made = false;
    TEST_LTC_SetFirstMeasurementCycleFinished(&test_ltc_state);
    TEST_ASSERT_EQUAL_UINT8(true, test_ltc_state.first_measurement_made);
}

void testLTC_InitializeMonitoringPin(void) {
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_1);
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_2);
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_3);
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_4);
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_5);
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_6);
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_7);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_1);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_2);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_3);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_4);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_5);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_6);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_7);
    LTC_InitializeMonitoringPin();
}

void testLTC_IdentifyAfes(void) {
    LTC_IdentifyAfes();
}
