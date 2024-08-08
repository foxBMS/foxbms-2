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
 * @file    test_can_cbs_tx_pack-state-estimation.c
 * @author  foxBMS Team
 * @date    2021-07-27 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_pack-state-estimation.c")

TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[7u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u};

float_t testSignalData[4u] = {0.0f, 1.0f, 2.0f, 3.0f};

static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumSoc = {7u, 10u, 0.1f, 0.0f, 0.0f, 102.3f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumSoc = {13u, 10u, 0.1f, 0.0f, 0.0f, 102.3f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMinimumSoe = {19u, 10u, 0.1f, 0.0f, 0.0f, 102.3f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalMaximumSoe = {25u, 10u, 0.1f, 0.0f, 0.0f, 102.3f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalSoh        = {47u, 8u, 0.5f, 0.0f, 0.0f, 127.5f};
static const CAN_SIGNAL_TYPE_s cantx_testSignalEnergy     = {55u, 16u, 100.0f, 0.0f, 0.0f, 6553500.0f};

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest       = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues           = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                          = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                          = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                          = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState           = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_MONITORING_s can_tableInsulation = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags            = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags            = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags            = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_AEROSOL_SENSOR_s can_tableAerosolSensor = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &imd_canDataQueue,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor   = &can_tableCurrentSensor,
    .pTableOpenWire        = &can_tableOpenWire,
    .pTableStateRequest    = &can_tableStateRequest,
    .pTablePackValues      = &can_tablePackValues,
    .pTableSof             = &can_tableSof,
    .pTableSoc             = &can_tableSoc,
    .pTableSoe             = &can_tableSoe,
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMsl             = &can_tableMslFlags,
    .pTableRsl             = &can_tableRslFlags,
    .pTableMol             = &can_tableMolFlags,
    .pTableAerosolSensor   = &can_tableAerosolSensor,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    can_tableSoc.maximumSoc_perc[0u] = 75.0f;
    can_tableSoc.maximumSoc_perc[1u] = 50.5f;

    can_tableSoc.minimumSoc_perc[0u] = 33.3f;
    can_tableSoc.minimumSoc_perc[1u] = 44.4f;

    can_tableSoe.maximumSoe_perc[0u] = 50.0f;
    can_tableSoe.maximumSoe_perc[1u] = 25.0f;

    can_tableSoe.minimumSoe_perc[0u] = 66.6f;
    can_tableSoe.minimumSoe_perc[1u] = 77.7f;

    can_tableSoe.minimumSoe_Wh[0u] = 55.5f;
    can_tableSoe.minimumSoe_Wh[1u] = 75.0f;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_CalculateMaximumPackSoc
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/3: Number of connected strings is 0
 *            - RT2/3: Number of connected strings is 1
 *            - RT3/3: Number of connected strings is 2
 */
void testCANTX_CalculateMaximumPackSoc(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumPackSoc(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testSignalData[0u], cantx_testSignalMaximumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/3: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumPackSoc(&can_kShim);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/3: Test implementation */
    float_t maximumPackSocOneStringClosed = can_tableSoc.maximumSoc_perc[0u] / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&maximumPackSocOneStringClosed, cantx_testSignalMaximumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/3: Call function under test */
    testResult = TEST_CANTX_CalculateMaximumPackSoc(&can_kShim);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT3/3: Test implementation */
    float_t maximumPackSocTwoStringsClosed = (2 * can_tableSoc.maximumSoc_perc[0u]) / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&maximumPackSocTwoStringsClosed, cantx_testSignalMaximumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT3/3: Call function under test */
    testResult = TEST_CANTX_CalculateMaximumPackSoc(&can_kShim);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_GetMaximumStringSoc
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetMaximumStringSoc(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetMaximumStringSoc(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    float_t testResult = TEST_CANTX_GetMaximumStringSoc(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoc.maximumSoc_perc[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetMaximumStringSoc(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoc.maximumSoc_perc[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetMaximumStringSoc(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoc.maximumSoc_perc[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetMaximumStringSoc(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(0.0f, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMinimumPackSoc
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/3: Number of connected strings is 0
 *            - RT2/3: Number of connected strings is 1
 *            - RT3/3: Number of connected strings is 2
 */
void testCANTX_CalculateMinimumPackSoc(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMinimumPackSoc(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testSignalData[0u], cantx_testSignalMinimumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/3: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMinimumPackSoc(&can_kShim);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/3: Test implementation */
    float_t minimumPackSocOneStringClosed = can_tableSoc.minimumSoc_perc[0u] / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&minimumPackSocOneStringClosed, cantx_testSignalMinimumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/3: Call function under test */
    testResult = TEST_CANTX_CalculateMinimumPackSoc(&can_kShim);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT3/3: Test implementation */
    float_t minimumPackSocTwoStringsClosed = (2 * can_tableSoc.minimumSoc_perc[0u]) / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&minimumPackSocTwoStringsClosed, cantx_testSignalMinimumSoc);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT3/3: Call function under test */
    testResult = TEST_CANTX_CalculateMinimumPackSoc(&can_kShim);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_GetMinimumStringSoc
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetMinimumStringSoc(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetMinimumStringSoc(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    float_t testResult = TEST_CANTX_GetMinimumStringSoc(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoc.minimumSoc_perc[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetMinimumStringSoc(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoc.minimumSoc_perc[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetMinimumStringSoc(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoc.minimumSoc_perc[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetMinimumStringSoc(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(100.0f, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMaximumPackSoe
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/3: Number of connected strings is 0
 *            - RT2/3: Number of connected strings is 1
 *            - RT3/3: Number of connected strings is 2
 */
void testCANTX_CalculateMaximumPackSoe(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMaximumPackSoe(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testSignalData[0u], cantx_testSignalMaximumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/3: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMaximumPackSoe(&can_kShim);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/3: Test implementation */
    float_t maximumPackSoeOneStringClosed = can_tableSoe.maximumSoe_perc[0u] / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&maximumPackSoeOneStringClosed, cantx_testSignalMaximumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/3: Call function under test */
    testResult = TEST_CANTX_CalculateMaximumPackSoe(&can_kShim);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT3/3: Test implementation */
    float_t maximumPackSoeTwoStringsClosed = (2 * can_tableSoe.maximumSoe_perc[0u]) / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&maximumPackSoeTwoStringsClosed, cantx_testSignalMaximumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT3/3: Call function under test */
    testResult = TEST_CANTX_CalculateMaximumPackSoe(&can_kShim);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_GetMaximumStringSoe
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetMaximumStringSoe(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetMaximumStringSoe(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    float_t testResult = TEST_CANTX_GetMaximumStringSoe(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.maximumSoe_perc[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetMaximumStringSoe(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.maximumSoe_perc[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetMaximumStringSoe(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.maximumSoe_perc[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetMaximumStringSoe(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(0.0f, testResult);
}

/**
 * @brief   Testing CANTX_CalculateMinimumPackSoe
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/3: Number of connected strings is 0
 *            - RT2/3: Number of connected strings is 1
 *            - RT3/3: Number of connected strings is 2
 */
void testCANTX_CalculateMinimumPackSoe(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculateMinimumPackSoe(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testSignalData[0u], cantx_testSignalMinimumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/3: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculateMinimumPackSoe(&can_kShim);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT2/3: Test implementation */
    float_t minimumPackSoeOneStringClosed = can_tableSoe.minimumSoe_perc[0u] / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&minimumPackSoeOneStringClosed, cantx_testSignalMinimumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/3: Call function under test */
    testResult = TEST_CANTX_CalculateMinimumPackSoe(&can_kShim);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);

    /* ======= RT3/3: Test implementation */
    float_t minimumPackSoeTwoStringsClosed = (2 * can_tableSoe.minimumSoe_perc[0u]) / BS_NR_OF_STRINGS;
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_IsStringClosed_IgnoreAndReturn(true);
    CAN_TxPrepareSignalData_Expect(&minimumPackSoeTwoStringsClosed, cantx_testSignalMinimumSoe);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT3/3: Call function under test */
    testResult = TEST_CANTX_CalculateMinimumPackSoe(&can_kShim);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(1u, testResult);
}

/**
 * @brief   Testing CANTX_GetMinimumStringSoe
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/4: Only first string is closed
 *            - RT2/4: Only second string is closed
 *            - RT3/4: Both Strings are closed
 *            - RT4/4: No String is closed
 */
void testCANTX_GetMinimumStringSoe(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetMinimumStringSoe(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT1/4: Call function under test */
    float_t testResult = TEST_CANTX_GetMinimumStringSoe(&can_kShim);
    /* ======= RT1/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.minimumSoe_perc[0u], testResult);

    /* ======= RT2/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT2/4: Call function under test */
    testResult = TEST_CANTX_GetMinimumStringSoe(&can_kShim);
    /* ======= RT2/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.minimumSoe_perc[1u], testResult);

    /* ======= RT3/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringClosed_ExpectAndReturn(1u, true);
    /* ======= RT3/4: Call function under test */
    testResult = TEST_CANTX_GetMinimumStringSoe(&can_kShim);
    /* ======= RT3/4: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.minimumSoe_perc[0u], testResult);

    /* ======= RT4/4: Test implementation */
    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    /* ======= RT4/4: Call function under test */
    testResult = TEST_CANTX_GetMinimumStringSoe(&can_kShim);
    /* ======= RT4/4: Test output verification */
    TEST_ASSERT_EQUAL(100.0f, testResult);
}

/**
 * @brief   Testing CANTX_CalculatePackSoh
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Data prepared as expected
 */
void testCANTX_CalculatePackSoh(void) {
    /* ======= Routine tests =============================================== */
    float_t packSoh = 100.0f;
    /* ======= RT1/1: Test implementation */
    CAN_TxPrepareSignalData_Expect(&packSoh, cantx_testSignalSoh);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/1: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculatePackSoh();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testSignalData[1u], testResult);
}

/**
 * @brief   Testing CANTX_CalculatePackEnergy
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/3: Number of connected strings is 0
 *            - RT2/3: Number of connected strings is 1
 *            - RT3/3: Number of connected strings is 2
 */
void testCANTX_CalculatePackEnergy(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_CalculatePackEnergy(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(0u);
    CAN_TxPrepareSignalData_Expect(&testSignalData[0u], cantx_testSignalEnergy);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT1/3: Call function under test */
    uint64_t testResult = TEST_CANTX_CalculatePackEnergy(&can_kShim);
    /* ======= RT1/3: Test output verification */
    TEST_ASSERT_EQUAL(testSignalData[1u], testResult);

    /* ======= RT2/3: Test implementation */
    float_t packEnergyOneStringConnected = can_tableSoe.minimumSoe_Wh[0u];
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(1u);
    CAN_TxPrepareSignalData_Expect(&packEnergyOneStringConnected, cantx_testSignalEnergy);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT2/3: Call function under test */
    testResult = TEST_CANTX_CalculatePackEnergy(&can_kShim);
    /* ======= RT2/3: Test output verification */
    TEST_ASSERT_EQUAL(testSignalData[1u], testResult);

    /* ======= RT3/3: Test implementation */
    float_t packEnergyTwoStringsConnected = 2 * can_tableSoe.minimumSoe_Wh[0u];
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    BMS_GetNumberOfConnectedStrings_ExpectAndReturn(2u);
    CAN_TxPrepareSignalData_Expect(&packEnergyTwoStringsConnected, cantx_testSignalEnergy);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    /* ======= RT3/3: Call function under test */
    testResult = TEST_CANTX_CalculatePackEnergy(&can_kShim);
    /* ======= RT3/3: Test output verification */
    TEST_ASSERT_EQUAL(testSignalData[1u], testResult);
}

/**
 * @brief   Testing CANTX_GetStringEnergy
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: String 0 energy > string 1 energy
 *            - RT2/2: String 0 energy < string 1 energy
 */
void testCANTX_GetStringEnergy(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_GetStringEnergy(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Call function under test */
    float_t testResult = TEST_CANTX_GetStringEnergy(&can_kShim);
    /* ======= RT1/2: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.minimumSoe_Wh[0u], testResult);

    /* ======= RT2/2: Call function under test */
    can_tableSoe.minimumSoe_Wh[0u] = 75.0f;
    can_tableSoe.minimumSoe_Wh[1u] = 55.5f;
    testResult                     = TEST_CANTX_GetStringEnergy(&can_kShim);
    /* ======= RT2/2: Test output verification */
    TEST_ASSERT_EQUAL(can_tableSoe.minimumSoe_Wh[1u], testResult);
    can_tableSoe.minimumSoe_Wh[0u] = 55.5f;
    can_tableSoe.minimumSoe_Wh[1u] = 75.0f;
}

/**
 * @brief   Testing CANTX_BuildPackStateEstimationMessage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for kpkCanShim -> assert
 *            - AT2/2: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Message data is set as expected
 */
void testCANTX_BuildPackStateEstimationMessage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildPackStateEstimationMessage(NULL_PTR, &testMessageData[0u]));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildPackStateEstimationMessage(&can_kShim, NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t testResult = 0u;
    /* ======= RT1/1: Test implementation */
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(0u);
    BMS_IsStringClosed_IgnoreAndReturn(false);
    CAN_TxPrepareSignalData_Expect(NULL_PTR, cantx_testSignalMinimumSoc);
    CAN_TxPrepareSignalData_IgnoreArg_pSignal();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(NULL_PTR, cantx_testSignalMaximumSoc);
    CAN_TxPrepareSignalData_IgnoreArg_pSignal();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 13u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(NULL_PTR, cantx_testSignalMinimumSoe);
    CAN_TxPrepareSignalData_IgnoreArg_pSignal();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 19u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(NULL_PTR, cantx_testSignalMaximumSoe);
    CAN_TxPrepareSignalData_IgnoreArg_pSignal();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 25u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(NULL_PTR, cantx_testSignalSoh);
    CAN_TxPrepareSignalData_IgnoreArg_pSignal();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 47u, 8u, 100.0f, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxPrepareSignalData_Expect(NULL_PTR, cantx_testSignalEnergy);
    CAN_TxPrepareSignalData_IgnoreArg_pSignal();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 55u, 16u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_BuildPackStateEstimationMessage(&can_kShim, &testResult);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testResult, testMessageData[6u]);
}

/**
 * @brief   Testing CANTX_PackStateEstimation
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/7: Invalid message id -> assert
 *            - AT2/7: Invalid message id type -> assert
 *            - AT3/7: Invalid message dlc size -> assert
 *            - AT4/7: Invalid message endianness -> assert
 *            - AT5/7: NULL_PTR for pCanData -> assert
 *            - AT6/7: NO NULL_PTR for pMuxId -> assert
 *            - AT7/7: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Message data is set as expected
 */
void testCANTX_PackStateEstimation(void) {
    /* ======= Assertion tests ============================================= */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_PACK_STATE_ESTIMATION_ID,
        .idType     = CANTX_PACK_STATE_ESTIMATION_ID_TYPE,
        .dlc        = CANTX_PACK_STATE_ESTIMATION_DLC,
        .endianness = CANTX_PACK_STATE_ESTIMATION_ENDIANNESS,
    };
    uint8_t muxId                = 0u;
    uint8_t canData[CAN_MAX_DLC] = {0u};
    /* ======= AT1/7 ======= */
    testMessage.id = CAN_MAX_11BIT_ID;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackStateEstimation(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.id = CANTX_PACK_STATE_ESTIMATION_ID;
    /* ======= AT2/7 ======= */
    testMessage.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackStateEstimation(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    /* ======= AT3/7 ======= */
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackStateEstimation(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.dlc = CANTX_PACK_STATE_ESTIMATION_DLC;
    /* ======= AT4/7 ======= */
    testMessage.endianness = CAN_LITTLE_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackStateEstimation(testMessage, canData, NULL_PTR, &can_kShim));
    testMessage.endianness = CAN_BIG_ENDIAN;
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackStateEstimation(testMessage, NULL_PTR, NULL_PTR, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackStateEstimation(testMessage, canData, &muxId, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_PackStateEstimation(testMessage, canData, NULL_PTR, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(0u);
    BMS_IsStringClosed_IgnoreAndReturn(false);
    CAN_TxPrepareSignalData_Ignore();
    DATA_Read2DataBlocks_ExpectAndReturn(can_kShim.pTableSoc, can_kShim.pTableSoe, STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 13u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 19u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 25u, 10u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, 100.0f, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 16u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], canData, testMessage.endianness);
    /* ======= RT1/1: Call function under test */

    uint32_t testResult = CANTX_PackStateEstimation(testMessage, canData, NULL_PTR, &can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
}
