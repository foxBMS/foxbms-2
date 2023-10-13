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
 * @file    test_can_1.c
 * @author  foxBMS Team
 * @date    2023-03-13 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the configuration validation functions and counter reset calculation in the the CAN driver
 * @details This file implements the test of the validation functions for tx message
 *          configuration with the first invalid configuration of message period
 *          and phase in dummy message 1.
 *          Both functions are tested to assert an error with invalid configuration.
 *          Also the test for the CAN_CalculateCounterResetValue function is
 *          implemented here. It is tested to calculate the correct value for
 *          the given dummy messages.
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
TEST_SOURCE_FILE("can.c")

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

#define TEST_CANTX_DUMMY_PERIOD_2_ms (30)
#define TEST_CANTX_DUMMY_PHASE_2_ms  (0)

#define TEST_CANTX_DUMMY_MESSAGE_2                                                   \
    {                                                                                \
        .id         = TEST_CANTX_ID_DUMMY,                                           \
        .dlc        = CAN_DEFAULT_DLC,                                               \
        .endianness = CAN_LITTLE_ENDIAN,                                             \
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,                                \
    },                                                                               \
    {                                                                                \
        .period = TEST_CANTX_DUMMY_PERIOD_2_ms, .phase = TEST_CANTX_DUMMY_PHASE_2_ms \
    }

#define TEST_CANTX_DUMMY_PERIOD_3_ms (70)
#define TEST_CANTX_DUMMY_PHASE_3_ms  (0)

#define TEST_CANTX_DUMMY_MESSAGE_3                                                   \
    {                                                                                \
        .id         = TEST_CANTX_ID_DUMMY,                                           \
        .dlc        = CAN_DEFAULT_DLC,                                               \
        .endianness = CAN_LITTLE_ENDIAN,                                             \
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,                                \
    },                                                                               \
    {                                                                                \
        .period = TEST_CANTX_DUMMY_PERIOD_3_ms, .phase = TEST_CANTX_DUMMY_PHASE_3_ms \
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

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues  = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor  = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire            = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest    = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &imd_canDataQueue,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor   = &can_tableCurrentSensor,
    .pTableOpenWire        = &can_tableOpenWire,
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
    {CAN_NODE_1, TEST_CANTX_DUMMY_MESSAGE_2, &TEST_CANTX_DummyCallback, NULL_PTR},
    {CAN_NODE_1, TEST_CANTX_DUMMY_MESSAGE_3, &TEST_CANTX_DummyCallback, NULL_PTR},
};

const CAN_RX_MESSAGE_TYPE_s can_rxMessages[] = {
    {CAN_NODE_1, TEST_CANRX_DUMMY_MESSAGE, &TEST_CANRX_DummyCallback},
};

const uint8_t can_txMessagesLength = sizeof(can_txMessages) / sizeof(can_txMessages[0]);
const uint8_t can_rxMessagesLength = sizeof(can_rxMessages) / sizeof(can_rxMessages[0]);

CAN_STATE_s *canTestState = NULL_PTR;

OS_QUEUE ftsk_dataQueue                = NULL_PTR;
OS_QUEUE ftsk_imdCanDataQueue          = NULL_PTR;
OS_QUEUE ftsk_canRxQueue               = NULL_PTR;
OS_QUEUE ftsk_canTxUnsentMessagesQueue = NULL_PTR;
volatile bool ftsk_allQueuesCreated    = false;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testCAN_ValidateConfiguredTxMessagePeriod(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_ValidateConfiguredTxMessagePeriod());
}

void testCAN_ValidateConfiguredTxMessagePhase(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_CAN_ValidateConfiguredTxMessagePhase());
}

void testCAN_CalculateCounterResetValue(void) {
    /* least common multiple of the test message periods is 3570ms, so with a
       tick of 10ms the expected counter reset value is 357 */
    TEST_ASSERT_EQUAL(357u, TEST_CAN_CalculateCounterResetValue());
}
