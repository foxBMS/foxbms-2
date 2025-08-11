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
 * @file    test_can_can_message_notification.c
 * @author  foxBMS Team
 * @date    2025-08-06 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of function 'canMessageNotification'
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_het.h"
#include "MockHL_reg_can.h"
#include "MockHL_reg_system.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockftask.h"
#include "Mockio.h"
#include "Mockos.h"
#include "Mockpex.h"

#include "HL_can.h"

#include "can.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/mcu")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/engine/config")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/* see src/app/driver/config/can_cfg_rx-message-definitions.h, but we omit
   this include here */
#define CANRX_NOT_PERIODIC (0u)

/* TX test case */
#define TEST_CANTX_ID_DUMMY        (0x001)
#define TEST_CANTX_DUMMY_PERIOD_ms (51u) /* Period must be divisible by CAN Tick */
#define TEST_CANTX_DUMMY_PHASE_ms  (11u) /* Phase must be divisible by CAN Tick */

#define TEST_CANTX_DUMMY_MESSAGE                                                 \
    {                                                                            \
        .id         = TEST_CANTX_ID_DUMMY,                                       \
        .dlc        = CAN_DEFAULT_DLC,                                           \
        .endianness = CAN_LITTLE_ENDIAN,                                         \
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,                            \
    },                                                                           \
    {                                                                            \
        .period = TEST_CANTX_DUMMY_PERIOD_ms, .phase = TEST_CANTX_DUMMY_PHASE_ms \
    }

/* Rx test case*/
#define TEST_CANRX_ID_DUMMY (0x002)

#define TEST_CANRX_DUMMY_MESSAGE                      \
    {                                                 \
        .id         = TEST_CANRX_ID_DUMMY,            \
        .dlc        = CAN_DEFAULT_DLC,                \
        .endianness = CAN_LITTLE_ENDIAN,              \
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT, \
    },                                                \
    {                                                 \
        .period = CANRX_NOT_PERIODIC                  \
    }

const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

const CAN_NODE_s can_node2Isolated = {
    .canNodeRegister = canREG2,
};
const CAN_SHIM_s can_kShim = {0};

static uint32_t TEST_CANTX_DummyCallback(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    return 0;
}

static uint32_t TEST_CANRX_DummyCallback(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    return 0;
}

const CAN_TX_MESSAGE_TYPE_s can_txMessages[] = {
    {CAN_NODE_1, TEST_CANTX_DUMMY_MESSAGE, &TEST_CANTX_DummyCallback, NULL_PTR},
};

const CAN_RX_MESSAGE_TYPE_s can_rxMessages[] = {
    {CAN_NODE_1, TEST_CANRX_DUMMY_MESSAGE, &TEST_CANRX_DummyCallback},
};

const uint8_t can_txMessagesLength = sizeof(can_txMessages) / sizeof(can_txMessages[0]);
const uint8_t can_rxMessagesLength = sizeof(can_rxMessages) / sizeof(can_rxMessages[0]);

OS_QUEUE ftsk_canRxQueue               = NULL_PTR;
OS_QUEUE ftsk_canTxUnsentMessagesQueue = NULL_PTR;
volatile bool ftsk_allQueuesCreated    = false;

/* Manually mocking functions from HL_can.h */
/* not called for this test; no implementation details needed */
void canInit(void) {
}

/* not called for this test; no implementation details needed */
uint32 canTransmit(canBASE_t *node, uint32 messageBox, const uint8 *data) {
    return 0u;
}

/* not called for this test; no implementation details needed */
uint32 canIsTxMessagePending(canBASE_t *node, uint32 messageBox) {
    return 0u;
}

/* not called for this test; no implementation details needed */
void canUpdateID(canBASE_t *node, uint32 messageBox, uint32 msgBoxArbitVal) {
}

/* The return value of 'canGetData' determines the function flow in
 * 'canMessageNotification', therefore an implementation is needed that allows
 * to test all branches, i.e., we return the mock value
 * 'canGetDataReturnValue'.
 */
uint32_t canGetDataReturnValue = 0u;
uint32 canGetData(canBASE_t *node, uint32 messageBox, uint8 *const data) {
    return canGetDataReturnValue;
}
/* The return value of 'canGetID' determines the function flow in
 * 'canMessageNotification', therefore an implementation is needed that allows
 * to test all branches, i.e., we return the mock value
 * 'canGetIDReturnValue'.
 */
uint32_t canGetIDReturnValue = 0u;
uint32 canGetID(canBASE_t *node, uint32 messageBox) {
    return canGetIDReturnValue;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testcanMessageNotification(void) {
    canMessageNotification(NULL_PTR, 0u);
}
