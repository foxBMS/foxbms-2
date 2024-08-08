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
 * @file    test_can_cbs_tx_imd-request.c
 * @author  foxBMS Team
 * @date    2023-06-14 (date of creation)
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
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "bender_iso165c_cfg.h"
#include "database_cfg.h"

#include "can_cbs_tx_imd-request.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_imd-request.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/imd/bender/iso165c/config")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
uint64_t testMessageData[4u] = {0u, 1u, 2u, 3u};

/* dummy data array filled with zero */
uint8_t testCanDataZeroArray[CAN_MAX_DLC] = {0};

const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

const CAN_NODE_s can_node2Isolated = {
    .canNodeRegister = canREG2,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/* Test invalid inputs */
void testCANTX_ImdRequestInput(void) {
    TEST_ASSERT_FAIL_ASSERT(CANTX_ImdRequest(CANTX_IMD_REQUEST_LAST_ACTION))
}

void testCANTX_RequestRelayOpenInput(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_RequestRelayOpen(2u));
}

void testCANTX_RequestRelayCloseInput(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_RequestRelayClose(2u));
}

void testCANTX_RequestRelayStateInput(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_RequestRelayState(2u));
}

/* Test helper functions */
void testCANTX_RequestRelayOpen(void) {
    /* test to open negative relay */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_SET_HV_RELAIS, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_D_VIFC_HV_RELAIS_NEGATIVE, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 24u, 16u, I165C_RELAY_STATE_OPEN, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_OPEN_NEGATIVE_RELAY));

    /* test to open positive relay */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_SET_HV_RELAIS, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_D_VIFC_HV_RELAIS_POSITIVE, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 24u, 16u, I165C_RELAY_STATE_OPEN, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_OPEN_POSITIVE_RELAY));
}

void testCANTX_RequestRelayClose(void) {
    /* test to close negative relay */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_SET_HV_RELAIS, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_D_VIFC_HV_RELAIS_NEGATIVE, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 24u, 16u, I165C_RELAY_STATE_CLOSED, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_CLOSE_NEGATIVE_RELAY));

    /* test to close positive relay */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_SET_HV_RELAIS, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_D_VIFC_HV_RELAIS_POSITIVE, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[2u], 24u, 16u, I165C_RELAY_STATE_CLOSED, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_CLOSE_POSITIVE_RELAY));
}

void testCANTX_RequestRelayState(void) {
    /* test to get negative relay state */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_GET_HV_RELAIS, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_D_VIFC_HV_RELAIS_NEGATIVE, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_NEGATIVE_RELAY_STATE));

    /* test to get positive relay state */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_GET_HV_RELAIS, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_D_VIFC_HV_RELAIS_POSITIVE, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_POSITIVE_RELAY_STATE));
}

void testCANTX_RequestEnableMeasurement(void) {
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_CTL_MEASUREMENT, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_ENABLE_MEASUREMENT, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_ENABLE_MEASUREMENT));
}

void testCANTXRequestDisableMeasurement(void) {
    /* test to get negative relay state */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_CTL_MEASUREMENT, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_DISABLE_MEASUREMENT, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_DISABLE_MEASUREMENT));
}

void testCANTXRequestSetAveragingFactor(void) {
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_IMC_SET_MEAN_FACTOR, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_MEASUREMENT_AVERAGING_FACTOR, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_SET_AVERAGING_FACTOR));
}

void testCANTX_RequestReadResistance(void) {
    /* test to get negative relay state */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_IMC_GET_R_ISO, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[1u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_READ_RESISTANCE));
}

void testCANTX_RequestUnlock(void) {
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_VIFC_CTL_LOCK, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_LOCK_MODE_UNLOCKED, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 24u, 16u, I165C_UNLOCK_PASSWORD, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_UNLOCK));
}

void testCANTX_RequestSelfTest(void) {
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_IMC_CTL_SELFTEST, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
#ifdef I165C_SELF_TEST_LONG
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_SELFTEST_SCENARIO_OVERALL, CAN_LITTLE_ENDIAN);
#else /* I165C_SELF_TEST_SHORT */
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_SELFTEST_SCENARIO_PARAMETERCONFIG, CAN_LITTLE_ENDIAN);
#endif
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_SELF_TEST));
}

void testCANTX_SetErrorThreshold(void) {
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_IMC_SET_R_ISO_ERR_THR, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_ERROR_THRESHOLD_kOhm, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_SET_ERROR_THRESHOLD));
}

void testCANTX_SetWarningThreshold(void) {
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[0u], 0u, 8u, I165C_CMD_S_IMC_SET_R_ISO_WRN_THR, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(
        &testMessageData[1u], 8u, 16u, I165C_WARNING_THRESHOLD_kOhm, CAN_LITTLE_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CAN_LITTLE_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        I165C_CAN_NODE, I165C_MESSAGE_TYPE_IMD_REQUEST, I165C_TX_MESSAGE_IDENTIFIER_TYPE, testCanDataZeroArray, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, CANTX_ImdRequest(CANTX_IMD_REQUEST_INITIALIZATION_SET_WARNING_THRESHOLD));
}
