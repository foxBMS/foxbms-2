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
 * @file    test_can_cbs_tx_fatal-error.c
 * @author  foxBMS Team
 * @date    2024-10-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details Not Yet Implemented
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

#include "can_cbs_tx_fatal-error.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_fatal-error.c")

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
#define CANTX_FATAL_ERROR_START_BIT (7u)
#define CANTX_FATAL_ERROR_LENGTH    (8u)

const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCANTX_SendFatalErrorIdOk(void) {
    uint64_t messageData = 0u;
    uint8_t data[]       = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};
    uint32_t errorId     = 14;

    CAN_TxSetMessageDataWithSignalData_Expect(
        &messageData, CANTX_FATAL_ERROR_START_BIT, CANTX_FATAL_ERROR_LENGTH, errorId, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_Expect(messageData, &data[0], CAN_BIG_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_FATAL_ERROR_MESSAGE, CANTX_BMS_FATAL_ERROR_ID, CANTX_BMS_FATAL_ERROR_ID_TYPE, &data[0], STD_OK);
    TEST_ASSERT_EQUAL(CANTX_SendFatalErrorId(14), STD_OK);
}

void testCANTX_SendFatalErrorIdNotOk(void) {
    uint64_t messageData = 0u;
    uint8_t data[]       = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};
    uint32_t errorId     = 14;

    CAN_TxSetMessageDataWithSignalData_Expect(
        &messageData, CANTX_FATAL_ERROR_START_BIT, CANTX_FATAL_ERROR_LENGTH, errorId, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData_Expect(messageData, &data[0], CAN_BIG_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_FATAL_ERROR_MESSAGE, CANTX_BMS_FATAL_ERROR_ID, CANTX_BMS_FATAL_ERROR_ID_TYPE, &data[0], STD_NOT_OK);
    TEST_ASSERT_EQUAL(CANTX_SendFatalErrorId(14), STD_NOT_OK);
}

void testCANTX_CANTX_SendMessageFatalErrorCodeOk(void) {
    uint64_t messageData = 1234u;
    uint8_t data[]       = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    CAN_TxSetCanDataWithMessageData_Expect(messageData, &data[0], CAN_BIG_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_FATAL_ERROR_MESSAGE, CANTX_BMS_FATAL_ERROR_ID, CANTX_BMS_FATAL_ERROR_ID_TYPE, &data[0], STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, TEST_CANTX_SendMessageFatalErrorCode(1234));
}

void testCANTX_CANTX_SendMessageFatalErrorCodeNotOk(void) {
    uint64_t messageData = 1234u;
    uint8_t data[]       = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    CAN_TxSetCanDataWithMessageData_Expect(messageData, &data[0], CAN_BIG_ENDIAN);
    CAN_DataSend_ExpectAndReturn(
        CAN_NODE_FATAL_ERROR_MESSAGE, CANTX_BMS_FATAL_ERROR_ID, CANTX_BMS_FATAL_ERROR_ID_TYPE, &data[0], STD_NOT_OK);
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_CANTX_SendMessageFatalErrorCode(1234));
}
