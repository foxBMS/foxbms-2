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
 * @file    test_can_cbs_rx_state-request.c
 * @author  foxBMS Team
 * @date    2021-07-28 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbal.h"
#include "Mockbal_cfg.h"
#include "Mockbms_cfg.h"
#include "Mockcan.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"
#include "Mocksys_mon.h"

#include "database_cfg.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

TEST_FILE("can_cbs_rx_state-request.c")

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest       = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues           = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                          = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOX_s can_tableSox                          = {.header.uniqueId = DATA_BLOCK_ID_SOX};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState           = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_MONITORING_s can_tableInsulation = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &imd_canDataQueue,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor   = &can_tableCurrentSensor,
    .pTableOpenWire        = &can_tableOpenWire,
    .pTableStateRequest    = &can_tableStateRequest,
    .pTablePackValues      = &can_tablePackValues,
    .pTableSof             = &can_tableSof,
    .pTableSox             = &can_tableSox,
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMsl             = &can_tableMslFlags,
    .pTableRsl             = &can_tableRslFlags,
    .pTableMol             = &can_tableMolFlags,
};

static uint8_t muxId = 0u;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    can_tableStateRequest.previousStateRequestViaCan = 0u;
    can_tableStateRequest.stateRequestViaCan         = 0u;
    can_tableStateRequest.stateRequestViaCanPending  = 0u;
    can_tableStateRequest.stateCounter               = 0u;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/** test the handling of illegal input by callback */
void testRxRequestIllegalInput(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_MAX_11BIT_ID,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = 0u,
        .endianness = CAN_LITTLE_ENDIAN,
    };

    uint8_t canData[CAN_MAX_DLC] = {0};

    /* test 0 */
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, canData, &can_kShim));

    /* test 1 */
    testMessage.id  = 0u;
    testMessage.dlc = CAN_DEFAULT_DLC + 1u;

    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, canData, &can_kShim));

    /* test 2 */
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, NULL_PTR, &can_kShim));

    /* test 3 */
    TEST_ASSERT_FAIL_ASSERT(CANRX_BmsStateRequest(testMessage, canData, NULL_PTR));
}

/** test mode request */
void testRxRequestModeRequest(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANRX_BMS_STATE_REQUEST_ID,
        .idType     = CANRX_BMS_STATE_REQUEST_ID_TYPE,
        .dlc        = CAN_FOXBMS_MESSAGES_DEFAULT_DLC,
        .endianness = CANRX_BMS_STATE_REQUEST_ENDIANNESS,
    };

    uint8_t canData[CAN_MAX_DLC] = {0};

    /* ignore reads that are not used in this test */
    DATA_Read1DataBlock_IgnoreAndReturn(STD_OK);
    BAL_GetInitializationState_IgnoreAndReturn(STD_NOT_OK);
    BAL_SetBalancingThreshold_Ignore();
    DATA_Write1DataBlock_IgnoreAndReturn(STD_OK);

    /* request disconnect (STANDBY) */
    canData[0u] = 0u;
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.stateRequestViaCan);

    /* request discharge (NORMAL) */
    canData[0u] = 1u;
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NORMAL, can_tableStateRequest.stateRequestViaCan);

    /* request charge (CHARGE) */
    canData[0u] = 2u;
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_CHARGE, can_tableStateRequest.stateRequestViaCan);

    /* no valid request */
    canData[0u] = 3u;
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_NOREQ, can_tableStateRequest.stateRequestViaCan);

    /* state counter overflow */
    can_tableStateRequest.stateCounter = UINT8_MAX;
    canData[0u]                        = 0u;
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
    TEST_ASSERT_EQUAL(0u, can_tableStateRequest.stateCounter);

    /* state: no update without change an no time */
    canData[0u] = 0u;
    OS_CheckTimeHasPassed_ExpectAndReturn(0u, 0u, false);
    OS_CheckTimeHasPassed_IgnoreArg_oldTimeStamp_ms();
    OS_CheckTimeHasPassed_IgnoreArg_timeToPass_ms();
    can_tableStateRequest.stateRequestViaCanPending = 42u;
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
    TEST_ASSERT_EQUAL(42u, can_tableStateRequest.stateRequestViaCanPending);

    /* state: update with change of time */
    canData[0u] = 0u;
    OS_CheckTimeHasPassed_ExpectAndReturn(0u, 0u, true);
    OS_CheckTimeHasPassed_IgnoreArg_oldTimeStamp_ms();
    OS_CheckTimeHasPassed_IgnoreArg_timeToPass_ms();
    can_tableStateRequest.stateRequestViaCanPending = 42u;
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
    TEST_ASSERT_EQUAL(BMS_REQ_ID_STANDBY, can_tableStateRequest.stateRequestViaCanPending);
}

/** test balancing request */
void testRxRequestBalancingRequest(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANRX_BMS_STATE_REQUEST_ID,
        .idType     = CANRX_BMS_STATE_REQUEST_ID_TYPE,
        .dlc        = CAN_FOXBMS_MESSAGES_DEFAULT_DLC,
        .endianness = CANRX_BMS_STATE_REQUEST_ENDIANNESS,
    };
    uint8_t canData[CAN_MAX_DLC] = {0};

    /* ignore reads that are not used in this test */
    DATA_Read1DataBlock_IgnoreAndReturn(STD_OK);
    DATA_Write1DataBlock_IgnoreAndReturn(STD_OK);
    OS_CheckTimeHasPassed_IgnoreAndReturn(false);

    /* request no balancing */
    canData[1u] = 0u;
    canData[2u] = 0u;
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_DISABLE_REQUEST, BAL_OK);
    BAL_SetBalancingThreshold_Expect(0u);
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);

    /* request balancing */
    canData[1u] = 1u;
    canData[2u] = 0x42;
    BAL_GetInitializationState_ExpectAndReturn(STD_OK);
    BAL_SetStateRequest_ExpectAndReturn(BAL_STATE_GLOBAL_ENABLE_REQUEST, BAL_OK);
    BAL_SetBalancingThreshold_Expect(0x42u);
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);

    /* balancing not initialized */
    canData[1u] = 1u;
    canData[2u] = 0x33u;
    BAL_GetInitializationState_ExpectAndReturn(STD_NOT_OK);
    BAL_SetBalancingThreshold_Expect(0x33u);
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
}

/** test reset flags request */
void testRxRequestResetFlags(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANRX_BMS_STATE_REQUEST_ID,
        .idType     = CANRX_BMS_STATE_REQUEST_ID_TYPE,
        .dlc        = CAN_FOXBMS_MESSAGES_DEFAULT_DLC,
        .endianness = CANRX_BMS_STATE_REQUEST_ENDIANNESS,
    };
    uint8_t canData[CAN_MAX_DLC] = {0};

    /* ignore reads that are not used in this test */
    DATA_Read1DataBlock_IgnoreAndReturn(STD_OK);
    DATA_Write1DataBlock_IgnoreAndReturn(STD_OK);
    OS_CheckTimeHasPassed_IgnoreAndReturn(false);
    BAL_SetBalancingThreshold_Ignore();
    BAL_GetInitializationState_IgnoreAndReturn(STD_NOT_OK);

    /* request to reset flags */
    canData[0u] = 4u;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        DIAG_Handler_ExpectAndReturn(
            DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_OK, DIAG_STRING, s, DIAG_HANDLER_RETURN_OK);
    }
    SYSM_ClearAllTimingViolations_Expect();
    CANRX_BmsStateRequest(testMessage, canData, &can_kShim);
}
