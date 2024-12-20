/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_can_cbs_tx_string-state.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details Test functions:
 *          - test_CANTX_StringState
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockcan.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"
#include "Mocksys_mon.h"

#include "database_cfg.h"

#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_string-state.c")

TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-cyclic")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

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
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags                  = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags                  = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags                  = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_BALANCING_CONTROL_s can_tableBalancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

OS_QUEUE imd_canDataQueue = NULL_PTR;

const CAN_SHIM_s can_kShim = {
    .pQueueImd              = &imd_canDataQueue,
    .pTableCellVoltage      = &can_tableCellVoltages,
    .pTableCellTemperature  = &can_tableTemperatures,
    .pTableMinMax           = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor    = &can_tableCurrentSensor,
    .pTableOpenWire         = &can_tableOpenWire,
    .pTableStateRequest     = &can_tableStateRequest,
    .pTablePackValues       = &can_tablePackValues,
    .pTableSof              = &can_tableSof,
    .pTableSoc              = &can_tableSoc,
    .pTableSoe              = &can_tableSoe,
    .pTableErrorState       = &can_tableErrorState,
    .pTableInsulation       = &can_tableInsulation,
    .pTableMsl              = &can_tableMslFlags,
    .pTableRsl              = &can_tableRslFlags,
    .pTableMol              = &can_tableMolFlags,
    .pTableBalancingControl = &can_tableBalancingControl,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void test_CANTX_StringState(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_STRING_STATE_ID,
        .idType     = CANTX_STRING_STATE_ID_TYPE,
        .dlc        = CANTX_STRING_STATE_DLC,
        .endianness = CANTX_STRING_STATE_ENDIANNESS,
    };
    uint8_t testCanData[CANTX_STRING_STATE_DLC] = {0u};

    /* configure data tables */
    /* String 0 */
    can_tableErrorState.stringFuseError[0u]            = true;
    can_tableErrorState.deepDischargeDetectedError[0u] = 0u;

    can_tableMslFlags.overtemperatureCharge[0u]      = 1u;
    can_tableMslFlags.undertemperatureCharge[0u]     = 0u;
    can_tableMslFlags.overtemperatureDischarge[0u]   = 1u;
    can_tableMslFlags.undertemperatureDischarge[0u]  = 0u;
    can_tableMslFlags.cellChargeOvercurrent[0u]      = 1u;
    can_tableMslFlags.stringChargeOvercurrent[0u]    = 0u;
    can_tableMslFlags.cellDischargeOvercurrent[0u]   = 1u;
    can_tableMslFlags.stringDischargeOvercurrent[0u] = 0u;
    can_tableMslFlags.overVoltage[0u]                = 1u;
    can_tableMslFlags.underVoltage[0u]               = 0u;

    can_tableMolFlags.overtemperatureCharge[0u]      = 1u;
    can_tableMolFlags.undertemperatureCharge[0u]     = 0u;
    can_tableMolFlags.overtemperatureDischarge[0u]   = 1u;
    can_tableMolFlags.undertemperatureDischarge[0u]  = 0u;
    can_tableMolFlags.cellChargeOvercurrent[0u]      = 1u;
    can_tableMolFlags.stringChargeOvercurrent[0u]    = 0u;
    can_tableMolFlags.cellDischargeOvercurrent[0u]   = 1u;
    can_tableMolFlags.stringDischargeOvercurrent[0u] = 0u;
    can_tableMolFlags.overVoltage[0u]                = 1u;
    can_tableMolFlags.underVoltage[0u]               = 0u;

    can_tableRslFlags.overtemperatureCharge[0u]      = 1u;
    can_tableRslFlags.undertemperatureCharge[0u]     = 0u;
    can_tableRslFlags.overtemperatureDischarge[0u]   = 1u;
    can_tableRslFlags.undertemperatureDischarge[0u]  = 0u;
    can_tableRslFlags.cellChargeOvercurrent[0u]      = 1u;
    can_tableRslFlags.stringChargeOvercurrent[0u]    = 0u;
    can_tableRslFlags.cellDischargeOvercurrent[0u]   = 1u;
    can_tableRslFlags.stringDischargeOvercurrent[0u] = 0u;
    can_tableRslFlags.overVoltage[0u]                = 1u;
    can_tableRslFlags.underVoltage[0u]               = 0u;

    can_tableErrorState.contactorInPositivePathOfStringFeedbackError[0u] = true;
    can_tableErrorState.contactorInNegativePathOfStringFeedbackError[0u] = false;
    can_tableErrorState.afeCommunicationSpiError[0u]                     = true;
    can_tableErrorState.afeCommunicationCrcError[0u]                     = false;
    can_tableErrorState.afeCellVoltageInvalidError[0u]                   = true;
    can_tableErrorState.afeCellTemperatureInvalidError[0u]               = false;
    can_tableErrorState.currentMeasurementInvalidError[0u]               = 1u;
    can_tableErrorState.currentMeasurementTimeoutError[0u]               = 0u;
    can_tableErrorState.currentSensorCoulombCounterTimeoutError[0u]      = true;
    can_tableErrorState.currentSensorEnergyCounterTimeoutError[0u]       = false;
    can_tableErrorState.currentSensorVoltage1TimeoutError[0u]            = true;
    can_tableErrorState.currentSensorVoltage2TimeoutError[0u]            = false;
    can_tableErrorState.currentSensorVoltage3TimeoutError[0u]            = true;
    can_tableErrorState.openWireDetectedError[0u]                        = false;

    can_tableErrorState.plausibilityCheckCellTemperatureError[0u]       = true;
    can_tableErrorState.plausibilityCheckCellVoltageError[0u]           = false;
    can_tableErrorState.plausibilityCheckPackVoltageError[0u]           = true;
    can_tableErrorState.plausibilityCheckCellTemperatureSpreadError[0u] = false;
    can_tableErrorState.plausibilityCheckCellVoltageSpreadError[0u]     = true;

    /* String 1 */
    can_tableErrorState.stringFuseError[1u]            = false;
    can_tableErrorState.deepDischargeDetectedError[1u] = 1u;

    can_tableMslFlags.overtemperatureCharge[1u]      = 0u;
    can_tableMslFlags.undertemperatureCharge[1u]     = 1u;
    can_tableMslFlags.overtemperatureDischarge[1u]   = 0u;
    can_tableMslFlags.undertemperatureDischarge[1u]  = 1u;
    can_tableMslFlags.cellChargeOvercurrent[1u]      = 0u;
    can_tableMslFlags.stringChargeOvercurrent[1u]    = 1u;
    can_tableMslFlags.cellDischargeOvercurrent[1u]   = 0u;
    can_tableMslFlags.stringDischargeOvercurrent[1u] = 1u;
    can_tableMslFlags.overVoltage[1u]                = 0u;
    can_tableMslFlags.underVoltage[1u]               = 1u;

    can_tableMolFlags.overtemperatureCharge[1u]      = 0u;
    can_tableMolFlags.undertemperatureCharge[1u]     = 1u;
    can_tableMolFlags.overtemperatureDischarge[1u]   = 0u;
    can_tableMolFlags.undertemperatureDischarge[1u]  = 1u;
    can_tableMolFlags.cellChargeOvercurrent[1u]      = 0u;
    can_tableMolFlags.stringChargeOvercurrent[1u]    = 1u;
    can_tableMolFlags.cellDischargeOvercurrent[1u]   = 0u;
    can_tableMolFlags.stringDischargeOvercurrent[1u] = 1u;
    can_tableMolFlags.overVoltage[1u]                = 0u;
    can_tableMolFlags.underVoltage[1u]               = 1u;

    can_tableRslFlags.overtemperatureCharge[1u]      = 0u;
    can_tableRslFlags.undertemperatureCharge[1u]     = 1u;
    can_tableRslFlags.overtemperatureDischarge[1u]   = 0u;
    can_tableRslFlags.undertemperatureDischarge[1u]  = 1u;
    can_tableRslFlags.cellChargeOvercurrent[1u]      = 0u;
    can_tableRslFlags.stringChargeOvercurrent[1u]    = 1u;
    can_tableRslFlags.cellDischargeOvercurrent[1u]   = 0u;
    can_tableRslFlags.stringDischargeOvercurrent[1u] = 1u;
    can_tableRslFlags.overVoltage[1u]                = 0u;
    can_tableRslFlags.underVoltage[1u]               = 1u;

    can_tableErrorState.contactorInPositivePathOfStringFeedbackError[1u] = false;
    can_tableErrorState.contactorInNegativePathOfStringFeedbackError[1u] = true;
    can_tableErrorState.afeCommunicationSpiError[1u]                     = false;
    can_tableErrorState.afeCommunicationCrcError[1u]                     = true;
    can_tableErrorState.afeCellVoltageInvalidError[1u]                   = false;
    can_tableErrorState.afeCellTemperatureInvalidError[1u]               = true;
    can_tableErrorState.currentMeasurementInvalidError[1u]               = 0u;
    can_tableErrorState.currentMeasurementTimeoutError[1u]               = 1u;
    can_tableErrorState.currentSensorCoulombCounterTimeoutError[1u]      = false;
    can_tableErrorState.currentSensorEnergyCounterTimeoutError[1u]       = true;
    can_tableErrorState.currentSensorVoltage1TimeoutError[1u]            = false;
    can_tableErrorState.currentSensorVoltage2TimeoutError[1u]            = true;
    can_tableErrorState.currentSensorVoltage3TimeoutError[1u]            = false;
    can_tableErrorState.openWireDetectedError[1u]                        = true;

    can_tableErrorState.plausibilityCheckCellTemperatureError[1u]       = false;
    can_tableErrorState.plausibilityCheckCellVoltageError[1u]           = true;
    can_tableErrorState.plausibilityCheckPackVoltageError[1u]           = false;
    can_tableErrorState.plausibilityCheckCellTemperatureSpreadError[1u] = true;
    can_tableErrorState.plausibilityCheckCellVoltageSpreadError[1u]     = false;

    /* test string 0 */
    uint8_t muxId = 0u;
    DATA_Read2DataBlocks_ExpectAndReturn(can_kShim.pTableRsl, can_kShim.pTableMol, STD_OK);
    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    CANTX_StringState(testMessage, testCanData, &muxId, &can_kShim);

    TEST_ASSERT_EQUAL(0x50u, testCanData[CAN_BYTE_0_POSITION]);
    TEST_ASSERT_EQUAL(0x75u, testCanData[CAN_BYTE_1_POSITION]);
    TEST_ASSERT_EQUAL(0x75u, testCanData[CAN_BYTE_2_POSITION]);
    TEST_ASSERT_EQUAL(0x75u, testCanData[CAN_BYTE_3_POSITION]);
    TEST_ASSERT_EQUAL(0x89u, testCanData[CAN_BYTE_4_POSITION]);
    TEST_ASSERT_EQUAL(0x58u, testCanData[CAN_BYTE_5_POSITION]);
    TEST_ASSERT_EQUAL(0xA9u, testCanData[CAN_BYTE_6_POSITION]);
    TEST_ASSERT_EQUAL(0x00u, testCanData[CAN_BYTE_7_POSITION]);

    /* test string 1 */
    muxId = 1u;
    BMS_IsStringClosed_ExpectAndReturn(1u, false);
    CANTX_StringState(testMessage, testCanData, &muxId, &can_kShim);

    TEST_ASSERT_EQUAL(0x81u, testCanData[CAN_BYTE_0_POSITION]);
    TEST_ASSERT_EQUAL(0xBAu, testCanData[CAN_BYTE_1_POSITION]);
    TEST_ASSERT_EQUAL(0xBAu, testCanData[CAN_BYTE_2_POSITION]);
    TEST_ASSERT_EQUAL(0xBAu, testCanData[CAN_BYTE_3_POSITION]);
    TEST_ASSERT_EQUAL(0x22u, testCanData[CAN_BYTE_4_POSITION]);
    TEST_ASSERT_EQUAL(0xAAu, testCanData[CAN_BYTE_5_POSITION]);
    TEST_ASSERT_EQUAL(0x52u, testCanData[CAN_BYTE_6_POSITION]);
    TEST_ASSERT_EQUAL(0x00u, testCanData[CAN_BYTE_7_POSITION]);
}
