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
 * @file    test_can_cbs_tx_pack-state-estimation.c
 * @author  foxBMS Team
 * @date    2021-07-27 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
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

#include "database_cfg.h"

#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_pack-state-estimation.c")

TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
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
void testCAN_TxStateEstimationCharging(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_PACK_STATE_ESTIMATION_ID,
        .idType     = CANTX_PACK_STATE_ESTIMATION_ID_TYPE,
        .dlc        = 8u,
        .endianness = CANTX_PACK_STATE_ESTIMATION_ENDIANNESS,
    };
    uint8_t data[8] = {0};

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableSoc->minimumSoc_perc[s] = 71.2f;
        can_kShim.pTableSoc->maximumSoc_perc[s] = 74.2f;
        can_kShim.pTableSoe->minimumSoe_perc[s] = 74.6f;
        can_kShim.pTableSoe->maximumSoe_perc[s] = 78.1f;
        can_kShim.pTableSoe->minimumSoe_Wh[s]   = 19200 / BS_NR_OF_STRINGS;
    }

    DATA_Read2DataBlocks_IgnoreAndReturn(0u);
    /* System is currently charging */
    BMS_GetBatterySystemState_IgnoreAndReturn(BMS_CHARGING);
    /* All strings connected */
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        BMS_IsStringClosed_IgnoreAndReturn(true);
    }
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);
    BMS_GetBatterySystemState_IgnoreAndReturn(BMS_CHARGING);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);

    CANTX_PackStateEstimation(testMessage, data, NULL_PTR, &can_kShim);

    /** Values of:
     *  pack SOC: 74.2%
     *  pack SOE: 78.1%
     *  pack Energy: 19.2kWh
     *  pack SOH: 100.0%
     */
    TEST_ASSERT_EQUAL(0x73, data[0]);
    TEST_ASSERT_EQUAL(0xED, data[1]);
    TEST_ASSERT_EQUAL(0xE8, data[2]);
    TEST_ASSERT_EQUAL(0x2F, data[3]);
    TEST_ASSERT_EQUAL(0xA0, data[4]);
    TEST_ASSERT_EQUAL(0x00, data[5]);
    TEST_ASSERT_EQUAL(0x07, data[6]);
    TEST_ASSERT_EQUAL(0x80, data[7]);
}

void testCAN_TxStateEstimationDischarging(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_PACK_STATE_ESTIMATION_ID,
        .idType     = CANTX_PACK_STATE_ESTIMATION_ID_TYPE,
        .dlc        = 8u,
        .endianness = CANTX_PACK_STATE_ESTIMATION_ENDIANNESS,
    };

    uint8_t data[8] = {0};

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableSoc->minimumSoc_perc[s] = 74.2f;
        can_kShim.pTableSoc->maximumSoc_perc[s] = 78.2f;
        can_kShim.pTableSoe->minimumSoe_perc[s] = 78.1f;
        can_kShim.pTableSoe->maximumSoe_perc[s] = 83.1f;
        can_kShim.pTableSoe->minimumSoe_Wh[s]   = 19200 / BS_NR_OF_STRINGS;
    }

    DATA_Read2DataBlocks_IgnoreAndReturn(0u);
    /* System is currently charging */
    BMS_GetBatterySystemState_IgnoreAndReturn(BMS_DISCHARGING);
    /* All strings connected */
    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        BMS_IsStringClosed_IgnoreAndReturn(true);
    }
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);
    BMS_GetBatterySystemState_IgnoreAndReturn(BMS_DISCHARGING);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);

    CANTX_PackStateEstimation(testMessage, data, NULL_PTR, &can_kShim);

    /** Values of:
     *  pack SOC: 74.2%
     *  pack SOE: 78.1%
     *  pack Energy: 19.2kWh
     *  pack SOH: 100.0%
     */
    TEST_ASSERT_EQUAL(0x73, data[0]);
    TEST_ASSERT_EQUAL(0xED, data[1]);
    TEST_ASSERT_EQUAL(0xE8, data[2]);
    TEST_ASSERT_EQUAL(0x2F, data[3]);
    TEST_ASSERT_EQUAL(0xA0, data[4]);
    TEST_ASSERT_EQUAL(0x00, data[5]);
    TEST_ASSERT_EQUAL(0x07, data[6]);
    TEST_ASSERT_EQUAL(0x80, data[7]);
}
