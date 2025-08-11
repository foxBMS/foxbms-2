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
 * @file    test_bender_iso165c.c
 * @author  foxBMS Team
 * @date    2021-01-19 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the Bender iso165c driver
 * @details Tests message composition
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockcan_cbs_rx.h"
#include "Mockcan_cbs_tx_imd-request.h"
#include "Mockcan_cfg.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockftask.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "bender_iso165c.h"
#include "can_cfg_rx-message-definitions.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/imd/bender/iso165c")
TEST_INCLUDE_PATH("../../src/app/driver/imd/bender/iso165c/config")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

OS_QUEUE ftsk_dataQueue             = NULL_PTR;
OS_QUEUE ftsk_imdCanDataQueue       = NULL_PTR;
OS_QUEUE ftsk_canRxQueue            = NULL_PTR;
volatile bool ftsk_allQueuesCreated = false;

const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

const CAN_NODE_s can_node2Isolated = {
    .canNodeRegister = canREG2, /* Isolated CAN interface */
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test function to compose/use CAN messages.
 *
 */
void testMessageComposition(void) {
    CAN_BUFFER_ELEMENT_s canMessage;
    uint8_t command;
    uint8_t tries;

    /* Check assertion of invalid parameter */
    canMessage.data[0u] = 0xA;
    command             = 0xA;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckResponse(command, NULL_PTR));

    /* Check that response ID corresponds to awaited acknowledge */
    canMessage.id       = CANRX_IMD_RESPONSE_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xA;
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    TEST_ASSERT_EQUAL(1u, TEST_I165C_CheckResponse(command, &canMessage));

    /* Check that response ID does not correspond to awaited acknowledge */
    canMessage.id       = CANRX_IMD_RESPONSE_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xB;
    for (uint8_t i = 0; i < 5; i++) {
        OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
        OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    }
    TEST_ASSERT_EQUAL(0u, TEST_I165C_CheckResponse(command, &canMessage));

    /* Check that response failed if ID is not CANRX_IMD_RESPONSE_ID, even if response matches command */
    canMessage.id       = CANRX_IMD_INFO_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xA;
    for (uint8_t i = 0; i < 5; i++) {
        OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
        OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    }
    TEST_ASSERT_EQUAL(0u, TEST_I165C_CheckResponse(command, &canMessage));

    /* Check that response failed if ID is not CANRX_IMD_RESPONSE_ID, if response does not match command */
    canMessage.id       = CANRX_IMD_INFO_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xB;
    for (uint8_t i = 0; i < 5; i++) {
        OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
        OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    }
    TEST_ASSERT_EQUAL(0u, TEST_I165C_CheckResponse(command, &canMessage));

    canMessage.id = I165C_MESSAGE_TYPE_IMD_INFO;
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_GetImdInfo(NULL_PTR));

    /* Test that an IMD_info frame was received on CAN */
    TEST_ASSERT_EQUAL(true, TEST_I165C_GetImdInfo(&canMessage));

    canMessage.id = I165C_MESSAGE_TYPE_IMD_INFO + 1u;
    for (uint8_t i = 0; i < 5; i++) {
        OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
        OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    }
    TEST_ASSERT_EQUAL(false, TEST_I165C_GetImdInfo(&canMessage));

    /* ----------- Test function that waits for acknowledge -----------------*/

    /* Check for invalid function parameters */
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckAcknowledgeArrived(command, NULL_PTR, NULL_PTR));
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckAcknowledgeArrived(command, NULL_PTR, &canMessage));
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckAcknowledgeArrived(command, &tries, NULL_PTR));

    /* Acknowledge arrived */
    canMessage.id       = CANRX_IMD_RESPONSE_ID;
    tries               = 0u;
    command             = 0xA;
    canMessage.data[0]  = 0xA;
    uint8_t ackReceived = 0u;

    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    ackReceived = TEST_I165C_CheckAcknowledgeArrived(command, &tries, &canMessage);
    TEST_ASSERT_EQUAL(1u, ackReceived);
    TEST_ASSERT_EQUAL(0u, tries);

    /* Acknowledge not arrived, increment try counter */
    canMessage.id      = CANRX_IMD_RESPONSE_ID;
    tries              = 0u;
    command            = 0xA;
    canMessage.data[0] = 0xB;

    for (uint8_t i = 0; i < 5; i++) {
        OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
        OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    }
    ackReceived = TEST_I165C_CheckAcknowledgeArrived(command, &tries, &canMessage);
    TEST_ASSERT_EQUAL(1u, tries);
    TEST_ASSERT_EQUAL(0u, ackReceived);

    /* Acknowledge not arrived, and allowed number of tries made, restart
     * Initialization (go to self test) */
    tries              = I165C_TRANSMISSION_ATTEMPTS - 1u;
    command            = 0xA;
    canMessage.data[0] = 0xB;

    for (uint8_t i = 0; i < 5; i++) {
        OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_imdCanDataQueue, 1u);
        OS_ReceiveFromQueue_ExpectAndReturn(ftsk_imdCanDataQueue, (void *)&canMessage, 0u, OS_SUCCESS);
    }
    ackReceived = TEST_I165C_CheckAcknowledgeArrived(command, &tries, &canMessage);
    TEST_ASSERT_EQUAL(I165C_TRANSMISSION_ATTEMPTS, tries);
    TEST_ASSERT_EQUAL(0u, ackReceived);
}
