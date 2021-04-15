/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    test_can.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2020-04-01 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_can.h"
#include "Mockcan_cfg.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockos.h"

#include "can.h"
#include "test_assert_helper.h"

TEST_FILE("can.c")

/*========== Definitions and Implementations for Unit Test ==================*/
static uint32_t can_dummy(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData) {
    return 0;
}

const CAN_MSG_TX_TYPE_s can_txMessages[] = {
    {0x001, 8, 100, 0, littleEndian, &can_dummy},
};

const CAN_MSG_RX_TYPE_s can_rxMessages[] = {
    {0x002, 8, 0, littleEndian, &can_dummy},
};

const uint8_t can_txLength = sizeof(can_txMessages) / sizeof(can_txMessages[0]);
const uint8_t can_rxLength = sizeof(can_rxMessages) / sizeof(can_rxMessages[0]);

CAN_STATE_s *canTestState = NULL_PTR;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    canTestState = TEST_CAN_GetCANState();

    canTestState->periodicEnable = false;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        canTestState->currentSensorPresent[stringNumber]   = false;
        canTestState->currentSensorCCPresent[stringNumber] = false;
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testDataSendNullPointerAsNode(void) {
    uint8_t data = 0;
    canIsTxMessagePending_IgnoreAndReturn(0u);
    canUpdateID_Ignore();
    canTransmit_IgnoreAndReturn(0u);
    TEST_ASSERT_FAIL_ASSERT(CAN_DataSend(NULL_PTR, 0u, &data));
}

void testDataSendNullPointerAsData(void) {
    canBASE_t node = {0};
    canIsTxMessagePending_IgnoreAndReturn(0u);
    canUpdateID_Ignore();
    canTransmit_IgnoreAndReturn(0u);
    TEST_ASSERT_FAIL_ASSERT(CAN_DataSend(&node, 0u, NULL_PTR));
}

void testDataSendNoMessagePending(void) {
    canBASE_t node = {0};
    uint8_t data   = 0;

    canIsTxMessagePending_IgnoreAndReturn(1u);

    for (uint8_t i = 0u; i < 32; i++) {
        TEST_ASSERT_EQUAL(STD_NOT_OK, CAN_DataSend(&node, i, &data));
    }
}

void testDataSendMessagePending(void) {
    canBASE_t node = {0};
    uint8_t data   = 0;

    /* simulate first messageBox has pending message */
    canIsTxMessagePending_ExpectAndReturn(&node, 1, 0u);
    canUpdateID_Expect(&node, 1, 0x20040000u);
    canTransmit_ExpectAndReturn(&node, 1, &data, 0u);
    TEST_ASSERT_EQUAL(STD_OK, CAN_DataSend(&node, 0x001, &data));

    /* simulate messageBox until the highest to have no pending messages */
    for (uint8_t messageBox = 1u; messageBox < (CAN_NR_OF_TX_MESSAGEBOX - 1); messageBox++) {
        canIsTxMessagePending_ExpectAndReturn(&node, messageBox, 1u);
    }
    /* last message box has message pending */
    canIsTxMessagePending_ExpectAndReturn(&node, CAN_NR_OF_TX_MESSAGEBOX - 1, 0u);
    canUpdateID_Expect(&node, CAN_NR_OF_TX_MESSAGEBOX - 1, 0x20040000u);
    canTransmit_ExpectAndReturn(&node, CAN_NR_OF_TX_MESSAGEBOX - 1, &data, 0u);
    TEST_ASSERT_EQUAL(STD_OK, CAN_DataSend(&node, 0x001, &data));
}

void testEnablePeriodic(void) {
    /* check state before */
    TEST_ASSERT_EQUAL(false, canTestState->periodicEnable);

    CAN_EnablePeriodic(false);
    TEST_ASSERT_EQUAL(false, canTestState->periodicEnable);

    CAN_EnablePeriodic(true);
    TEST_ASSERT_EQUAL(true, canTestState->periodicEnable);
}

void testIsCurrentSensorPresent(void) {
    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        /* check state before */
        TEST_ASSERT_EQUAL(false, canTestState->currentSensorPresent[stringNumber]);

        TEST_ASSERT_EQUAL(false, CAN_IsCurrentSensorPresent(stringNumber));

        /* set state to true */
        canTestState->currentSensorPresent[stringNumber] = true;

        /* check state again */
        TEST_ASSERT_EQUAL(true, canTestState->currentSensorPresent[stringNumber]);
    }
}

void testIsCurrentSensorCcPresent(void) {
    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        /* check state before */
        TEST_ASSERT_EQUAL(false, canTestState->currentSensorCCPresent[stringNumber]);

        TEST_ASSERT_EQUAL(false, CAN_IsCurrentSensorCcPresent(stringNumber));

        /* set state to true */
        canTestState->currentSensorCCPresent[stringNumber] = true;

        /* check state again */
        TEST_ASSERT_EQUAL(true, canTestState->currentSensorCCPresent[stringNumber]);
    }
}
