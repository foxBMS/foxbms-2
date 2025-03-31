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
 * @file    test_can_cbs_rx_cell-voltages.c
 * @author  foxBMS Team
 * @date    2023-08-31 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockdiag.h"
#include "Mockftask.h"
#include "Mockos.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_rx_cell-voltages.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
#define TEST_FULL_8_BITS  (255u)
#define TEST_FULL_64_BITS (pow(2, 64) - 1)

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest       = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues           = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                          = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                          = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                          = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState           = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_MONITORING_s can_tableInsulation = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags            = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags            = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags            = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_AEROSOL_SENSOR_s can_tableAerosolSensor = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};

OS_QUEUE imd_canDataQueue               = NULL_PTR;
OS_QUEUE ftsk_canToAfeCellVoltagesQueue = NULL_PTR;

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
    .pTableSoc             = &can_tableSoc,
    .pTableSoe             = &can_tableSoe,
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMsl             = &can_tableMslFlags,
    .pTableRsl             = &can_tableRslFlags,
    .pTableMol             = &can_tableMolFlags,
    .pTableAerosolSensor   = &can_tableAerosolSensor,
};

/**
 * CAN signal parameters for voltages
 */
static const uint8_t canrx_kCanCellVoltageMuxBitStart            = 7u;
static const uint8_t canrx_kCanCellVoltageMuxLength              = 8u;
static const uint8_t canrx_kCanCellVoltageInvalidFlagBitStart[4] = {12u, 13u, 14u, 15u};
static const uint8_t canrx_kCanCellVoltageInvalidFlagLength      = 1u;
static const uint8_t canrx_kCanCellVoltageBitStart[4]            = {11u, 30u, 33u, 52u};
static const uint8_t canrx_kCanCellVoltageLength                 = 13u;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing externalized static function
 *          CANRX_GetCanAfeCellVoltagesFromMessage
 * @details The following cases need to be tested:
 *          - Routine validation:
 *            - RT1/1 check if the extracted test_canCellVoltages is right;
 */
void testCANRX_GetCanAfeCellVoltagesFromMessage(void) {
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1 =======  */
    CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s test_canCellVoltages = {0};

    /* Prepare a uint64_t test message that matches the can message content */
    uint64_t testMessage          = (uint64_t)TEST_FULL_64_BITS;
    uint64_t pCanSignal           = 0;
    uint64_t pCanSignal_full8bits = (uint64_t)TEST_FULL_8_BITS;
    uint64_t pCanSignal_true      = (uint64_t)true;

    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessage,
        canrx_kCanCellVoltageMuxBitStart,
        canrx_kCanCellVoltageMuxLength,
        &pCanSignal,
        CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&pCanSignal_full8bits);

    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignal = 0;
        CAN_RxGetSignalDataFromMessageData_Expect(
            testMessage,
            canrx_kCanCellVoltageInvalidFlagBitStart[i],
            canrx_kCanCellVoltageInvalidFlagLength,
            &pCanSignal,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
        CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&pCanSignal_true);
    }

    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignal = 0;
        CAN_RxGetSignalDataFromMessageData_Expect(
            testMessage,
            canrx_kCanCellVoltageBitStart[i],
            canrx_kCanCellVoltageLength,
            &pCanSignal,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
        CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&pCanSignal_full8bits);
    }
    TEST_CANRX_GetCanAfeCellVoltagesFromMessage(&test_canCellVoltages, testMessage);
    TEST_ASSERT_EQUAL_UINT8(TEST_FULL_8_BITS, test_canCellVoltages.muxValue);
    TEST_ASSERT_EQUAL(true, test_canCellVoltages.invalidFlag[0]);
    TEST_ASSERT_EQUAL(true, test_canCellVoltages.invalidFlag[1]);
    TEST_ASSERT_EQUAL(true, test_canCellVoltages.invalidFlag[2]);
    TEST_ASSERT_EQUAL(true, test_canCellVoltages.invalidFlag[3]);
    TEST_ASSERT_EQUAL_UINT8(TEST_FULL_8_BITS, test_canCellVoltages.cellVoltage[0]);
    TEST_ASSERT_EQUAL_UINT8(TEST_FULL_8_BITS, test_canCellVoltages.cellVoltage[1]);
    TEST_ASSERT_EQUAL_UINT8(TEST_FULL_8_BITS, test_canCellVoltages.cellVoltage[2]);
    TEST_ASSERT_EQUAL_UINT8(TEST_FULL_8_BITS, test_canCellVoltages.cellVoltage[3]);
}

/**
 * @brief   Testing CANRX_CellVoltages
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/5: invalid message id for message -> assert
 *            - AT2/5: invalid message idType for message -> assert
 *            - AT3/5: invalid message DLC for message -> assert
 *            - AT4/5: NULL_PTR for kpkCanData -> assert
 *            - AT5/5: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/2: Function calls expected subroutines
 *            - RT2/2: Function calls expected subroutines
 */
void testCANRX_CellVoltages(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t canData[CAN_MAX_DLC] = {0};

    /* ======= AT1/5 ======= */
    CAN_MESSAGE_PROPERTIES_s testMessageInvalidId = {
        .id         = CAN_MAX_11BIT_ID,                   /* invalid message id */
        .idType     = CANRX_AFE_CELL_VOLTAGES_ID_TYPE,    /* valid id type */
        .dlc        = CAN_DEFAULT_DLC,                    /* valid dlc */
        .endianness = CANRX_AFE_CELL_VOLTAGES_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_CellVoltages(testMessageInvalidId, canData, &can_kShim));

    /* ======= AT2/5 ======= */
    CAN_MESSAGE_PROPERTIES_s testMessageInvalidIdType = {
        .id         = CANRX_AFE_CELL_VOLTAGES_ID,         /* valid message id */
        .idType     = CAN_EXTENDED_IDENTIFIER_29_BIT,     /* invalid id type */
        .dlc        = CAN_DEFAULT_DLC,                    /* valid dlc */
        .endianness = CANRX_AFE_CELL_VOLTAGES_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_CellVoltages(testMessageInvalidIdType, canData, &can_kShim));

    /* ======= AT3/5 ======= */
    CAN_MESSAGE_PROPERTIES_s testMessageInvalidDlc = {
        .id         = CANRX_AFE_CELL_VOLTAGES_ID,         /* valid message id */
        .idType     = CANRX_AFE_CELL_VOLTAGES_ID_TYPE,    /* valid id type */
        .dlc        = CAN_DEFAULT_DLC + 1u,               /* invalid dlc */
        .endianness = CANRX_AFE_CELL_VOLTAGES_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_CellVoltages(testMessageInvalidDlc, canData, &can_kShim));

    /* ======= AT4/5 ======= */
    CAN_MESSAGE_PROPERTIES_s validTestMessage = {
        .id         = CANRX_AFE_CELL_VOLTAGES_ID,         /* valid message id */
        .idType     = CANRX_AFE_CELL_VOLTAGES_ID_TYPE,    /* valid id type */
        .dlc        = CAN_DEFAULT_DLC,                    /* valid dlc */
        .endianness = CANRX_AFE_CELL_VOLTAGES_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_CellVoltages(validTestMessage, NULL_PTR, &can_kShim));

    /* ======= AT5/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_CellVoltages(validTestMessage, canData, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2 =======*/
    uint64_t messageData           = 0u;
    uint64_t pCanSignalMuxValue    = 0u;
    uint64_t pCanSignalInvalidFlag = 0u;
    uint64_t pCanSignalVoltage     = 0u;
    CAN_ENDIANNESS_e canEndianness = CAN_BIG_ENDIAN;

    CAN_RxGetMessageDataFromCanData_Expect(&messageData, canData, canEndianness);
    /* Get the mux value */
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        canrx_kCanCellVoltageMuxBitStart,
        canrx_kCanCellVoltageMuxLength,
        &pCanSignalMuxValue,
        CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    /* Get the invalid flag */
    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignalInvalidFlag = 0u;
        CAN_RxGetSignalDataFromMessageData_Expect(
            messageData,
            canrx_kCanCellVoltageInvalidFlagBitStart[i],
            canrx_kCanCellVoltageInvalidFlagLength,
            &pCanSignalInvalidFlag,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    }
    /* Get the voltages */
    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignalInvalidFlag = 0u;
        CAN_RxGetSignalDataFromMessageData_Expect(
            messageData,
            canrx_kCanCellVoltageBitStart[i],
            canrx_kCanCellVoltageLength,
            &pCanSignalVoltage,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    }
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_canToAfeCellVoltagesQueue, (void *)&messageData, 0, OS_SUCCESS);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_CAN_RX_QUEUE_FULL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_EQUAL_INT16(0, CANRX_CellVoltages(validTestMessage, canData, &can_kShim));

    /* ======= RT2/2 =======*/
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, canData, canEndianness);
    /* Get the mux value */
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        canrx_kCanCellVoltageMuxBitStart,
        canrx_kCanCellVoltageMuxLength,
        &pCanSignalMuxValue,
        CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    /* Get the invalid flag */
    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignalInvalidFlag = 0u;
        CAN_RxGetSignalDataFromMessageData_Expect(
            messageData,
            canrx_kCanCellVoltageInvalidFlagBitStart[i],
            canrx_kCanCellVoltageInvalidFlagLength,
            &pCanSignalInvalidFlag,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    }
    /* Get the voltages */
    for (uint8_t i = 0; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
        pCanSignalInvalidFlag = 0u;
        CAN_RxGetSignalDataFromMessageData_Expect(
            messageData,
            canrx_kCanCellVoltageBitStart[i],
            canrx_kCanCellVoltageLength,
            &pCanSignalVoltage,
            CANRX_AFE_CELL_VOLTAGES_ENDIANNESS);
    }
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_canToAfeCellVoltagesQueue, (void *)&messageData, 0, OS_FAIL);
    DIAG_Handler_ExpectAndReturn(DIAG_ID_CAN_RX_QUEUE_FULL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_EQUAL_INT16(0, CANRX_CellVoltages(validTestMessage, canData, &can_kShim));
}
