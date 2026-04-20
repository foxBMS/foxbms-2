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
 * @file    test_can_cbs_tx_f_cell-temperatures_1-temp-sensor.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
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
#include "Mockdatabase.h"
#include "Mockdatabase_helper.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_f_cell-temperatures.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/
/** the number of temperatures per message-frame */
#define CANTX_NUMBER_OF_MUX_TEMPERATURES_PER_MESSAGE (6u)

uint64_t testMessageData[14u] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u};

float_t testSignalData[3u] = {0u, 1u, 2u};

float_t testCellTemperature0 = 850.0f;
float_t testCellTemperature1 = 570.0f;

static const CAN_SIGNAL_TYPE_s cantx_testCell0Temperature_degC = {23u, 8u, 10.0f, 0.0f, -1280.0f, 1270.0f};

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
    can_kShim.pTableCellTemperature->invalidCellTemperature[0u][0u][0u] = 0u;
    can_kShim.pTableCellTemperature->invalidCellTemperature[0u][0u][1u] = 1u;

    can_kShim.pTableCellTemperature->cellTemperature_ddegC[0u][0u][0u] = (int16_t)testCellTemperature0;
    can_kShim.pTableCellTemperature->cellTemperature_ddegC[0u][0u][1u] = (int16_t)testCellTemperature1;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing CANTX_CellTemperatures
 * @details The following cases need to be tested:
 *          - Argument validation:
 *              - none
 *          - Routine validation:
 *            - RT1/1: Setting one temperature
 *
 */
void testCANTX_CellTemperatures(void) {
    /* BS_NR_OF_TEMP_SENSORS == 1 in this test */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = 0x260,
        .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
        .dlc        = 8u,
        .endianness = CAN_BIG_ENDIAN,
    };
    uint8_t testCanData[CAN_MAX_DLC] = {0u};
    uint8_t testMuxId                = 0u;
    /* ======= Routine tests =============================================== */
    /* Calculate the global cell ID based on the multiplexer value for the first cell */
    uint16_t temperatureSensorId = 0;

    /* ======= RT1/1: Test implementation */
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[0u], 7u, 8u, 0u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[1u]);

    DATA_GetStringNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetModuleNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    DATA_GetSensorNumberFromTemperatureIndex_ExpectAndReturn(temperatureSensorId, 0u);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[1u], 8u, 1u, 1u, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[2u]);
    CAN_TxPrepareSignalData_Expect(&testCellTemperature0, cantx_testCell0Temperature_degC);
    CAN_TxPrepareSignalData_ReturnThruPtr_pSignal(&testSignalData[1u]);
    CAN_TxSetMessageDataWithSignalData_Expect(&testMessageData[2u], 23u, 8u, testSignalData[1u], CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData_ReturnThruPtr_pMessage(&testMessageData[3u]);
    temperatureSensorId++;

    CAN_TxSetCanDataWithMessageData_Expect(testMessageData[3u], testCanData, CAN_BIG_ENDIAN);
    /* ======= RT1/1: Call function under test */
    uint32_t testResult = CANTX_CellTemperatures(testMessage, testCanData, &testMuxId, &can_kShim);
    /* ======= RT1/1: Test output verification */
    TEST_ASSERT_EQUAL(0u, testResult);
    TEST_ASSERT_EQUAL(1u, testMuxId);
}
