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
 * @file    test_nxp_mc33775a_afe.c
 * @author  foxBMS Team
 * @date    2020-06-10 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of nxp_mc33775a_afe.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "Mockafe_dma.h"
#include "Mockdma.h"
#include "Mocknxp_mc3377x.h"
#include "Mockos.h"
#include "Mockpex.h"

#include "nxp_afe.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("nxp_mc33775a_afe.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/common/mc3377x/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/nxp/mc33775a/vendor")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/spi")

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_CELL_VOLTAGE_s n77x_cellVoltage           = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s n77x_cellTemperature   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_MIN_MAX_s n77x_minMax                     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_BALANCING_CONTROL_s n77x_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s n77x_allGpioVoltage   = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s n77x_balancingFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_SLAVE_CONTROL_s n77x_slaveControl = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
static DATA_BLOCK_OPEN_WIRE_s n77x_openWire         = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static N77X_SUPPLY_CURRENT_s n77x_supplyCurrent     = {0};
static N77X_ERROR_TABLE_s n77x_errorTable           = {0};

N77X_STATE_s n77x_stateBase = {
    .firstMeasurementMade       = false,
    .currentString              = 0u,
    .pSpiTxSequenceStart        = NULL_PTR,
    .pSpiTxSequence             = NULL_PTR,
    .pSpiRxSequenceStart        = NULL_PTR,
    .pSpiRxSequence             = NULL_PTR,
    .currentMux                 = {0u},
    .pMuxSequenceStart          = NULL_PTR,
    .pMuxSequence               = NULL_PTR,
    .n77xData.cellVoltage       = &n77x_cellVoltage,
    .n77xData.cellTemperature   = &n77x_cellTemperature,
    .n77xData.allGpioVoltage    = &n77x_allGpioVoltage,
    .n77xData.minMax            = &n77x_minMax,
    .n77xData.balancingFeedback = &n77x_balancingFeedback,
    .n77xData.balancingControl  = &n77x_balancingControl,
    .n77xData.slaveControl      = &n77x_slaveControl,
    .n77xData.openWire          = &n77x_openWire,
    .n77xData.supplyCurrent     = &n77x_supplyCurrent,
    .n77xData.errorTable        = &n77x_errorTable,
};

uint8_t testString = 0;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testNXP_Measure(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    N77x_Measure_Expect(&n77x_stateBase);
    TEST_ASSERT_EQUAL(STD_OK, NXP_Measure());
}

void testNXP_Initialize(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    PEX_SetPinDirectionOutput_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPin_Expect(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    uint32_t testTime = 5;
    OS_GetTickCount_ExpectAndReturn(testTime);
    OS_DelayTaskUntil_Expect(&testTime, 10u);

    TEST_ASSERT_EQUAL(STD_OK, NXP_Initialize());
}

void testNXP_StartMeasurement(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    TEST_ASSERT_EQUAL(STD_NOT_OK, NXP_StartMeasurement());
}

void testNXP_IsFirstMeasurementCycleFinished(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    N77x_IsFirstMeasurementCycleFinished_ExpectAndReturn(&n77x_stateBase, false);
    TEST_ASSERT_FALSE(NXP_IsFirstMeasurementCycleFinished());
}

void testNXP_RequestTemperatureRead(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    TEST_ASSERT_EQUAL(STD_NOT_OK, NXP_RequestTemperatureRead(testString));
}

void testNXP_RequestBalancingFeedbackRead(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    TEST_ASSERT_EQUAL(STD_NOT_OK, NXP_RequestBalancingFeedbackRead(testString));
}

void testNXP_RequestEepromRead(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    TEST_ASSERT_EQUAL(STD_NOT_OK, NXP_RequestEepromRead(testString));
}

void testNXP_RequestEeprimWrite(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    TEST_ASSERT_EQUAL(STD_NOT_OK, NXP_RequestEepromWrite(testString));
}

void testNXP_RequestOpenWireCheck(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 ======= */
    TEST_ASSERT_EQUAL(STD_NOT_OK, NXP_RequestOpenWireCheck(testString));
}
