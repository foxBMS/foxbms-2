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
 * @file    test_can.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver
 * @details This file implements the test of the validation functions for tx message
 *          configuration with correct configuration for message period and phase.
 *          Both functions are tested to not assert an error with valid configuration.
 *          Invalid configurations are tested in test_can_1 and test_can_2.
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_can.h"
#include "Mockcan_cfg.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockftask.h"
#include "Mockimd.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockos.h"
#include "Mockpex.h"
#include "Mockqueue.h"
#include "Mocktest_can_mpu_prototype_queue_create_stub.h"

#include "version_cfg.h"

#include "can.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/* see src/app/driver/config/can_cfg_rx-message-definitions.h, but we omit
   this include here */
#define CANRX_NOT_PERIODIC (0u)

/* TX test case */
#define TEST_CANTX_ID_DUMMY        (0x001)
#define TEST_CANTX_DUMMY_PERIOD_ms (100)
#define TEST_CANTX_DUMMY_PHASE_ms  (0)

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

static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState           = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_MONITORING_s can_tableInsulation = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags            = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags            = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire           = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues       = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags            = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_SOC_s can_tableSoc                      = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                      = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_SOH_s can_tableSoh                      = {.header.uniqueId = DATA_BLOCK_ID_SOH};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest   = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &ftsk_imdCanDataQueue,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCurrentSensor   = &can_tableCurrentSensor,
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableMol             = &can_tableMolFlags,
    .pTableMsl             = &can_tableMslFlags,
    .pTableOpenWire        = &can_tableOpenWire,
    .pTablePackValues      = &can_tablePackValues,
    .pTableRsl             = &can_tableRslFlags,
    .pTableSoc             = &can_tableSoc,
    .pTableSoe             = &can_tableSoe,
    .pTableSof             = NULL_PTR,
    .pTableSoh             = &can_tableSoh,
    .pTableStateRequest    = &can_tableStateRequest,
};

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

CAN_BUFFER_ELEMENT_s dummyMessageBuffer = {
    .canNode = CAN_NODE_1,
    .id      = TEST_CANTX_ID_DUMMY,
    .idType  = CAN_STANDARD_IDENTIFIER_11_BIT,
    .data    = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};

const uint16_t numberOfRepetitionsToReset = (TEST_CANTX_DUMMY_PERIOD_ms / CAN_TICK_ms) - 1;

CAN_STATE_s *canTestState = NULL_PTR;

OS_QUEUE ftsk_dataQueue                = NULL_PTR;
OS_QUEUE ftsk_imdCanDataQueue          = NULL_PTR;
OS_QUEUE ftsk_canRxQueue               = NULL_PTR;
OS_QUEUE ftsk_canTxUnsentMessagesQueue = NULL_PTR;
volatile bool ftsk_allQueuesCreated    = false;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    canTestState = TEST_CAN_GetCANState();

    canTestState->periodicEnable = false;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        canTestState->currentSensorPresent[s]   = false;
        canTestState->currentSensorCCPresent[s] = false;
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
    TEST_ASSERT_FAIL_ASSERT(CAN_DataSend(NULL_PTR, 0u, CAN_STANDARD_IDENTIFIER_11_BIT, &data));
}

void testDataSendNullPointerAsData(void) {
    CAN_NODE_s node = {0};
    canIsTxMessagePending_IgnoreAndReturn(0u);
    canUpdateID_Ignore();
    canTransmit_IgnoreAndReturn(0u);
    TEST_ASSERT_FAIL_ASSERT(CAN_DataSend(&node, 0u, CAN_STANDARD_IDENTIFIER_11_BIT, NULL_PTR));
}

void testDataSendNoMessagePending(void) {
    CAN_NODE_s *pNode = CAN_NODE_2;
    uint8_t data      = 0;

    canIsTxMessagePending_IgnoreAndReturn(1u);

    for (uint8_t i = 0u; i < 32; i++) {
        TEST_ASSERT_EQUAL(STD_NOT_OK, CAN_DataSend(pNode, i, CAN_STANDARD_IDENTIFIER_11_BIT, &data));
    }
}

void testDataSendInvalidIdentifierType(void) {
    CAN_NODE_s *pNode                    = CAN_NODE_1;
    uint8_t data                         = 0;
    CAN_IDENTIFIER_TYPE_e identifierType = CAN_INVALID_TYPE;

    canIsTxMessagePending_IgnoreAndReturn(1u);
    TEST_ASSERT_FAIL_ASSERT(CAN_DataSend(pNode, TEST_CANTX_ID_DUMMY, identifierType, &data));
}

void testDataSendMessagePending(void) {
    CAN_NODE_s *pNode = CAN_NODE_1;
    uint8_t data      = 0;

    /* simulate first messageBox has pending message */
    canIsTxMessagePending_ExpectAndReturn(pNode->canNodeRegister, 1, 0u);
    canUpdateID_Expect(pNode->canNodeRegister, 1, 0x20040000u);
    canTransmit_ExpectAndReturn(pNode->canNodeRegister, 1, &data, 0u);
    TEST_ASSERT_EQUAL(STD_OK, CAN_DataSend(pNode, 0x001, CAN_STANDARD_IDENTIFIER_11_BIT, &data));

    /* simulate messageBox until the highest to have no pending messages */
    for (uint8_t messageBox = 1u; messageBox < (CAN_NR_OF_TX_MESSAGE_BOX - 1); messageBox++) {
        canIsTxMessagePending_ExpectAndReturn(pNode->canNodeRegister, messageBox, 1u);
    }
    /* last message box has message pending */
    canIsTxMessagePending_ExpectAndReturn(pNode->canNodeRegister, CAN_NR_OF_TX_MESSAGE_BOX - 1, 0u);
    canUpdateID_Expect(pNode->canNodeRegister, CAN_NR_OF_TX_MESSAGE_BOX - 1, 0x20040000u);
    canTransmit_ExpectAndReturn(pNode->canNodeRegister, CAN_NR_OF_TX_MESSAGE_BOX - 1, &data, 0u);
    TEST_ASSERT_EQUAL(STD_OK, CAN_DataSend(pNode, 0x001, CAN_STANDARD_IDENTIFIER_11_BIT, &data));
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
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* check state before */
        TEST_ASSERT_EQUAL(false, canTestState->currentSensorPresent[s]);

        TEST_ASSERT_EQUAL(false, CAN_IsCurrentSensorPresent(s));

        /* set state to true */
        canTestState->currentSensorPresent[s] = true;

        /* check state again */
        TEST_ASSERT_EQUAL(true, canTestState->currentSensorPresent[s]);
    }
}

void testIsCurrentSensorCcPresent(void) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* check state before */
        TEST_ASSERT_EQUAL(false, canTestState->currentSensorCCPresent[s]);
        TEST_ASSERT_EQUAL(false, CAN_IsCurrentSensorCcPresent(s));

        /* set state to true */
        canTestState->currentSensorCCPresent[s] = true;

        /* check state again */
        TEST_ASSERT_EQUAL(true, canTestState->currentSensorCCPresent[s]);
    }
}

void testCAN_ValidateConfiguredTxMessagePeriod(void) {
    TEST_ASSERT_PASS_ASSERT(TEST_CAN_ValidateConfiguredTxMessagePeriod());
}

void testCAN_ValidateConfiguredTxMessagePhase(void) {
    TEST_ASSERT_PASS_ASSERT(TEST_CAN_ValidateConfiguredTxMessagePhase());
}

void testCAN_CheckDatabaseNullPointer(void) {
    const CAN_SHIM_s can_shim_corrupted = {
        /* corrupted struct example */

        .pTableCellTemperature = &can_tableTemperatures,
        .pTableCellVoltage     = &can_tableCellVoltages,
        .pTableCurrentSensor   = &can_tableCurrentSensor,
        .pTableErrorState      = &can_tableErrorState,
        .pTableInsulation      = &can_tableInsulation,
        .pTableMinMax          = &can_tableMinimumMaximumValues,
        .pTableMol             = &can_tableMolFlags,
        .pTableMsl             = &can_tableMslFlags,
        .pTableOpenWire        = &can_tableOpenWire,
        .pTablePackValues      = &can_tablePackValues,
        .pTableRsl             = &can_tableRslFlags,
        .pTableSoc             = &can_tableSoc,
        .pTableSoe             = &can_tableSoe,
        .pTableSof             = NULL_PTR,
        .pTableSoh             = &can_tableSoh,
        .pTableStateRequest    = &can_tableStateRequest,
    };

    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_CheckDatabaseNullPointer(can_shim_corrupted));
}

void testCAN_PeriodicTransmitQueueFull(void) {
    /* Assume no messages in queue */
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_FAIL);

    /* assume all message boxes are full */
    canIsTxMessagePending_IgnoreAndReturn(1u);
    MATH_MinimumOfTwoUint8_t_IgnoreAndReturn(CAN_MAX_DLC);

    /* expect a message to be sent to queue */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_canTxUnsentMessagesQueue, (void *)&dummyMessageBuffer, 0u, OS_FAIL);
    /* sending to queue failed, expect error sent to diag */
    DIAG_Handler_ExpectAndReturn(DIAG_ID_CAN_TX_QUEUE_FULL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);

    /* test call */
    TEST_CAN_PeriodicTransmit();

    /* call repeatedly until phase is zero again */
    for (uint16_t i = 0; i < numberOfRepetitionsToReset; i++) {
        TEST_CAN_PeriodicTransmit();
    }
}

void testCAN_PeriodicTransmitQueueHasSpace(void) {
    /* Assume no messages in queue */
    OS_ReceiveFromQueue_IgnoreAndReturn(OS_FAIL);

    /* assume all message boxes are full */
    canIsTxMessagePending_IgnoreAndReturn(1u);
    MATH_MinimumOfTwoUint8_t_IgnoreAndReturn(CAN_MAX_DLC);

    /* expect a message to be sent to queue */
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_canTxUnsentMessagesQueue, (void *)&dummyMessageBuffer, 0u, OS_SUCCESS);
    /* sending to queue successful, expect no error */
    DIAG_Handler_ExpectAndReturn(DIAG_ID_CAN_TX_QUEUE_FULL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);

    /* test call */
    TEST_CAN_PeriodicTransmit();

    /* call repeatedly until phase is zero again */
    for (uint16_t i = 0; i < numberOfRepetitionsToReset; i++) {
        TEST_CAN_PeriodicTransmit();
    }
}

void testCAN_IsMessagePeriodElapsed(void) {
    /* Invalid messageIndex */
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_IsMessagePeriodElapsed(0u, UINT16_MAX));

    /* test case period not elapsed */
    TEST_ASSERT_EQUAL(false, TEST_CAN_IsMessagePeriodElapsed(1u, 0u));

    /* test case period elapsed */
    TEST_ASSERT_EQUAL(true, TEST_CAN_IsMessagePeriodElapsed(0u, 0u));
}

void testCAN_SendMessagesFromQueue(void) {
    CAN_BUFFER_ELEMENT_s dummyMessageBufferEmpty = {
        .canNode = NULL_PTR, .id = 0u, .idType = CAN_INVALID_TYPE, .data = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};

    /** only test the case of failing, because the case of successful call would
     * require actual (not mocked) call of OS_ReceiveFromQueue to write to message buffer */
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_canTxUnsentMessagesQueue, (void *)&dummyMessageBufferEmpty, 0u, OS_FAIL);

    /* test call */
    TEST_ASSERT_PASS_ASSERT(CAN_SendMessagesFromQueue());
}
