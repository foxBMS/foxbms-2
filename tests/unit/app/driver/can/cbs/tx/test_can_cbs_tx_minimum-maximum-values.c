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
 * @file    test_can_cbs_tx_minimum-maximum-values.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
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

TEST_FILE("can_cbs_tx_minimum-maximum-values.c")

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
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCANTX_MinimumMaximumValuesAllStringsOpen(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_MINIMUM_MAXIMUM_VALUES_ID,
        .idType     = CANTX_MINIMUM_MAXIMUM_VALUES_ID_TYPE,
        .dlc        = 8u,
        .endianness = CANTX_MINIMUM_MAXIMUM_VALUES_ENDIANNESS,
    };
    uint8_t data[8] = {0};

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableMinMax->minimumCellVoltage_mV[s]    = 2000;
        can_kShim.pTableMinMax->maximumCellVoltage_mV[s]    = 3000;
        can_kShim.pTableMinMax->minimumTemperature_ddegC[s] = -150;
        can_kShim.pTableMinMax->maximumTemperature_ddegC[s] = 350;
    }

    DATA_Read1DataBlock_IgnoreAndReturn(0u);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(0u);
    CANTX_MinimumMaximumValues(testMessage, data, NULL_PTR, &can_kShim);

    /** Values of:
     *  minimum cell voltage: 2000mV
     *  maximum cell voltage: 3000mV
     *  minimum temperature: -15degC
     *  maximum temperature: 35degC
     */
    TEST_ASSERT_EQUAL(0x5D, data[0]);
    TEST_ASSERT_EQUAL(0xC1, data[1]);
    TEST_ASSERT_EQUAL(0xF4, data[2]);
    TEST_ASSERT_EQUAL(0x00, data[3]);
    TEST_ASSERT_EQUAL(0x00, data[4]);
    TEST_ASSERT_EQUAL(0x00, data[5]);
    TEST_ASSERT_EQUAL(0x23, data[6]);
    TEST_ASSERT_EQUAL(0xF1, data[7]);
}

void testCANTX_MinimumMaximumValuesAllStringsClosed(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_MINIMUM_MAXIMUM_VALUES_ID,
        .idType     = CANTX_MINIMUM_MAXIMUM_VALUES_ID_TYPE,
        .dlc        = 8u,
        .endianness = CANTX_MINIMUM_MAXIMUM_VALUES_ENDIANNESS,
    };
    uint8_t data[8] = {0};

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        can_kShim.pTableMinMax->minimumCellVoltage_mV[s]    = 2000 + s;
        can_kShim.pTableMinMax->maximumCellVoltage_mV[s]    = 3000 - s;
        can_kShim.pTableMinMax->minimumTemperature_ddegC[s] = -150 + s;
        can_kShim.pTableMinMax->maximumTemperature_ddegC[s] = 350 - s;
    }

    DATA_Read1DataBlock_IgnoreAndReturn(0u);
    BMS_GetNumberOfConnectedStrings_IgnoreAndReturn(BS_NR_OF_STRINGS);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        BMS_IsStringClosed_IgnoreAndReturn(true);
    }
    CANTX_MinimumMaximumValues(testMessage, data, NULL_PTR, &can_kShim);

    /** Values of:
     *  minimum cell voltage: 2000mV
     *  maximum cell voltage: 3000mV
     *  minimum temperature: -15degC
     *  maximum temperature: 35degC
     */
    TEST_ASSERT_EQUAL(0x5D, data[0]);
    TEST_ASSERT_EQUAL(0xC1, data[1]);
    TEST_ASSERT_EQUAL(0xF4, data[2]);
    TEST_ASSERT_EQUAL(0x00, data[3]);
    TEST_ASSERT_EQUAL(0x00, data[4]);
    TEST_ASSERT_EQUAL(0x00, data[5]);
    TEST_ASSERT_EQUAL(0x23, data[6]);
    TEST_ASSERT_EQUAL(0xF1, data[7]);
}
