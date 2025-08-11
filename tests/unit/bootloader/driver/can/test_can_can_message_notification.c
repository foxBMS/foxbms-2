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
 * @date    2024-11-28 (date of creation)
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
#include "Mockboot_cfg.h"
#include "Mockcan_bootloader-version-info.h"
#include "Mockcan_cbs.h"
#include "Mockcan_cfg.h"
#include "Mockcan_helper.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockrti.h"

#include "HL_hal_stdtypes.h"

#include "can.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can.c")

TEST_INCLUDE_PATH("../../src/bootloader/driver/can")
TEST_INCLUDE_PATH("../../src/bootloader/driver/config")
TEST_INCLUDE_PATH("../../src/bootloader/driver/rti")
TEST_INCLUDE_PATH("../../src/bootloader/driver/mcu")
TEST_INCLUDE_PATH("../../src/bootloader/driver/io")
TEST_INCLUDE_PATH("../../src/bootloader/engine/boot")
TEST_INCLUDE_PATH("../../src/bootloader/engine/can")

/*========== Definitions and Implementations for Unit Test ==================*/

#define NUM_DATA_READ_SUB_CALLS                         (1)
#define TEST_VALID_CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT (18u)

canBASE_t dummy1           = {0};
const CAN_NODE_s can_node1 = {.pCanNodeRegister = &dummy1};

canBASE_t dummy2                   = {0};
const CAN_NODE_s can_node2Isolated = {.pCanNodeRegister = &dummy2};

CAN_DATA_TRANSFER_INFO_s can_infoOfDataTransfer = {0u};

/* this array reflects the actual implementation, except for the las entry.
 * One entry needs to have a NULL_PTR as callback, as otherwise this
 * condition can not be tested. */
const CAN_RX_MESSAGE_TYPE_s can_rxMessages[CAN_NUMBER_OF_RX_MESSAGES] = {
    {((CAN_NODE_s *)&can_node1),
     {.id         = CAN_RX_REQUEST_MESSAGE_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxActionRequest},
    {((CAN_NODE_s *)&can_node1),
     {.id         = CAN_RX_TRANSFER_PROCESS_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxTransferProcessInfo},
    {((CAN_NODE_s *)&can_node1),
     {.id         = CAN_RX_LOOP_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxLoopInfo},
    {((CAN_NODE_s *)&can_node1),
     {.id         = CAN_RX_DATA_8_BYTES_ID,
      .idType     = CAN_EXTENDED_IDENTIFIER_29_BIT, /* invalid ID length */
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxData8Bytes},
    {((CAN_NODE_s *)&can_node1),
     {.id         = CAN_RX_CRC_8_BYTES_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = NULL_PTR}, /* invalid call back to test NULL_PTR check */
};

const CAN_TX_MESSAGE_TYPE_s can_txMessages[CAN_NUMBER_OF_TX_MESSAGES] = {0u};

CAN_FSM_STATES_e can_stateOfCanCommunication = 0u;
BOOT_FSM_STATES_e boot_state                 = 0u;

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
/**
 * @brief   Iterate over a callback that supplies various scenarios and check if they work as expected
 * @details This function uses the callback #MockCAN_RxActionRequest_Callback() in order to check
 * the message data and properties.
 */
uint32_t MockCAN_RxActionRequest_Callback(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    int num_calls) {
    uint8_t messageData[CAN_DEFAULT_DLC] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    CAN_MESSAGE_PROPERTIES_s blu         = {
                .id         = CAN_RX_REQUEST_MESSAGE_ID,
                .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
                .dlc        = CAN_DEFAULT_DLC,
                .endianness = CAN_LITTLE_ENDIAN};

    /* determine a value depending on num_calls (has to be synchronized with test) */
    switch (num_calls) {
        case 0:
            TEST_ASSERT_EQUAL(blu.id, message.id);
            TEST_ASSERT_EQUAL(blu.idType, message.idType);
            TEST_ASSERT_EQUAL(blu.dlc, message.dlc);
            TEST_ASSERT_EQUAL(blu.endianness, message.endianness);
            for (uint8_t i = 0; i < CAN_DEFAULT_DLC; i++) {
                TEST_ASSERT_EQUAL(messageData[i], kpkCanData[i]);
            }
            break;
        default:
            TEST_FAIL_MESSAGE("DATA_ReadBlock_Callback was called too often");
    }
    /* ENTER HIGHEST CASE NUMBER IN EXPECT; checks whether all cases are used */
    TEST_ASSERT_EQUAL_MESSAGE(0, (NUM_DATA_READ_SUB_CALLS - 1), "Check code of stub. Something does not fit.");

    if (num_calls >= NUM_DATA_READ_SUB_CALLS) {
        TEST_FAIL_MESSAGE("This stub is fishy");
    }

    return 0u;
}

/**
 * @brief   Testing extern function canMessageNotification
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none (arguments are not validated)
 *          - Routine validation:
 *            - RT1/7: message box not of interest &rarr; do nothing
 *            - RT2/7: no new data &rarr; do nothing
 *            - RT3/7: valid data, but no valid CAN ID &rarr; do nothing
 *            - RT4/7: everything valid &rarr; call the callback
 *            - RT5/7: received message generally valid, but callback is
 *                      a NULL_PTR &rarr; do nothing
 *            - RT6/7: received message generally valid, but on an unexpected
 *                     CAN node &rarr; do nothing
 *            - RT7/7:  received message generally valid, but an invalid
 *                     ID type &rarr; do nothing
 */
void testcanMessageNotification(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/7: Test implementation */
    uint32 messageBox = 0u;

    /* ======= RT1/7: call function under test */
    canMessageNotification(can_node1.pCanNodeRegister, messageBox);

    /* ======= RT1/7: test output verification */
    /* nothing to be verified */

    /* ======= RT2/7: Test implementation */
    messageBox            = CAN_NR_OF_TX_MESSAGE_BOX + 1u;
    canGetDataReturnValue = 0u;

    /* ======= RT2/7: call function under test */
    canMessageNotification(can_node1.pCanNodeRegister, messageBox);

    /* ======= RT2/7: test output verification */
    /* nothing to be verified */

    /* ======= RT3/7: Test implementation */
    /* we are in the inner loop, but we did not get a valid CAN ID */
    canGetDataReturnValue = 1u;         /* CAN_HAL_RETVAL_NO_DATA_LOST */
    canGetIDReturnValue   = UINT32_MAX; /* invalid CAN ID in foxBMS */

    /* ======= RT3/7: call function under test */
    canMessageNotification(can_node1.pCanNodeRegister, messageBox);

    /* ======= RT3/7: test output verification */
    /* nothing to be verified */

    /* ======= RT4/7: Test implementation */
    /* everything valid:
     * 1) message received on expected CAN node
     * 2) CAN ID matches expected ID
     * 3) ID types match
     * therefore we handle the message and call the callback
     * */
    canGetDataReturnValue = 1u; /* CAN_HAL_RETVAL_NO_DATA_LOST */
    canGetIDReturnValue   = CAN_RX_REQUEST_MESSAGE_ID << TEST_VALID_CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT;

    /* Now inside the Callback */
    /* uint8_t messageData[CAN_DEFAULT_DLC] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    CAN_MESSAGE_PROPERTIES_s blu         = {
                .id         = CAN_RX_REQUEST_MESSAGE_ID,
                .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
                .dlc        = CAN_DEFAULT_DLC,
                .endianness = CAN_LITTLE_ENDIAN}; */
    CAN_RxActionRequest_Stub(MockCAN_RxActionRequest_Callback);
    /* TODO: the argument validation fails on Linux. Once this is fixed,
     * we need remove 'ignore_arg' in the unit test configuration files */
    /* CAN_RxActionRequest_IgnoreArg_message(); */

    /* ======= RT4/7: call function under test */
    canMessageNotification(can_node1.pCanNodeRegister, messageBox);

    /* ======= RT4/7: test output verification */
    /* nothing to be verified */

    /* ======= RT5/7: Test implementation */
    /* this message contains (for testing purposes) a NULL_PTR as callback
     * and because of that we do not call the callback */
    canGetDataReturnValue = 1u; /* CAN_HAL_RETVAL_NO_DATA_LOST */
    canGetIDReturnValue   = CAN_RX_CRC_8_BYTES_ID << TEST_VALID_CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT;

    /* ======= RT5/7: call function under test */
    canMessageNotification(can_node1.pCanNodeRegister, messageBox);

    /* ======= RT5/7: test output verification */
    /* nothing to be verified */

    /* ======= RT6/7: Test implementation */
    /* message is received on an unexpected CAN node */
    canGetDataReturnValue = 1u; /* CAN_HAL_RETVAL_NO_DATA_LOST */
    canGetIDReturnValue   = CAN_RX_REQUEST_MESSAGE_ID << TEST_VALID_CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT;

    /* ======= RT6/7: call function under test */
    canMessageNotification(can_node2Isolated.pCanNodeRegister, messageBox);

    /* ======= RT6/7: test output verification */
    /* nothing to be verified */

    /* ======= RT7/7: Test implementation */
    /* this message contains (for testing purposes) an invalid ID type
     * and because of that we do not call the callback */
    canGetDataReturnValue = 1u; /* CAN_HAL_RETVAL_NO_DATA_LOST */
    canGetIDReturnValue   = CAN_RX_DATA_8_BYTES_ID << TEST_VALID_CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT;

    /* ======= RT7/7: call function under test */
    canMessageNotification(can_node1.pCanNodeRegister, messageBox);

    /* ======= RT7/7: test output verification */
    /* nothing to be verified */
}
