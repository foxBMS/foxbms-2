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
 * @file    test_can_cbs_rx_imd-info.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "bender_iso165c_cfg.h"
#include "database_cfg.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_rx_imd-info.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/imd/bender/iso165c/config")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
/* dummy data array filled with zero */
const uint8_t testCanDataZeroArray[CAN_MAX_DLC] = {0};

const CAN_NODE_s can_node1 = {
    .canNodeRegister = canREG1,
};

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
/* test assertions */
void testAssertion_CANRX_ImdInfo(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_MAX_11BIT_ID, /* invalid ID */
        .idType     = CANRX_IMD_INFO_ID_TYPE,
        .dlc        = CANRX_IMD_INFO_DLC,
        .endianness = CANRX_IMD_INFO_ENDIANNESS,
    };

    /* test invalid ID*/
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfo(testMessage, testCanDataZeroArray, &can_kShim));

    /* test invalid DLC*/
    testMessage.id  = CANRX_IMD_INFO_ID;
    testMessage.dlc = 9u;
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfo(testMessage, testCanDataZeroArray, &can_kShim));

    /* test invalid endianness*/
    testMessage.dlc        = CANRX_IMD_INFO_DLC;
    testMessage.endianness = CAN_BIG_ENDIAN;
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfo(testMessage, testCanDataZeroArray, &can_kShim));

    /* test can data is null pointer*/
    testMessage.endianness = CANRX_IMD_INFO_ENDIANNESS;
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfo(testMessage, NULL_PTR, &can_kShim));

    /* test can shim is null pointer*/
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfo(testMessage, testCanDataZeroArray, NULL_PTR));
}

void testAssertion_CANRX_TransferImdInfoMessageToCanBuffer(void) {
    CAN_BUFFER_ELEMENT_s testBuffer = {0};
    uint8_t testDlc                 = 9u;
    /* test dlc to big */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TransferImdInfoMessageToCanBuffer(testDlc, testCanDataZeroArray, &testBuffer));

    /* test can data is null pointer */
    testDlc = 6u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TransferImdInfoMessageToCanBuffer(testDlc, NULL_PTR, &testBuffer));

    /* test can buffer element is null pointer */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TransferImdInfoMessageToCanBuffer(testDlc, testCanDataZeroArray, NULL_PTR));
}

void testAssertion_CANRX_ImdInfoGetDataFromMessage(void) {
    /* test can data is null pointer*/
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfoGetDataFromMessage(NULL_PTR, &can_tableInsulation));

    /* test data table is null pointer*/
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfoGetDataFromMessage(testCanDataZeroArray, NULL_PTR));
}

void testAssertion_CANRX_ImdInfoCheckMeasurementMode(void) {
    /* test can data is null pointer */
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfoCheckMeasurementMode(NULL_PTR, I165C_ENABLE_MEASUREMENT));

    /* test measurement mode not defined */
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfoCheckMeasurementMode(testCanDataZeroArray, 2u));
}

void testAssertion_CANRX_ImdInfoHasSelfTestBeenExecuted(void) {
    /* test can data is null pointer */
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfoHasSelfTestBeenExecuted(NULL_PTR));
}

void testAssertion_CANRX_ImdInfoIsSelfTestFinished(void) {
    /* test can data is null pointer*/
    TEST_ASSERT_FAIL_ASSERT(CANRX_ImdInfoIsSelfTestFinished(NULL_PTR));
}

/* test functionality */
void test_CANRX_ImdInfo(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANRX_IMD_INFO_ID,
        .idType     = CANRX_IMD_INFO_ID_TYPE,
        .dlc        = CANRX_IMD_INFO_DLC,
        .endianness = CANRX_IMD_INFO_ENDIANNESS,
    };
    CAN_BUFFER_ELEMENT_s canMessage = {
        .canNode = I165C_CAN_NODE,
        .id      = testMessage.id,
        .idType  = testMessage.idType,
        .data    = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};
    MATH_MinimumOfTwoUint8_t_ExpectAndReturn(testMessage.dlc, CAN_MAX_DLC, testMessage.dlc);
    OS_SendToBackOfQueue_ExpectAndReturn(*(can_kShim.pQueueImd), (void *)&canMessage, 0u, OS_SUCCESS);
    CANRX_ImdInfo(testMessage, testCanDataZeroArray, &can_kShim);
}

void test_CANRX_TransferImdInfoMessageToCanBuffer(void) {
    CAN_BUFFER_ELEMENT_s testBuffer = {
        .canNode = I165C_CAN_NODE,
        .id      = CANRX_IMD_INFO_ID,
        .idType  = CANRX_IMD_INFO_ID_TYPE,
        .data    = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};
    uint8_t testDlc                      = CANRX_IMD_INFO_DLC;
    uint8_t testData[CANRX_IMD_INFO_DLC] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu};

    MATH_MinimumOfTwoUint8_t_ExpectAndReturn(testDlc, CAN_MAX_DLC, testDlc);
    TEST_CANRX_TransferImdInfoMessageToCanBuffer(testDlc, testData, &testBuffer);

    TEST_ASSERT_EQUAL(0x12u, testBuffer.data[0u]);
    TEST_ASSERT_EQUAL(0x34u, testBuffer.data[1u]);
    TEST_ASSERT_EQUAL(0x56u, testBuffer.data[2u]);
    TEST_ASSERT_EQUAL(0x78u, testBuffer.data[3u]);
    TEST_ASSERT_EQUAL(0x9Au, testBuffer.data[4u]);
    TEST_ASSERT_EQUAL(0xBCu, testBuffer.data[5u]);
    TEST_ASSERT_EQUAL(0x00u, testBuffer.data[6u]);
    TEST_ASSERT_EQUAL(0x00u, testBuffer.data[7u]);
}

void test_CANRX_ImdInfoGetDataFromMessage(void) {
    uint8_t const canData[CANRX_IMD_INFO_DLC] = {0x39u, 0x30u, 0x3Fu, 0u, 0x17u, 0x31u};
    CANRX_ImdInfoGetDataFromMessage(canData, &can_tableInsulation);

    /* measured resistance */
    TEST_ASSERT_EQUAL(12345u, can_tableInsulation.insulationResistance_kOhm);

    /* imc status */
    TEST_ASSERT_EQUAL(true, can_tableInsulation.dfIsCriticalResistanceDetected);
    TEST_ASSERT_EQUAL(true, can_tableInsulation.dfIsChassisFaultDetected);
    TEST_ASSERT_EQUAL(true, can_tableInsulation.dfIsDeviceErrorDetected);
    TEST_ASSERT_EQUAL(false, can_tableInsulation.areDeviceFlagsValid);
    TEST_ASSERT_EQUAL(true, can_tableInsulation.dfIsWarnableResistanceDetected);

    /* vifc status*/
    TEST_ASSERT_EQUAL(false, can_tableInsulation.isImdRunning);
    TEST_ASSERT_EQUAL(false, can_tableInsulation.dfIsMeasurementUpToDate);

    /* test if flags get reverted correctly */
    uint8_t const resetCanData[CANRX_IMD_INFO_DLC] = {0x11u, 0x27u, 0u, 0u, 0u, 0u};
    CANRX_ImdInfoGetDataFromMessage(resetCanData, &can_tableInsulation);

    /* measured resistance */
    TEST_ASSERT_EQUAL(10001, can_tableInsulation.insulationResistance_kOhm);

    /* imc status */
    TEST_ASSERT_EQUAL(false, can_tableInsulation.dfIsCriticalResistanceDetected);
    TEST_ASSERT_EQUAL(false, can_tableInsulation.dfIsChassisFaultDetected);
    TEST_ASSERT_EQUAL(false, can_tableInsulation.dfIsDeviceErrorDetected);
    TEST_ASSERT_EQUAL(true, can_tableInsulation.areDeviceFlagsValid);
    TEST_ASSERT_EQUAL(false, can_tableInsulation.dfIsWarnableResistanceDetected);

    /* vifc status*/
    TEST_ASSERT_EQUAL(true, can_tableInsulation.isImdRunning);
    TEST_ASSERT_EQUAL(true, can_tableInsulation.dfIsMeasurementUpToDate);
}

void test_CANRX_ImdInfoCheckMeasurementMode(void) {
    uint8_t const canData[CANRX_IMD_INFO_DLC] = {0u, 0u, 0u, 0u, 0x01u, 0u};
    uint8_t mode                              = I165C_ENABLE_MEASUREMENT;

    /* valid combination*/
    TEST_ASSERT_EQUAL(true, CANRX_ImdInfoCheckMeasurementMode(canData, mode));

    /* invalid combination */
    mode = I165C_DISABLE_MEASUREMENT;
    TEST_ASSERT_EQUAL(false, CANRX_ImdInfoCheckMeasurementMode(canData, mode));

    /* switch actual measurement mode to disabled */
    /* valid combination*/
    TEST_ASSERT_EQUAL(true, CANRX_ImdInfoCheckMeasurementMode(testCanDataZeroArray, mode));

    /* invalid combination */
    mode = I165C_ENABLE_MEASUREMENT;
    TEST_ASSERT_EQUAL(false, CANRX_ImdInfoCheckMeasurementMode(testCanDataZeroArray, mode));
}

void test_CANRX_ImdInfoHasSelfTestBeenExecuted(void) {
    /* overall self test executed */
    uint8_t canData[CANRX_IMD_INFO_DLC] = {0u, 0u, 0u, 0u, 0u, 0x20u};
    TEST_ASSERT_EQUAL(true, CANRX_ImdInfoHasSelfTestBeenExecuted(canData));

    /* parameter config self test executed*/
    canData[5u] = 0x10u;
    TEST_ASSERT_EQUAL(true, CANRX_ImdInfoHasSelfTestBeenExecuted(canData));

    /* no self test executed */
    canData[5u] = 0x30u;
    TEST_ASSERT_EQUAL(false, CANRX_ImdInfoHasSelfTestBeenExecuted(canData));
}

void test_CANRX_ImdInfoIsSelfTestFinished(void) {
    /* self test is running */
    uint8_t canData[CANRX_IMD_INFO_DLC] = {0u, 0u, 0x10u, 0u, 0u, 0u};
    TEST_ASSERT_EQUAL(false, CANRX_ImdInfoIsSelfTestFinished(canData));

    /* insulation measurement is active */
    canData[2u] = 0u;
    TEST_ASSERT_EQUAL(false, CANRX_ImdInfoIsSelfTestFinished(canData));
    canData[4u] = 0x01u;

    /* self test was not executed */
#ifdef I165C_SELF_TEST_LONG
    /* overall self test */
    canData[5u] = 0x10u;
    TEST_ASSERT_EQUAL(false, CANRX_ImdInfoIsSelfTestFinished(canData));
#else
    /* parameter config self test*/
    canData[5u] = 0x20u;
    TEST_ASSERT_EQUAL(false, CANRX_ImdInfoIsSelfTestFinished(canData));
#endif
    /* self test is finished*/
    canData[5u] = 0u;
    TEST_ASSERT_EQUAL(true, CANRX_ImdInfoIsSelfTestFinished(canData));
}
