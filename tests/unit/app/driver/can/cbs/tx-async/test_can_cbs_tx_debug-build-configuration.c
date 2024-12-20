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
 * @file    test_can_cbs_tx_debug-build-configuration.c
 * @author  foxBMS Team
 * @date    2023-02-09 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"

#include "app_build_cfg.h"
#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"
#include "can_cfg.h"

#include "can_cbs_tx_debug-build-configuration.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_debug-build-configuration.c")

TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")
TEST_INCLUDE_PATH("../../src/version")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[9u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u};

float_t testSignalData[4u] = {0u, 1u, 2u, 3u};

static const CAN_SIGNAL_TYPE_s signalOpenWirePeriod = {0u, 0u, 1000.0f, 0.0f, 0.0f, 4095000.0f};

static const CAN_SIGNAL_TYPE_s signalBatteryCellTemperature = {0u, 0u, 1.0f, 0.0f, -131072.0f, 131071.0f};

#define TEST_PLACEHOLDER_CONVERTED_BOOL (0u)

/* dummy data array filled with zero */
uint8_t testCanDataZeroArray[CAN_MAX_DLC] = {0};

/* Dummy for version file implementation */
const VER_BUILD_CONFIGURATION_s ver_foxbmsBuildConfiguration = {
    .socAlgorithm            = SOC_ALGORITHM_NONE,
    .soeAlgorithm            = SOE_ALGORITHM_COUNTING,
    .sofAlgorithm            = SOF_ALGORITHM_TRAPEZOID,
    .sohAlgorithm            = SOH_ALGORITHM_DEBUG,
    .imdName                 = IMD_BENDER_ISO_165C,
    .balancingStrategy       = BALANCING_STRATEGY_NONE,
    .rtos                    = FREERTOS,
    .afeName                 = LTC_LTC6813_1,
    .temperatureSensorName   = EPC00,
    .temperatureSensorMethod = POLYNOMIAL,
};

OS_QUEUE ftsk_imdCanDataQueue = NULL_PTR;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing CANTX_DebugBuildConfiguration
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/5: Application messages aren't queued successfully
 *            - RT2/5: Slave messages aren't queued successfully
 *            - RT3/5: Battery cell messages aren't queued successfully
 *            - RT4/5: Battery system messages aren't queued successfully
 *            - RT5/5: All messages are queued successfully
 */
void testCANTX_DebugBuildConfiguration(void) {
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC] = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    /* ======= RT1/5: Test implementation */
    /* Messages sent by CANTX_SendApplicationConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT1/5: Call function under test */
    STD_RETURN_TYPE_e testResult = CANTX_DebugBuildConfiguration();
    /* ======= RT1/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT2/5: Test implementation */
    /* Messages sent by CANTX_SendApplicationConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendSlaveConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT2/5: Call function under test */
    testResult = CANTX_DebugBuildConfiguration();
    /* ======= RT2/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT3/5: Test implementation */
    /* Messages sent by CANTX_SendApplicationConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendSlaveConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    /* Messages sent by CANTX_SendBatteryCellConfiguration */
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT3/5: Call function under test */
    testResult = CANTX_DebugBuildConfiguration();
    /* ======= RT3/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT4/5: Test implementation */
    /* Messages sent by CANTX_SendApplicationConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendSlaveConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    /* Messages sent by CANTX_SendBatteryCellConfiguration */
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendBatterySystemConfiguration */
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT4/5: Call function under test */
    testResult = CANTX_DebugBuildConfiguration();
    /* ======= RT4/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT5/5: Test implementation */
    /* Messages sent by CANTX_SendApplicationConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendSlaveConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendBatteryCellConfiguration */
    CAN_TxSetMessageDataWithSignalData_Ignore();
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* Messages sent by CANTX_SendBatterySystemConfiguration */
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[0u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT5/5: Call function under test */
    testResult = CANTX_DebugBuildConfiguration();
    /* ======= RT5/5: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_SendApplicationConfiguration
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data and sends message
 */
void testCANTX_SendApplicationConfiguration(void) {
    /* TODO when application message is implemented*/
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC] = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x10u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 4u, ver_foxbmsBuildConfiguration.socAlgorithm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 11u, 4u, ver_foxbmsBuildConfiguration.soeAlgorithm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 23u, 4u, ver_foxbmsBuildConfiguration.sofAlgorithm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[4u], 19u, 4u, ver_foxbmsBuildConfiguration.sohAlgorithm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[5u], 31u, 4u, ver_foxbmsBuildConfiguration.balancingStrategy, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[6u], 27u, 4u, ver_foxbmsBuildConfiguration.imdName, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[7u], 39u, 4u, ver_foxbmsBuildConfiguration.rtos, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[8u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT1/1: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_SendApplicationConfiguration();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_SendSlaveConfiguration
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data and sends message
 */
void testCANTX_SendSlaveConfiguration(void) {
    /* TODO when slave message is implemented*/
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC] = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x00u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 8u, ver_foxbmsBuildConfiguration.afeName, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 23u, 8u, ver_foxbmsBuildConfiguration.temperatureSensorName, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 31u, 8u, ver_foxbmsBuildConfiguration.temperatureSensorMethod, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[4u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT1/1: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_SendSlaveConfiguration();
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_SendBatteryCellConfiguration
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT01/10: CANTX_MUX_BATTERY_CELL multiplexer message isn't queued successfully
 *            - RT02/10: CANTX_MUX_BATTERY_CELL_MAX_CHARGE_CURRENT multiplexer message isn't queued successfully
 *            - RT03/10: CANTX_MUX_BATTERY_CELL_MAX_CHARGE_TEMPERATURE multiplexer message isn't queued successfully
 *            - RT04/10: CANTX_MUX_BATTERY_CELL_MAX_DISCHARGE_CURRENT multiplexer message isn't queued successfully
 *            - RT05/10: CANTX_MUX_BATTERY_CELL_MAX_DISCHARGE_TEMPERATURE multiplexer message isn't queued successfully
 *            - RT06/10: CANTX_MUX_BATTERY_CELL_MAX_VOLTAGE multiplexer message isn't queued successfully
 *            - RT07/10: CANTX_MUX_BATTERY_CELL_MIN_CHARGE_TEMPERATURE multiplexer message isn't queued successfully
 *            - RT08/10: CANTX_MUX_BATTERY_CELL_MIN_DISCHARGE_TEMPERATURE multiplexer message isn't queued successfully
 *            - RT09/10: CANTX_MUX_BATTERY_CELL_MIN_VOLTAGE multiplexer message isn't queued successfully
 *            - RT10/10: All messages are queued successfully
 */
void testCANTX_SendBatteryCellConfiguration(void) {
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC]  = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    float_t temperatureMaxChargeSignals[3u] = {
        (float_t)BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC};
    float_t temperatureMaxDischargeSignals[3u] = {
        (float_t)BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC};
    float_t temperatureMinChargeSignals[3u] = {
        (float_t)BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC};
    float_t temperatureMinDischargeSignals[3u] = {
        (float_t)BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC};
    /* ======= RT01/10: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT01/10: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT01/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT02/10: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT02/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT02/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT03/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT03/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT03/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT04/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT04/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT04/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT05/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x24u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT05/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT05/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT06/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x24u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x25u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 18u, BC_VOLTAGE_MAX_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 29u, 18u, BC_VOLTAGE_MAX_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 18u, BC_VOLTAGE_MAX_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT06/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT06/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT07/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x24u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x25u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 18u, BC_VOLTAGE_MAX_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 29u, 18u, BC_VOLTAGE_MAX_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 18u, BC_VOLTAGE_MAX_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x26u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMinChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMinChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMinChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT07/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT07/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT08/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x24u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x25u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 18u, BC_VOLTAGE_MAX_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 29u, 18u, BC_VOLTAGE_MAX_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 18u, BC_VOLTAGE_MAX_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x26u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMinChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMinChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMinChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x27u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMinDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMinDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMinDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT08/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT08/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT09/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x24u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x25u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 18u, BC_VOLTAGE_MAX_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 29u, 18u, BC_VOLTAGE_MAX_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 18u, BC_VOLTAGE_MAX_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x26u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMinChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMinChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMinChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x27u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMinDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMinDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMinDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x28u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 18u, BC_VOLTAGE_MIN_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 29u, 18u, BC_VOLTAGE_MIN_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 18u, BC_VOLTAGE_MIN_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT09/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT09/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT10/10: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x24u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMaxDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMaxDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMaxDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x25u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 18u, BC_VOLTAGE_MAX_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 29u, 18u, BC_VOLTAGE_MAX_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 18u, BC_VOLTAGE_MAX_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x26u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMinChargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMinChargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMinChargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x27u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 18u, temperatureMinDischargeSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 29u, 18u, temperatureMinDischargeSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 43u, 18u, temperatureMinDischargeSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x28u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 18u, BC_VOLTAGE_MIN_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 29u, 18u, BC_VOLTAGE_MIN_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 43u, 18u, BC_VOLTAGE_MIN_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT10/10: Call function under test */
    testResult = TEST_CANTX_SendBatteryCellConfiguration();
    /* ======= RT10/10: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_SetDebugBuildConfigurationMux
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/5: NULL_PTR for pMessageData -> assert
 *            - AT2/5: invalid value for muxValue -> assert
 *            - AT3/5: invalid value for muxValue -> assert
 *            - AT4/5: invalid value for muxValue -> assert
 *            - AT5/5: invalid value for muxValue -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetDebugBuildConfigurationMux(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/5 ======= */
    uint64_t muxValue    = 0u;
    uint64_t messageData = 0u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetDebugBuildConfigurationMux(NULL_PTR, muxValue));

    /* ======= AT2/5 ======= */
    muxValue = 0x1Fu;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetDebugBuildConfigurationMux(&messageData, muxValue));

    /* ======= AT3/5 ======= */
    muxValue = 0x29u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetDebugBuildConfigurationMux(&messageData, muxValue));

    /* ======= AT4/5 ======= */
    muxValue = 0x2Fu;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetDebugBuildConfigurationMux(&messageData, muxValue));

    /* ======= AT5/5 ======= */
    muxValue = 0x38u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetDebugBuildConfigurationMux(&messageData, muxValue));

    /* ======= Routine tests =============================================== */
    muxValue = 0x10u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x10u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetDebugBuildConfigurationMux(&messageData, muxValue);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[1u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x20u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 16u, BC_VOLTAGE_NOMINAL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 31u, 16u, BC_VOLTAGE_DEEP_DISCHARGE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 47u, 16u, BC_CAPACITY_mAh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 63u, 8u, BC_ENERGY_Wh, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[5u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x21u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 18u, BC_CURRENT_MAX_CHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 29u, 18u, BC_CURRENT_MAX_CHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 43u, 18u, BC_CURRENT_MAX_CHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData           = 0u;
    float_t temperatureSignals[3u] = {
        (float_t)BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x22u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[0u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 18u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[1u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 29u, 18u, testSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[2u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 43u, 18u, testSignalData[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x23u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 18u, BC_CURRENT_MAX_DISCHARGE_MSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 29u, 18u, BC_CURRENT_MAX_DISCHARGE_RSL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 43u, 18u, BC_CURRENT_MAX_DISCHARGE_MOL_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;

    float_t temperatureSignals[3u] = {
        (float_t)BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x24u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[0u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 18u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[1u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 29u, 18u, testSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[2u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 43u, 18u, testSignalData[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMaxVoltageMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMaxVoltageMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMaxVoltageMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x25u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 18u, BC_VOLTAGE_MAX_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 29u, 18u, BC_VOLTAGE_MAX_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 43u, 18u, BC_VOLTAGE_MAX_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMaxVoltageMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData           = 0u;
    float_t temperatureSignals[3u] = {
        (float_t)BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x26u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[0u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 18u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[1u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 29u, 18u, testSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[2u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 43u, 18u, testSignalData[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData           = 0u;
    float_t temperatureSignals[3u] = {
        (float_t)BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC,
        (float_t)BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x27u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[0u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 18u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[1u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 29u, 18u, testSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxPrepareSignalData_Expect(&temperatureSignals[2u], signalBatteryCellTemperature);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 43u, 18u, testSignalData[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMinVoltageMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatteryCellMuxMinVoltageMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatteryCellMuxMinVoltageMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x28u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 18u, BC_VOLTAGE_MIN_MSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 29u, 18u, BC_VOLTAGE_MIN_RSL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 43u, 18u, BC_VOLTAGE_MIN_MOL_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatteryCellMuxMinVoltageMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SendBatterySystemConfiguration
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/9: CANTX_MUX_BATTERY_SYSTEM_GENERAL_1 multiplexer message isn't queued successfully
 *            - RT2/9: CANTX_MUX_BATTERY_SYSTEM_GENERAL_2 multiplexer message isn't queued successfully
 *            - RT3/9: CANTX_MUX_BATTERY_SYSTEM_CONTACTORS multiplexer message isn't queued successfully
 *            - RT4/9: CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR multiplexer message isn't queued successfully
 *            - RT5/9: CANTX_MUX_BATTERY_SYSTEM_FUSE multiplexer message isn't queued successfully
 *            - RT6/9: CANTX_MUX_BATTERY_SYSTEM_MAX_CURRENT multiplexer message isn't queued successfully
 *            - RT7/9: CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK multiplexer message isn't queued successfully
 *            - RT8/9: CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS multiplexer message isn't queued successfully
 *            - RT9/9: All messages are queued successfully
 */
void testCANTX_SendBatterySystemConfiguration(void) {
    /* ======= Routine tests =============================================== */
    uint8_t testCanDataFilled[CAN_MAX_DLC] = {0x12, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    float_t openWirePeriodSignals[4u]      = {
        (float_t)BS_STANDBY_OPEN_WIRE_PERIOD_ms,
        (float_t)BS_NORMAL_OPEN_WIRE_PERIOD_ms,
        (float_t)BS_CHARGE_OPEN_WIRE_PERIOD_ms,
        (float_t)BS_ERROR_OPEN_WIRE_PERIOD_ms};
    /* ======= RT1/9: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);

    /* ======= RT1/9: Call function under test */
    STD_RETURN_TYPE_e testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT1/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT2/9: Test implementation */
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT2/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT2/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT3/9: Test implementation */
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT3/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT3/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT4/9: Test implementation */
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x33u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 2u, BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR, CAN_BIG_ENDIAN);
#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 1u, 0u, CAN_BIG_ENDIAN);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 0u, 0u, CAN_BIG_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 12u, BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 12u, BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT4/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT4/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT5/9: Test implementation */
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x33u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 2u, BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR, CAN_BIG_ENDIAN);
#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 1u, 0u, CAN_BIG_ENDIAN);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 0u, 0u, CAN_BIG_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 12u, BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 12u, BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x34u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT5/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT5/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT6/9: Test implementation */
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x33u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 2u, BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR, CAN_BIG_ENDIAN);
#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 1u, 0u, CAN_BIG_ENDIAN);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 0u, 0u, CAN_BIG_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 12u, BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 12u, BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x34u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x35u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 28u, BS_MAXIMUM_STRING_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 28u, BS_MAXIMUM_PACK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT6/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT6/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT7/9: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x33u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 2u, BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR, CAN_BIG_ENDIAN);
#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 1u, 0u, CAN_BIG_ENDIAN);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 0u, 0u, CAN_BIG_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 12u, BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 12u, BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x34u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x35u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 28u, BS_MAXIMUM_STRING_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 28u, BS_MAXIMUM_PACK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x36u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 12u, openWirePeriodSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 18u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 17u, 12u, openWirePeriodSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 37u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 36u, 12u, openWirePeriodSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 40u, 12u, openWirePeriodSignals[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT7/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT7/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT8/9: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x33u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 2u, BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR, CAN_BIG_ENDIAN);
#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 1u, 0u, CAN_BIG_ENDIAN);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 0u, 0u, CAN_BIG_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 12u, BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 12u, BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x34u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x35u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 28u, BS_MAXIMUM_STRING_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 28u, BS_MAXIMUM_PACK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x36u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 12u, openWirePeriodSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 18u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 17u, 12u, openWirePeriodSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 37u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 36u, 12u, openWirePeriodSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 40u, 12u, openWirePeriodSignals[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x37u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 14u, BS_NR_OF_CELL_BLOCKS_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 14u, BS_NR_OF_CELL_BLOCKS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 14u, BS_NR_OF_TEMP_SENSORS_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 53u, 14u, BS_NR_OF_TEMP_SENSORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_NOT_OK);
    /* ======= RT8/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT8/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, testResult);

    /* ======= RT9/9: Test implementation */
    CAN_TxPrepareSignalData_Ignore();
    CAN_ConvertBooleanToInteger_IgnoreAndReturn(TEST_PLACEHOLDER_CONVERTED_BOOL);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 12u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x33u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 13u, 2u, BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR, CAN_BIG_ENDIAN);
#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 1u, 0u, CAN_BIG_ENDIAN);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 0u, 0u, CAN_BIG_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 12u, BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 12u, BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x34u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 11u, 12u, BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 31u, 16u, BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x35u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 28u, BS_MAXIMUM_STRING_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 28u, BS_MAXIMUM_PACK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x36u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 14u, 12u, openWirePeriodSignals[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 18u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 17u, 12u, openWirePeriodSignals[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 37u, 1u, TEST_PLACEHOLDER_CONVERTED_BOOL, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 36u, 12u, openWirePeriodSignals[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 40u, 12u, openWirePeriodSignals[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x37u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 15u, 14u, BS_NR_OF_CELL_BLOCKS_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 17u, 14u, BS_NR_OF_CELL_BLOCKS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 35u, 14u, BS_NR_OF_TEMP_SENSORS_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 53u, 14u, BS_NR_OF_TEMP_SENSORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataFilled, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_ReturnThruPtr_pCanData(testCanDataFilled);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_DEBUG_MESSAGE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID,
        CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
        testCanDataFilled,
        STD_OK);
    /* ======= RT9/9: Call function under test */
    testResult = TEST_CANTX_SendBatterySystemConfiguration();
    /* ======= RT9/9: Test output verification */
    TEST_ASSERT_EQUAL(STD_OK, testResult);
}

/**
 * @brief   Testing CANTX_SetBatteryCellMuxMinVoltageMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxGeneral1MessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxGeneral1MessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x30u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 8u, BS_NR_OF_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 23u, 8u, BS_NR_OF_MODULES_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 31u, 8u, BS_NR_OF_CELL_BLOCKS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[4u], 39u, 8u, BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 47u, 8u, BS_NR_OF_GPIOS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 55u, 8u, BS_NR_OF_GPAS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[7u], 63u, 8u, BS_NR_OF_TEMP_SENSORS_PER_MODULE, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxGeneral1MessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[8u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatterySystemMuxGeneral2MessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxGeneral2MessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxGeneral2MessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x31u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_POSITIVE_DISCHARGE_CURRENT, testSignalData[0u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 1u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_IGNORE_INTERLOCK_FEEDBACK, testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 14u, 1u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_CHECK_CAN_TIMING, testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 13u, 1u, testSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_BALANCING_DEFAULT_INACTIVE, testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 12u, 1u, testSignalData[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[5u], 11u, 12u, BS_BALANCING_RESISTANCE_ohm, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 31u, 12u, BS_REST_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[7u], 35u, 16u, BS_RELAXATION_PERIOD_10ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxGeneral2MessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[8u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatterySystemMuxContactorsMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxContactorsMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxContactorsMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x32u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 8u, BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 8u, BS_NR_OF_CONTACTORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 31u, 32u, BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxContactorsMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[4u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatterySystemMuxCurrentSensorMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxCurrentSensorMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxCurrentSensorMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x33u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_CURRENT_SENSOR_PRESENT, testSignalData[0u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 1u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 13u, 2u, BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 1u, 0u, CAN_BIG_ENDIAN);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 14u, 0u, 0u, CAN_BIG_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[4u], 11u, 12u, BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[5u], 31u, 12u, BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[6u], 35u, 12u, BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[7u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
#else
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 55u, 12u, BS_CS_THRESHOLD_NO_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxCurrentSensorMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[8u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatterySystemMuxFuseMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxFuseMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxFuseMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x34u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_CHECK_FUSE_PLACED_IN_CHARGE_PATH, testSignalData[0u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 1u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_CHECK_FUSE_PLACED_IN_NORMAL_PATH, testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 14u, 1u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 11u, 12u, BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[4u], 31u, 16u, BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxFuseMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[5u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatterySystemMuxMaxCurrentMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxMaxCurrentMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxMaxCurrentMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x35u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 28u, BS_MAXIMUM_STRING_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 35u, 28u, BS_MAXIMUM_PACK_CURRENT_mA, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxMaxCurrentMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[3u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatterySystemMuxOpenWireCheckMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxOpenWireCheckMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxOpenWireCheckMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData              = 0u;
    float_t openWirePeriodSignals[4u] = {
        (float_t)BS_STANDBY_OPEN_WIRE_PERIOD_ms,
        (float_t)BS_NORMAL_OPEN_WIRE_PERIOD_ms,
        (float_t)BS_CHARGE_OPEN_WIRE_PERIOD_ms,
        (float_t)BS_ERROR_OPEN_WIRE_PERIOD_ms};
    uint64_t convertedSignalData[3u] = {0u, 1u, 2u};
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x36u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_STANDBY_PERIODIC_OPEN_WIRE_CHECK, convertedSignalData[0u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 15u, 1u, convertedSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&openWirePeriodSignals[0u], signalOpenWirePeriod);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[0u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 14u, 12u, testSignalData[0u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_NORMAL_PERIODIC_OPEN_WIRE_CHECK, convertedSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[3u], 18u, 1u, convertedSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxPrepareSignalData_Expect(&openWirePeriodSignals[1u], signalOpenWirePeriod);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 17u, 12u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    CAN_ConvertBooleanToInteger_ExpectAndReturn(BS_CHARGE_PERIODIC_OPEN_WIRE_CHECK, convertedSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[5u], 37u, 1u, convertedSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[6u]);
    CAN_TxPrepareSignalData_Expect(&openWirePeriodSignals[2u], signalOpenWirePeriod);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[6u], 36u, 12u, testSignalData[2u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[7u]);
    CAN_TxPrepareSignalData_Expect(&openWirePeriodSignals[3u], signalOpenWirePeriod);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[7u], 40u, 12u, testSignalData[3u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[8u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxOpenWireCheckMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[8u], messageData);
}

/**
 * @brief   Testing CANTX_SetBatterySystemMuxTotalNumbersMessageData
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pMessageData -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the message data
 */
void testCANTX_SetBatterySystemMuxTotalNumbersMessageData(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetBatterySystemMuxTotalNumbersMessageData(NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t messageData = 0u;
    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0x37u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 15u, 14u, BS_NR_OF_CELL_BLOCKS_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 17u, 14u, BS_NR_OF_CELL_BLOCKS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[3u], 35u, 14u, BS_NR_OF_TEMP_SENSORS_PER_STRING, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[4u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[4u], 53u, 14u, BS_NR_OF_TEMP_SENSORS, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[5u]);
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetBatterySystemMuxTotalNumbersMessageData(&messageData);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(testMessageData[5u], messageData);
}
