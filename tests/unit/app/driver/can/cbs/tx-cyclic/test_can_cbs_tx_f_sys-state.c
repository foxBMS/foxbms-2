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
 * @file    test_can_cbs_tx_f_sys-state.c
 * @author  foxBMS Team
 * @date    2021-07-27 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mocksys.h"
#include "Mocksys_mon.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_f_sys-state.c")

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
TEST_INCLUDE_PATH("../../src/app/engine/sys")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
const CAN_SHIM_s can_kShim = {
    .pQueueImd             = NULL_PTR,
    .pTableCellVoltage     = NULL_PTR,
    .pTableCellTemperature = NULL_PTR,
    .pTableMinMax          = NULL_PTR,
    .pTableCurrent         = NULL_PTR,
    .pTableOpenWire        = NULL_PTR,
    .pTableStateRequest    = NULL_PTR,
    .pTablePackValues      = NULL_PTR,
    .pTableSof             = NULL_PTR,
    .pTableSoc             = NULL_PTR,
    .pTableSoe             = NULL_PTR,
    .pTableErrorState      = NULL_PTR,
    .pTableInsulation      = NULL_PTR,
    .pTableMsl             = NULL_PTR,
    .pTableRsl             = NULL_PTR,
    .pTableMol             = NULL_PTR,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCANTX_SysState(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_SYSTEM_STATE_ID,
        .idType     = CANTX_SYSTEM_STATE_ID_TYPE,
        .dlc        = CANTX_SYSTEM_STATE_DLC,
        .endianness = CANTX_SYSTEM_STATE_ENDIANNESS,
    };

    uint8_t testCanDataZeroArray[CANTX_SYSTEM_STATE_DLC] = {0u};
    uint64_t testMessageData[3]                          = {0u, 1u, 2u};
    uint8_t testMuxId                                    = 10u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testErrorMessageId = {
        .id         = CANTX_BMS_STATE_DETAILS_ID,
        .idType     = CANTX_SYSTEM_STATE_ID_TYPE,
        .dlc        = CANTX_SYSTEM_STATE_DLC,
        .endianness = CANTX_SYSTEM_STATE_ENDIANNESS,
    };
    TEST_ASSERT_FAIL_ASSERT(CANTX_SysState(testErrorMessageId, testCanDataZeroArray, NULL_PTR, &can_kShim));
    /* ======= AT2/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testErrorMessageIdType = {
        .id         = CANTX_SYSTEM_STATE_ID,
        .idType     = CAN_EXTENDED_IDENTIFIER_29_BIT,
        .dlc        = CANTX_SYSTEM_STATE_DLC,
        .endianness = CANTX_SYSTEM_STATE_ENDIANNESS,
    };
    TEST_ASSERT_FAIL_ASSERT(CANTX_SysState(testErrorMessageIdType, testCanDataZeroArray, NULL_PTR, &can_kShim));
    /* ======= AT3/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testErrorMessageDlc = {
        .id         = CANTX_SYSTEM_STATE_ID,
        .idType     = CANTX_SYSTEM_STATE_ID_TYPE,
        .dlc        = CANTX_SYSTEM_STATE_DLC - 1,
        .endianness = CANTX_SYSTEM_STATE_ENDIANNESS,
    };
    TEST_ASSERT_FAIL_ASSERT(CANTX_SysState(testErrorMessageDlc, testCanDataZeroArray, NULL_PTR, &can_kShim));
    /* ======= AT4/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testErrorMessageEndianness = {
        .id         = CANTX_SYSTEM_STATE_ID,
        .idType     = CANTX_SYSTEM_STATE_ID_TYPE,
        .dlc        = CANTX_SYSTEM_STATE_DLC,
        .endianness = CAN_LITTLE_ENDIAN,
    };
    TEST_ASSERT_FAIL_ASSERT(CANTX_SysState(testErrorMessageEndianness, testCanDataZeroArray, NULL_PTR, &can_kShim));

    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_SysState(testMessage, NULL_PTR, NULL_PTR, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_SysState(testMessage, testCanDataZeroArray, &testMuxId, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_SysState(testMessage, testCanDataZeroArray, NULL_PTR, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test working case */
    SYS_GetSystemState_ExpectAndReturn(1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 0u, 5u, 1u, CANTX_SYSTEM_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);

    SYS_GetSystemSubstate_ExpectAndReturn(1u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 7u, 7u, 1u, CANTX_SYSTEM_STATE_ENDIANNESS);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);

    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[2u], testCanDataZeroArray, CANTX_SYSTEM_STATE_ENDIANNESS);

    /* ======= RT1/1: Call function under test */
    CANTX_SysState(testMessage, testCanDataZeroArray, NULL_PTR, &can_kShim);
}
