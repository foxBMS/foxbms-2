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
 * @file    test_can_cbs_rx_cs_isabellenhuette-ivt-s.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
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
TEST_SOURCE_FILE("can_cbs_rx_cs_isabellenhuette-ivt-s.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

#define CANRX_CURRENT_SENSOR_ID_START_BIT                (7u)
#define CANRX_CURRENT_SENSOR_ID_LENGTH                   (8u)
#define CANRX_CURRENT_SENSOR_SYSTEM_ERROR_START_BIT      (15u)
#define CANRX_CURRENT_SENSOR_SYSTEM_ERROR_LENGTH         (CAN_BIT)
#define CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_START_BIT (14u)
#define CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_LENGTH    (CAN_BIT)
#define CANRX_CURRENT_SENSOR_CHANNEL_ERROR_START_BIT     (13u)
#define CANRX_CURRENT_SENSOR_CHANNEL_ERROR_LENGTH        (CAN_BIT)
#define CANRX_CURRENT_SENSOR_OCS_START_BIT               (12u)
#define CANRX_CURRENT_SENSOR_OCS_LENGTH                  (CAN_BIT)
#define CANRX_CURRENT_SENSOR_MESSAGE_COUNT_START_BIT     (11u)
#define CANRX_CURRENT_SENSOR_MESSAGE_COUNT_LENGTH        (4u)
#define CANRX_CURRENT_SENSOR_DATA_START_BIT              (23u)
#define CANRX_CURRENT_SENSOR_DATA_LENGTH                 (32u)

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues  = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_s can_tableCurrent               = {.header.uniqueId = DATA_BLOCK_ID_CURRENT};
static DATA_BLOCK_CURRENT_SENSOR_TEMPERATURE_s can_tableSensorTemperature = {
    .header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR_TEMPERATURE};
static DATA_BLOCK_POWER_s can_tablePower                          = {.header.uniqueId = DATA_BLOCK_ID_POWER};
static DATA_BLOCK_CURRENT_COUNTER_s can_tableSensorCurrentCounter = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_COUNTER};
static DATA_BLOCK_ENERGY_COUNTER_s can_tableSensorEnergyCounter   = {.header.uniqueId = DATA_BLOCK_ID_ENERGY_COUNTER};
static DATA_BLOCK_SYSTEM_VOLTAGE_1_s can_tableSystemVoltage1      = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_VOLTAGE_1};
static DATA_BLOCK_SYSTEM_VOLTAGE_2_s can_tableSystemVoltage2      = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_VOLTAGE_2};
static DATA_BLOCK_SYSTEM_VOLTAGE_3_s can_tableSystemVoltage3      = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_VOLTAGE_3};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire                   = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest           = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues               = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                              = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                              = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                              = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState               = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_s can_tableInsulation                = {.header.uniqueId = DATA_BLOCK_ID_INSULATION};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags                    = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags                    = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags                    = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_AEROSOL_SENSOR_s can_tableAerosolSensor         = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd                      = &imd_canDataQueue,
    .pTableCellVoltage              = &can_tableCellVoltages,
    .pTableCellTemperature          = &can_tableTemperatures,
    .pTableMinMax                   = &can_tableMinimumMaximumValues,
    .pTableCurrent                  = &can_tableCurrent,
    .pTableCurrentSensorTemperature = &can_tableSensorTemperature,
    .pTablePower                    = &can_tablePower,
    .pTableCurrentCounter           = &can_tableSensorCurrentCounter,
    .pTableEnergyCounter            = &can_tableSensorEnergyCounter,
    .pTableSystemVoltage1           = &can_tableSystemVoltage1,
    .pTableSystemVoltage2           = &can_tableSystemVoltage2,
    .pTableSystemVoltage3           = &can_tableSystemVoltage3,
    .pTableOpenWire                 = &can_tableOpenWire,
    .pTableStateRequest             = &can_tableStateRequest,
    .pTablePackValues               = &can_tablePackValues,
    .pTableSof                      = &can_tableSof,
    .pTableSoc                      = &can_tableSoc,
    .pTableSoe                      = &can_tableSoe,
    .pTableErrorState               = &can_tableErrorState,
    .pTableInsulation               = &can_tableInsulation,
    .pTableMsl                      = &can_tableMslFlags,
    .pTableRsl                      = &can_tableRslFlags,
    .pTableMol                      = &can_tableMolFlags,
    .pTableAerosolSensor            = &can_tableAerosolSensor,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCANRX_HandleChannelError(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testMessageId    = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID;
    /* Assertion tests */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleChannelError(NULL_PTR, testMessageId, validStringNumber));
    uint32_t invalidTooLowMessageId = CANRX_BMS_STATE_REQUEST_ID;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleChannelError(&can_kShim, invalidTooLowMessageId, validStringNumber));
    uint32_t invalidTooHighMessageId = 0x999u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleChannelError(&can_kShim, invalidTooHighMessageId, validStringNumber));
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, invalidStringNumber));

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID */
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID;
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID;
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID;
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);

    /* case  CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID;
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);

    /* case  CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID;
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID;
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID;
    TEST_CANRX_HandleChannelError(&can_kShim, testMessageId, validStringNumber);
}

void testCANRX_ResetError(void) {
    uint8_t validStringNumber   = 0u;
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;

    /* Assertion tests */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ResetError(NULL_PTR, validStringNumber));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ResetError(&can_kShim, invalidStringNumber));

    TEST_CANRX_ResetError(&can_kShim, validStringNumber);
}

void testCANRX_HandleMeasurementError(void) {
    uint8_t validStringNumber   = 0u;
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;

    /* Assertion tests */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleMeasurementError(NULL_PTR, validStringNumber));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleMeasurementError(&can_kShim, invalidStringNumber));

    TEST_CANRX_HandleMeasurementError(&can_kShim, validStringNumber);
}

void testCANRX_HandleSystemError(void) {
    uint8_t validStringNumber   = 0u;
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;

    /* Assertion tests */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleSystemError(NULL_PTR, validStringNumber));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleSystemError(&can_kShim, invalidStringNumber));

    TEST_CANRX_HandleSystemError(&can_kShim, validStringNumber);
}

void testCANRX_HandleSensorData(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testMessageId    = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleSensorData(NULL_PTR, testMessageId, validStringNumber, testSignalData));
    uint32_t invalidTooLowMessageId = CANRX_BMS_STATE_REQUEST_ID;
    /* ======= AT2/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANRX_HandleSensorData(&can_kShim, invalidTooLowMessageId, validStringNumber, testSignalData));
    uint32_t invalidTooHighMessageId = 0x999u;
    /* ======= AT3/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANRX_HandleSensorData(&can_kShim, invalidTooHighMessageId, validStringNumber, testSignalData));
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    /* ======= AT4/4 ======= */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrent, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableSystemVoltage1, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableSystemVoltage2, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableSystemVoltage3, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);

    /* case  CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrentSensorTemperature, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);

    /* case  CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTablePower, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrentCounter, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);

    /* case CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID;
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableEnergyCounter, STD_OK);
    TEST_CANRX_HandleSensorData(&can_kShim, testMessageId, validStringNumber, testSignalData);
}

void testCANRX_SetCurrent(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetCurrent(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetCurrent(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrent, STD_OK);
    TEST_CANRX_SetCurrent(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetVoltageU1(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetVoltageU1(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetVoltageU1(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableSystemVoltage1, STD_OK);
    TEST_CANRX_SetVoltageU1(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetVoltageU2(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetVoltageU2(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetVoltageU2(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableSystemVoltage2, STD_OK);
    TEST_CANRX_SetVoltageU2(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetVoltageU3(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetVoltageU3(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetVoltageU3(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableSystemVoltage3, STD_OK);
    TEST_CANRX_SetVoltageU3(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetTemperature(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetTemperature(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetTemperature(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrentSensorTemperature, STD_OK);
    TEST_CANRX_SetTemperature(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetPower(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetPower(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetPower(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTablePower, STD_OK);
    TEST_CANRX_SetPower(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetCoulombCounting(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetCoulombCounting(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetCoulombCounting(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrentCounter, STD_OK);
    TEST_CANRX_SetCoulombCounting(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetEnergyCounting(void) {
    uint8_t validStringNumber = 0u;
    uint32_t testSignalData   = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetEnergyCounting(NULL_PTR, validStringNumber, testSignalData));
    /* ======= AT2/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetEnergyCounting(&can_kShim, invalidStringNumber, testSignalData));

    /* ======= Routine tests =============================================== */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableEnergyCounter, STD_OK);
    TEST_CANRX_SetEnergyCounting(&can_kShim, validStringNumber, testSignalData);
}

void testCANRX_SetStringNumberBasedOnCanMessageId(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    uint32_t invalidTooLowMessageId = CANRX_BMS_STATE_REQUEST_ID;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetStringNumberBasedOnCanMessageId(invalidTooLowMessageId));
    /* ======= AT2/2 ======= */
    uint32_t invalidTooHighMessageId = 0x999u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetStringNumberBasedOnCanMessageId(invalidTooHighMessageId));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2 ======= */
    uint32_t testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID;
    TEST_CANRX_SetStringNumberBasedOnCanMessageId(testMessageId);
    /* ======= RT2/2 ======= */
    testMessageId = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID + 1u;
    TEST_CANRX_SetStringNumberBasedOnCanMessageId(testMessageId);
}

void testCANRX_CsIsabellenhuetteIvtS(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID,
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID_TYPE,
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_DLC,
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ENDIANNESS,
    };
    uint8_t testCanData = 0;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testTooLowIdMessage = {
        .id = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID -
              1u, /* CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID is lowest id in valid range */
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID_TYPE,
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_DLC,
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ENDIANNESS,
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_CsIsabellenhuetteIvtS(testTooLowIdMessage, &testCanData, &can_kShim));
    /* ======= AT2/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testTooHighIdMessage = {
        .id = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID +
              1u, /* CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID is highest id in valid range */
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID_TYPE,
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_DLC,
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ENDIANNESS,
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_CsIsabellenhuetteIvtS(testTooHighIdMessage, &testCanData, &can_kShim));
    /* ======= AT3/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testInvalidIdTypeMessage = {
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID,
        .idType     = CAN_EXTENDED_IDENTIFIER_29_BIT, /* valid is CAN_STANDARD_IDENTIFIER_11_BIT */
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_DLC,
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ENDIANNESS,
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_CsIsabellenhuetteIvtS(testInvalidIdTypeMessage, &testCanData, &can_kShim));
    /* ======= AT4/7 ======= */
    CAN_MESSAGE_PROPERTIES_s testInvalidDlcMessage = {
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID,
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID_TYPE,
        .dlc        = CAN_DEFAULT_DLC, /* valid is CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC == 6u */
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ENDIANNESS,
    };
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_CsIsabellenhuetteIvtS(testInvalidDlcMessage, &testCanData, &can_kShim));

    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_CsIsabellenhuetteIvtS(testMessage, NULL_PTR, &can_kShim));
    /* ======= AT7/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_CsIsabellenhuetteIvtS(testMessage, &testCanData, NULL_PTR));

    /* ======= Routine tests =============================================== */
    uint64_t testMessageData = 0uLL;
    uint64_t testCanSignal   = 0uLL;

    /* ======= RT1/2 ======= */
    /* branches checking (canSignal != 0uLL) return false */
    CAN_RxGetMessageDataFromCanData_Expect(&testMessageData, &testCanData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_OCS_START_BIT,
        CANRX_CURRENT_SENSOR_OCS_LENGTH,
        &testCanSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_CHANNEL_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_CHANNEL_ERROR_LENGTH,
        &testCanSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_LENGTH,
        &testCanSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_SYSTEM_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_SYSTEM_ERROR_LENGTH,
        &testCanSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_DATA_START_BIT,
        CANRX_CURRENT_SENSOR_DATA_LENGTH,
        &testCanSignal,
        testMessage.endianness);

    /* Expects from CANRX_HandleSensorData call */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrent, STD_OK);

    CANRX_CsIsabellenhuetteIvtS(testMessage, &testCanData, &can_kShim);

    /* ======= RT2/2 ======= */
    /* branches checking (canSignal != 0uLL) return true */
    testMessageData             = 0uLL;
    testCanSignal               = 0uLL;
    uint64_t testSignalData[4u] = {0u, 1u, 2u, 3u};
    CAN_RxGetMessageDataFromCanData_Expect(&testMessageData, &testCanData, testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_OCS_START_BIT,
        CANRX_CURRENT_SENSOR_OCS_LENGTH,
        &testCanSignal,
        testMessage.endianness);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_CHANNEL_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_CHANNEL_ERROR_LENGTH,
        &testCanSignal,
        testMessage.endianness);
    /* canSignal != 0uLL */
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[1u]);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_MEASUREMENT_ERROR_LENGTH,
        &testSignalData[1u],
        testMessage.endianness);
    /* canSignal != 0uLL */
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[2u]);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_SYSTEM_ERROR_START_BIT,
        CANRX_CURRENT_SENSOR_SYSTEM_ERROR_LENGTH,
        &testSignalData[2u],
        testMessage.endianness);
    /* canSignal != 0uLL */
    CAN_RxGetSignalDataFromMessageData_ReturnThruPtr_pCanSignal(&testSignalData[3u]);
    CAN_RxGetSignalDataFromMessageData_Expect(
        testMessageData,
        CANRX_CURRENT_SENSOR_DATA_START_BIT,
        CANRX_CURRENT_SENSOR_DATA_LENGTH,
        &testSignalData[3u],
        testMessage.endianness);

    /* Expects from CANRX_HandleSensorData call */
    OS_GetTickCount_ExpectAndReturn(0u);
    DATA_Write1DataBlock_ExpectAndReturn(can_kShim.pTableCurrent, STD_OK);

    CANRX_CsIsabellenhuetteIvtS(testMessage, &testCanData, &can_kShim);
}
