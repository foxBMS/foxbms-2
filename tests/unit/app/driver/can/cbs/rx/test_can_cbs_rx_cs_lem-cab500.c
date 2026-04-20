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
 * @file    test_can_cbs_rx_cs_lem-cab500.c
 * @author  foxBMS Team
 * @date    2025-05-01 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockcan_helper.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_rx_cs_lem-cab500.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

#define CANRX_LEM_ZERO_AMPERE_VALUE (0x80000000)

#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_START_BIT (32u)
#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_LENGTH    (CAN_BIT)

#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_START_BIT (39u)
#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_LENGTH    (7u)

#define CANRX_CS_LEM_CAB500_STRING0_CURRENT_START_BIT (7u)
#define CANRX_CS_LEM_CAB500_STRING0_CURRENT_LENGTH    (32u)

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues  = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_s can_tableCurrent               = {.header.uniqueId = DATA_BLOCK_ID_CURRENT};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire            = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest    = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues        = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                       = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                       = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                       = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState        = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_s can_tableInsulation         = {.header.uniqueId = DATA_BLOCK_ID_INSULATION};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags             = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags             = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags             = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_AEROSOL_SENSOR_s can_tableAerosolSensor  = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &imd_canDataQueue,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableCurrent         = &can_tableCurrent,
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

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCANRX_LemResetError(void) {
    /* Fail Asserts */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemResetError(NULL_PTR, 0));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemResetError(&can_kShim, BS_NR_OF_STRINGS + 1u));

    /* Function test */
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableCurrent->invalidMeasurement[s] = 1u;
        TEST_CANRX_LemResetError(&can_kShim, s);
        TEST_ASSERT_EQUAL(0u, can_kShim.pTableCurrent->invalidMeasurement[s]);
    }
}

void testCANRX_LemHandleChannelError(void) {
    /* Fail Asserts */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemHandleChannelError(NULL_PTR, CANRX_CS_LEM_CAB500_STRING0_ID, 0u));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemHandleChannelError(&can_kShim, CANRX_CS_LEM_CAB500_STRING0_ID - 1u, 0u));
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANRX_LemHandleChannelError(&can_kShim, CANRX_CS_LEM_CAB500_STRING0_ID, BS_NR_OF_STRINGS));

    /* Function test */
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableCurrent->invalidMeasurement[s] = 0u;
        TEST_CANRX_LemHandleChannelError(&can_kShim, CANRX_CS_LEM_CAB500_STRING0_ID, s);
        TEST_ASSERT_EQUAL(1u, can_kShim.pTableCurrent->invalidMeasurement[s]);
    }
}

void testCANRX_LemSetStringNumberBasedOnCanMessageId(void) {
    /* Fail Asserts */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemSetStringNumberBasedOnCanMessageId(CANRX_CS_LEM_CAB500_STRING0_ID - 1u));

    /* Function test */
    TEST_ASSERT_EQUAL(0u, TEST_CANRX_LemSetStringNumberBasedOnCanMessageId(CANRX_CS_LEM_CAB500_STRING0_ID));
}

void testCANRX_LemSetCurrent(void) {
    /* Fail Asserts */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemSetCurrent(NULL_PTR, 0u, 0x4562u));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemSetCurrent(&can_kShim, BS_NR_OF_STRINGS, 0x4562u));

    /* Function test */
    int32_t signalData                  = 3456u;
    uint8_t s                           = 0u;
    can_kShim.pTableCurrent->newCurrent = 0u;

    for (; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableCurrent->current_mA[s]        = 0u;
        can_kShim.pTableCurrent->previousTimestamp[s] = 0u;
        can_kShim.pTableCurrent->timestamp[s]         = 1234u;

        OS_GetTickCount_ExpectAndReturn(2389u);
        TEST_CANRX_LemSetCurrent(&can_kShim, s, (uint32_t)(signalData + CANRX_LEM_ZERO_AMPERE_VALUE));

        TEST_ASSERT_EQUAL(signalData, can_kShim.pTableCurrent->current_mA[s]);
        TEST_ASSERT_EQUAL(1234u, can_kShim.pTableCurrent->previousTimestamp[s]);
        TEST_ASSERT_EQUAL(2389u, can_kShim.pTableCurrent->timestamp[s]);
    }

    TEST_ASSERT_EQUAL(s, can_kShim.pTableCurrent->newCurrent);
}

void testCANRX_LemHandleSensorData(void) {
    /* Fail Assert */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_LemHandleSensorData(NULL_PTR, CANRX_CS_LEM_CAB500_STRING0_ID, 0u, 0x4562u));
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANRX_LemHandleSensorData(&can_kShim, CANRX_CS_LEM_CAB500_STRING0_ID - 1, 0u, 0x4562u));
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANRX_LemHandleSensorData(&can_kShim, CANRX_CS_LEM_CAB500_STRING0_ID, BS_NR_OF_STRINGS, 0x4562u));

    /* Function test */
    int32_t signalData                  = 3456u;
    uint8_t s                           = 0u;
    can_kShim.pTableCurrent->newCurrent = 0u;

    for (; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableCurrent->current_mA[s] = 0u;
        OS_GetTickCount_ExpectAndReturn(2389u);
        TEST_CANRX_LemHandleSensorData(
            &can_kShim, CANRX_CS_LEM_CAB500_STRING0_ID, s, (uint32_t)(signalData + CANRX_LEM_ZERO_AMPERE_VALUE));

        TEST_ASSERT_EQUAL(signalData, can_kShim.pTableCurrent->current_mA[s]);
    }

    TEST_ASSERT_EQUAL(s, can_kShim.pTableCurrent->newCurrent);
}

void testCANRX_CsLemCab500(void) {
    uint8_t canData = 42u;
    /* Fail Assert */
    CAN_MESSAGE_PROPERTIES_s testMessageInvalidId = {
        .id         = 0x123u,                                 /* invalid message id */
        .idType     = CANRX_CS_LEM_CAB500_STRING0_ID_TYPE,    /* valid id type */
        .dlc        = CANRX_CS_LEM_CAB500_STRING0_DLC,        /* valid dlc */
        .endianness = CANRX_CS_LEM_CAB500_STRING0_ENDIANNESS, /* valid endianness */
    };

    TEST_ASSERT_FAIL_ASSERT(CANRX_CsLemCab500(testMessageInvalidId, &canData, &can_kShim));

    CAN_MESSAGE_PROPERTIES_s testMessageInvalidIdType = {
        .id         = CANRX_CS_LEM_CAB500_STRING0_ID,         /* valid message id */
        .idType     = CAN_INVALID_TYPE,                       /* invalid id type */
        .dlc        = CANRX_CS_LEM_CAB500_STRING0_DLC,        /* valid dlc */
        .endianness = CANRX_CS_LEM_CAB500_STRING0_ENDIANNESS, /* valid endianness */
    };

    TEST_ASSERT_FAIL_ASSERT(CANRX_CsLemCab500(testMessageInvalidIdType, &canData, &can_kShim));

    CAN_MESSAGE_PROPERTIES_s testMessageInvalidDlc = {
        .id         = CANRX_CS_LEM_CAB500_STRING0_ID,         /* valid message id */
        .idType     = CANRX_CS_LEM_CAB500_STRING0_ID_TYPE,    /* valid id type */
        .dlc        = 0u,                                     /* invalid dlc */
        .endianness = CANRX_CS_LEM_CAB500_STRING0_ENDIANNESS, /* valid endianness */
    };

    TEST_ASSERT_FAIL_ASSERT(CANRX_CsLemCab500(testMessageInvalidDlc, &canData, &can_kShim));

    CAN_MESSAGE_PROPERTIES_s testMessageInvalidEndian = {
        .id         = CANRX_CS_LEM_CAB500_STRING0_ID,      /* valid message id */
        .idType     = CANRX_CS_LEM_CAB500_STRING0_ID_TYPE, /* valid id type */
        .dlc        = CANRX_CS_LEM_CAB500_STRING0_DLC,     /* valid dlc */
        .endianness = CAN_LITTLE_ENDIAN,                   /* invalid endianness */
    };

    TEST_ASSERT_FAIL_ASSERT(CANRX_CsLemCab500(testMessageInvalidEndian, &canData, &can_kShim));

    CAN_MESSAGE_PROPERTIES_s testMessageValid = {
        .id         = CANRX_CS_LEM_CAB500_STRING0_ID,         /* valid message id */
        .idType     = CANRX_CS_LEM_CAB500_STRING0_ID_TYPE,    /* valid id type */
        .dlc        = CANRX_CS_LEM_CAB500_STRING0_DLC,        /* valid dlc */
        .endianness = CANRX_CS_LEM_CAB500_STRING0_ENDIANNESS, /* valid endianness */
    };

    TEST_ASSERT_FAIL_ASSERT(CANRX_CsLemCab500(testMessageValid, NULL_PTR, &can_kShim));
    TEST_ASSERT_FAIL_ASSERT(CANRX_CsLemCab500(testMessageValid, &canData, NULL_PTR));

    /* ================================================================================= */

    uint64_t messageData = 0u;
    uint64_t canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, &canData, testMessageValid.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_START_BIT,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_LENGTH,
        &canSignal,
        testMessageValid.endianness);

    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CANRX_CS_LEM_CAB500_STRING0_CURRENT_START_BIT,
        CANRX_CS_LEM_CAB500_STRING0_CURRENT_LENGTH,
        &canSignal,
        testMessageValid.endianness);

    uint32_t signalData = (int32_t)canSignal;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableCurrent->current_mA[s] = 0u;
        OS_GetTickCount_ExpectAndReturn(2389u);

        TEST_ASSERT_EQUAL(signalData, can_kShim.pTableCurrent->current_mA[s]);
    }

    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrent, STD_OK);

    CANRX_CsLemCab500(testMessageValid, &canData, &can_kShim);

    /* === */

    messageData = 0;
    canSignal   = 0u;
    CAN_RxGetMessageDataFromCanData_Expect(&messageData, &canData, testMessageValid.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_START_BIT,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_LENGTH,
        &canSignal,
        testMessageValid.endianness);
    uint64_t canSignalNotNull = 1u;
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&canSignalNotNull);

    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_START_BIT,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_LENGTH,
        &canSignalNotNull,
        testMessageValid.endianness);

    CAN_RxGetSignalDataFromMessageData_Expect(
        messageData,
        CANRX_CS_LEM_CAB500_STRING0_CURRENT_START_BIT,
        CANRX_CS_LEM_CAB500_STRING0_CURRENT_LENGTH,
        &canSignalNotNull,
        testMessageValid.endianness);

    signalData = (int32_t)canSignal;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableCurrent->current_mA[s] = 0u;
        OS_GetTickCount_ExpectAndReturn(2389u);

        TEST_ASSERT_EQUAL(signalData, can_kShim.pTableCurrent->current_mA[s]);
    }

    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrent, STD_OK);

    CANRX_CsLemCab500(testMessageValid, &canData, &can_kShim);
}
