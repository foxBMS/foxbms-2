/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_bender_iso165c.c
 * @author  foxBMS Team
 * @date    2021-01-19 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the Bender iso165c driver
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
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
    uint8_t dataWord;
    uint8_t dataByte;
    uint16_t data16;
    uint8_t data8;
    uint8_t command;
    uint8_t id;
    uint8_t tries;

    /* Do as if there is a message in the queue */
    OS_GetNumberOfStoredMessagesInQueue_IgnoreAndReturn(1u);
    MPU_xQueueReceive_IgnoreAndReturn(1u);
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_SUCCESS);

    canMessage.id = 0x22u;
    for (uint8_t i = 1u; i <= 8u; i++) {
        canMessage.data[i] = i;
    }
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_ResetCanData(NULL_PTR));
    /* Reset must set all data to 0 */
    TEST_I165C_ResetCanData(&canMessage);
    for (uint8_t i = 0u; i < 8u; i++) {
        TEST_ASSERT_EQUAL(0u, canMessage.data[i]);
    }

    data16   = 666u;
    dataWord = I165C_DW1;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_WriteDataWord(dataWord, data16, NULL_PTR););
    /* Test that data is written correctly to CAN frame */
    TEST_I165C_WriteDataWord(dataWord, data16, &canMessage);
    TEST_ASSERT_EQUAL(0x9Au, canMessage.data[1u]);
    TEST_ASSERT_EQUAL(0x02u, canMessage.data[2u]);
    dataWord = I165C_DW2;
    TEST_I165C_WriteDataWord(dataWord, data16, &canMessage);
    TEST_ASSERT_EQUAL(0x9Au, canMessage.data[3u]);
    TEST_ASSERT_EQUAL(0x02u, canMessage.data[4u]);

    for (uint8_t i = 1u; i <= 4u; i++) {
        canMessage.data[i] = i;
    }
    data16   = 0u;
    dataWord = I165C_DW1;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_ReadDataWord(dataWord, NULL_PTR, canMessage));
    /* Test that dataword data is read correctly from usual CAN frame */
    TEST_I165C_ReadDataWord(dataWord, &data16, canMessage);
    TEST_ASSERT_EQUAL(0x201u, data16);
    data16   = 0u;
    dataWord = I165C_DW2;
    TEST_I165C_ReadDataWord(dataWord, &data16, canMessage);
    TEST_ASSERT_EQUAL(0x403u, data16);

    for (uint8_t i = 0u; i <= 5u; i++) {
        canMessage.data[i] = i + 1u;
    }
    data16   = 0u;
    dataWord = I165C_DW1;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_ReadDataWordImdInfo(dataWord, NULL_PTR, canMessage));
    /* Test that dataword data is read correctly from special IMD_Info CAN frame */
    TEST_I165C_ReadDataWordImdInfo(dataWord, &data16, canMessage);
    TEST_ASSERT_EQUAL(0x201u, data16);
    data16   = 0u;
    dataWord = I165C_DW2;
    TEST_I165C_ReadDataWordImdInfo(dataWord, &data16, canMessage);
    TEST_ASSERT_EQUAL(0x403u, data16);
    data16   = 0u;
    dataWord = I165C_DW3;
    TEST_I165C_ReadDataWordImdInfo(dataWord, &data16, canMessage);
    TEST_ASSERT_EQUAL(0x605u, data16);

    for (uint8_t i = 1u; i <= 4u; i++) {
        canMessage.data[i] = i;
    }
    data8    = 0u;
    dataByte = I165C_DB1;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_ReadDataByte(dataByte, NULL_PTR, canMessage));
    /* Test that databyte data is read correctly from usual CAN frame */
    TEST_I165C_ReadDataByte(dataByte, &data8, canMessage);
    TEST_ASSERT_EQUAL(0x1u, data8);
    data8    = 0u;
    dataByte = I165C_DB2;
    TEST_I165C_ReadDataByte(dataByte, &data8, canMessage);
    TEST_ASSERT_EQUAL(0x2u, data8);
    data8    = 0u;
    dataByte = I165C_DB3;
    TEST_I165C_ReadDataByte(dataByte, &data8, canMessage);
    TEST_ASSERT_EQUAL(0x3u, data8);
    data8    = 0u;
    dataByte = I165C_DB4;
    TEST_I165C_ReadDataByte(dataByte, &data8, canMessage);
    TEST_ASSERT_EQUAL(0x4u, data8);

    id      = 0xA;
    command = 0xB;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_WriteCmd(id, command, NULL_PTR));
    /* Test that cmd is written correctly to CAN frame */
    TEST_I165C_WriteCmd(id, command, &canMessage);
    TEST_ASSERT_EQUAL(0xAu, canMessage.id);
    TEST_ASSERT_EQUAL(I165C_RX_MESSAGE_IDENTIFIER_TYPE, canMessage.idType);
    TEST_ASSERT_EQUAL(0xBu, canMessage.data[0u]);

    /* Check assertion of invalid parameter */
    canMessage.data[0u] = 0xA;
    command             = 0xA;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckResponse(command, NULL_PTR));

    /* Check that response ID corresponds to awaited acknowledge */
    canMessage.id       = CANRX_IMD_RESPONSE_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xA;
    TEST_ASSERT_EQUAL(true, TEST_I165C_CheckResponse(command, &canMessage));

    /* Check that response ID does not correspond to awaited acknowledge */
    canMessage.id       = CANRX_IMD_RESPONSE_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xB;
    TEST_ASSERT_EQUAL(false, TEST_I165C_CheckResponse(command, &canMessage));

    /* Check that response failed if ID is not CANRX_IMD_RESPONSE_ID, even if response matches command */
    canMessage.id       = CANRX_IMD_INFO_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xA;
    TEST_ASSERT_EQUAL(false, TEST_I165C_CheckResponse(command, &canMessage));

    /* Check that response failed if ID is not CANRX_IMD_RESPONSE_ID, if respose does not match command */
    canMessage.id       = CANRX_IMD_INFO_ID;
    canMessage.data[0u] = 0xA;
    command             = 0xB;
    TEST_ASSERT_EQUAL(false, TEST_I165C_CheckResponse(command, &canMessage));

    canMessage.id = I165C_MESSAGETYPE_IMD_INFO;
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_GetImdInfo(NULL_PTR));
    /* Test that an IMD_info frame was received on CAN */
    TEST_ASSERT_EQUAL(true, TEST_I165C_GetImdInfo(&canMessage));
    canMessage.id = I165C_MESSAGETYPE_IMD_INFO + 1u;
    TEST_ASSERT_EQUAL(false, TEST_I165C_GetImdInfo(&canMessage));

    /* Test if CAN data indicated that iso165c is not initialized */
    for (uint8_t i = 0u; i < 8u; i++) {
        canMessage.data[i] = 0u;
    }
    TEST_ASSERT_EQUAL(false, TEST_I165C_IsSelfTestFinished(canMessage));

    /* Test if CAN data indicated that iso165c is not initialized */
    for (uint8_t i = 0u; i < 8u; i++) {
        canMessage.data[i] = 0x00u;
    }
    /*
     * D_IMC_STATUS is located in byte 2 and byte 3 of CAN message:
     * Bit 4: Self test running -> set to 1 */
    canMessage.data[2u] |= (1u << I165C_SELFTEST_RUNNING_SHIFT);
    /* D_VIFC_STATUS is located in byte 4 and byte 5 of CAN message
     * Bit 0: Insulation measurement active -> set to 0
     * Bit 12: Self-test long executed  -> set to 0
     * Bit 13: Self-test short executed -> set to 0
     */
    canMessage.data[4u] |= (0u << I165C_INSULATION_MEASUREMENT_STATUS_SHIFT);
    canMessage.data[5u] |=
        (0u << (I165C_IMC_SELFTEST_OVERALL_SCENARIO_SHIFT - 8u)); /* Subtract 8 because upper byte is used */
    canMessage.data[5u] |=
        (0u << (I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO_SHIFT - 8u)); /* Subtract 8 because upper byte is used */

    TEST_ASSERT_EQUAL(true, TEST_I165C_IsSelfTestFinished(canMessage));

    /* ----------- Test function that waits for acknowledge -----------------*/

    /* Check for invalid function parameters */
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckAcknowledgeArrived(command, NULL_PTR, NULL_PTR));
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckAcknowledgeArrived(command, NULL_PTR, &canMessage));
    TEST_ASSERT_FAIL_ASSERT(TEST_I165C_CheckAcknowledgeArrived(command, &tries, NULL_PTR));

    /* Acknowledge arrived */
    canMessage.id      = CANRX_IMD_RESPONSE_ID;
    tries              = 0u;
    command            = 0xA;
    canMessage.data[0] = 0xA;
    bool ackReceived   = false;

    ackReceived = TEST_I165C_CheckAcknowledgeArrived(command, &tries, &canMessage);
    TEST_ASSERT_EQUAL(true, ackReceived);
    TEST_ASSERT_EQUAL(0u, tries);

    /* Acknowledge not arrived, increment try counter */
    canMessage.id      = CANRX_IMD_RESPONSE_ID;
    tries              = 0u;
    command            = 0xA;
    canMessage.data[0] = 0xB;

    ackReceived = TEST_I165C_CheckAcknowledgeArrived(command, &tries, &canMessage);
    TEST_ASSERT_EQUAL(1u, tries);
    TEST_ASSERT_EQUAL(false, ackReceived);

    /* Acknowledge not arrived, and allowed number of tries made, restart
     * Initialization (go to self test) */
    tries              = I165C_TRANSMISSION_ATTEMPTS - 1u;
    command            = 0xA;
    canMessage.data[0] = 0xB;

    ackReceived = TEST_I165C_CheckAcknowledgeArrived(command, &tries, &canMessage);
    TEST_ASSERT_EQUAL(I165C_TRANSMISSION_ATTEMPTS, tries);
    TEST_ASSERT_EQUAL(false, ackReceived);
}
