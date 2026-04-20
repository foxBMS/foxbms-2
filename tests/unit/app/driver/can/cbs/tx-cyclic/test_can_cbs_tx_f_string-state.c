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
 * @file    test_can_cbs_tx_f_string-state.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details Test functions:
 *          - testCANTX_StringState
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
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_tx_f_string-state.c")

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

/**
 * Configuration of the signals
 */
#define CANTX_SIGNAL_MUX_STRING_START_BIT                                  (3u)
#define CANTX_SIGNAL_MUX_STRING_LENGTH                                     (4u)
#define CANTX_SIGNAL_STRING_DEEP_DISCHARGE_ERROR_START_BIT                 (7u)
#define CANTX_SIGNAL_STRING_DEEP_DISCHARGE_ERROR_LENGTH                    (CAN_BIT)
#define CANTX_SIGNAL_STRING_IS_STRING_FUSE_BLOWN_START_BIT                 (6u)
#define CANTX_SIGNAL_STRING_IS_STRING_FUSE_BLOWN_LENGTH                    (CAN_BIT)
#define CANTX_SIGNAL_STRING_IS_BALANCING_ACTIVE_START_BIT                  (5u)
#define CANTX_SIGNAL_STRING_IS_BALANCING_ACTIVE_LENGTH                     (CAN_BIT)
#define CANTX_SIGNAL_STRING_IS_STRING_CONNECTED_START_BIT                  (4u)
#define CANTX_SIGNAL_STRING_IS_STRING_CONNECTED_LENGTH                     (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_UNDERVOLTAGE_ERROR_START_BIT               (15u)
#define CANTX_SIGNAL_STRING_MSL_UNDERVOLTAGE_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_OVERVOLTAGE_ERROR_START_BIT                (14u)
#define CANTX_SIGNAL_STRING_MSL_OVERVOLTAGE_ERROR_LENGTH                   (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_DISCHARGE_ERROR_START_BIT (13u)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_DISCHARGE_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_CHARGE_ERROR_START_BIT    (12u)
#define CANTX_SIGNAL_STRING_MSL_CELL_OVERCURRENT_CHARGE_ERROR_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_DISCHARGE_ERROR_START_BIT (11u)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_DISCHARGE_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_DISCHARGE_ERROR_START_BIT  (10u)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_DISCHARGE_ERROR_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_CHARGE_ERROR_START_BIT    (9u)
#define CANTX_SIGNAL_STRING_MSL_UNDERTEMPERATURE_CHARGE_ERROR_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_CHARGE_ERROR_START_BIT     (8u)
#define CANTX_SIGNAL_STRING_MSL_OVERTEMPERATURE_CHARGE_ERROR_LENGTH        (CAN_BIT)

#define CANTX_SIGNAL_STRING_MOL_UNDERVOLTAGE_WARNING_START_BIT               (23u)
#define CANTX_SIGNAL_STRING_MOL_UNDERVOLTAGE_WARNING_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_OVERVOLTAGE_WARNING_START_BIT                (22u)
#define CANTX_SIGNAL_STRING_MOL_OVERVOLTAGE_WARNING_LENGTH                   (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_DISCHARGE_WARNING_START_BIT (21u)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_CHARGE_WARNING_START_BIT    (20u)
#define CANTX_SIGNAL_STRING_MOL_CELL_OVERCURRENT_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_DISCHARGE_WARNING_START_BIT (19u)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_DISCHARGE_WARNING_START_BIT  (18u)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_DISCHARGE_WARNING_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_CHARGE_WARNING_START_BIT    (17u)
#define CANTX_SIGNAL_STRING_MOL_UNDERTEMPERATURE_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_CHARGE_WARNING_START_BIT     (16u)
#define CANTX_SIGNAL_STRING_MOL_OVERTEMPERATURE_CHARGE_WARNING_LENGTH        (CAN_BIT)

#define CANTX_SIGNAL_STRING_RSL_UNDERVOLTAGE_WARNING_START_BIT               (31u)
#define CANTX_SIGNAL_STRING_RSL_UNDERVOLTAGE_WARNING_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_OVERVOLTAGE_WARNING_START_BIT                (30u)
#define CANTX_SIGNAL_STRING_RSL_OVERVOLTAGE_WARNING_LENGTH                   (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_DISCHARGE_WARNING_START_BIT (29u)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_CHARGE_WARNING_START_BIT    (28u)
#define CANTX_SIGNAL_STRING_RSL_CELL_OVERCURRENT_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_DISCHARGE_WARNING_START_BIT (27u)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_DISCHARGE_WARNING_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_DISCHARGE_WARNING_START_BIT  (26u)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_DISCHARGE_WARNING_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_CHARGE_WARNING_START_BIT    (25u)
#define CANTX_SIGNAL_STRING_RSL_UNDERTEMPERATURE_CHARGE_WARNING_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_CHARGE_WARNING_START_BIT     (24u)
#define CANTX_SIGNAL_STRING_RSL_OVERTEMPERATURE_CHARGE_WARNING_LENGTH        (CAN_BIT)

#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_VOLT_MEAS_OOR_ERROR_START_BIT   (39u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_VOLT_MEAS_OOR_ERROR_LENGTH      (CAN_BIT)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_CRC_ERROR_START_BIT             (37u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_CRC_ERROR_LENGTH                (CAN_BIT)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_COMMUNICATION_ERROR_START_BIT   (35u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_COMMUNICATION_ERROR_LENGTH      (CAN_BIT)
#define CANTX_SIGNAL_STRING_SLAVE_HARDWARE_ERROR_START_BIT                   (34u)
#define CANTX_SIGNAL_STRING_SLAVE_HARDWARE_ERROR_LENGTH                      (CAN_BIT)
#define CANTX_SIGNAL_STRING_NEGATIVE_CONTACTOR_ERROR_START_BIT               (33u)
#define CANTX_SIGNAL_STRING_NEGATIVE_CONTACTOR_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_POSITIVE_CONTACTOR_ERROR_START_BIT               (32u)
#define CANTX_SIGNAL_STRING_POSITIVE_CONTACTOR_ERROR_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_STRING_FUSED_STRING_VOLTAGE_MEASUREMENT_ERROR_START_BIT (47u)
#define CANTX_SIGNAL_STRING_FUSED_STRING_VOLTAGE_MEASUREMENT_ERROR_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_MEASUREMENT_ERROR_START_BIT       (46u)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_MEASUREMENT_ERROR_LENGTH          (CAN_BIT)
#define CANTX_SIGNAL_STRING_ENERGY_COUNTING_MEASUREMENT_ERROR_START_BIT      (45u)
#define CANTX_SIGNAL_STRING_ENERGY_COUNTING_MEASUREMENT_ERROR_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_STRING_COULOMB_COUNTING_MEASUREMENT_ERROR_START_BIT     (44u)
#define CANTX_SIGNAL_STRING_COULOMB_COUNTING_MEASUREMENT_ERROR_LENGTH        (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_CURRENT_MEASUREMENT_ERROR_START_BIT       (43u)
#define CANTX_SIGNAL_STRING_STRING_CURRENT_MEASUREMENT_ERROR_LENGTH          (CAN_BIT)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_TEMP_MEAS_OOR_ERROR_START_BIT   (41u)
#define CANTX_SIGNAL_STRING_DAISY_CHAIN_BASE_TEMP_MEAS_OOR_ERROR_LENGTH      (CAN_BIT)
#define CANTX_SIGNAL_STRING_VOLTAGE_SPREAD_PLAUSIBILITY_ERROR_START_BIT      (55u)
#define CANTX_SIGNAL_STRING_VOLTAGE_SPREAD_PLAUSIBILITY_ERROR_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_STRING_TEMPERATURE_SPREAD_PLAUSIBILITY_ERROR_START_BIT  (54u)
#define CANTX_SIGNAL_STRING_TEMPERATURE_SPREAD_PLAUSIBILITY_ERROR_LENGTH     (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_PLAUSIBILITY_ERROR_START_BIT      (53u)
#define CANTX_SIGNAL_STRING_STRING_VOLTAGE_PLAUSIBILITY_ERROR_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_STRING_CELL_VOLTAGE_PLAUSIBILITY_ERROR_START_BIT        (52u)
#define CANTX_SIGNAL_STRING_CELL_VOLTAGE_PLAUSIBILITY_ERROR_LENGTH           (CAN_BIT)
#define CANTX_SIGNAL_STRING_CELL_TEMPERATURE_PLAUSIBILITY_ERROR_START_BIT    (51u)
#define CANTX_SIGNAL_STRING_CELL_TEMPERATURE_PLAUSIBILITY_ERROR_LENGTH       (CAN_BIT)
#define CANTX_SIGNAL_STRING_OPEN_WIRE_ERROR_START_BIT                        (49u)
#define CANTX_SIGNAL_STRING_OPEN_WIRE_ERROR_LENGTH                           (CAN_BIT) sl
#define CANTX_SIGNAL_STRING_PACK_VOLTAGE_MEASUREMENT_ERROR_START_BIT         (48u)
#define CANTX_SIGNAL_STRING_PACK_VOLTAGE_MEASUREMENT_ERROR_LENGTH            (CAN_BIT)
#define CANTX_SIGNAL_STRING_STRING_OVERCURRENT_ERROR_START_BIT               (63u)
#define CANTX_SIGNAL_STRING_STRING_OVERCURRENT_ERROR_LENGTH                  (CAN_BIT)

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages          = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures      = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues       = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_s can_tableCurrent                    = {.header.uniqueId = DATA_BLOCK_ID_CURRENT};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire                 = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest         = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues             = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                            = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                            = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                            = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState             = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_s can_tableInsulation              = {.header.uniqueId = DATA_BLOCK_ID_INSULATION};
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
    .pTableCurrent          = &can_tableCurrent,
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
void test_CANTX_SetMslFlags(void) {
    uint64_t testMessageData = 0u;
    uint8_t testString       = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetMslFlags(NULL_PTR, &testMessageData, testString));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetMslFlags(&can_kShim, NULL_PTR, testString));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetMslFlags(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetMslFlags(&can_kShim, &testMessageData, testString);
}

void testCANTX_SetMolFlags(void) {
    uint64_t testMessageData = 0u;
    uint8_t testString       = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetMolFlags(NULL_PTR, &testMessageData, testString));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetMolFlags(&can_kShim, NULL_PTR, testString));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetMolFlags(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetMolFlags(&can_kShim, &testMessageData, testString);
}

void testCANTX_SetRslFlags(void) {
    uint64_t testMessageData = 0u;
    uint8_t testString       = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetRslFlags(NULL_PTR, &testMessageData, testString));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetRslFlags(&can_kShim, NULL_PTR, testString));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetRslFlags(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetRslFlags(&can_kShim, &testMessageData, testString);
}

void testCANTX_SetCurrentSensorFlags(void) {
    uint64_t testMessageData = 0u;
    uint8_t testString       = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetCurrentSensorFlags(NULL_PTR, &testMessageData, testString));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetCurrentSensorFlags(&can_kShim, NULL_PTR, testString));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetCurrentSensorFlags(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetCurrentSensorFlags(&can_kShim, &testMessageData, testString);
}

void testCANTX_SetPlausibilityFlags(void) {
    uint64_t testMessageData = 0u;
    uint8_t testString       = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetPlausibilityFlags(NULL_PTR, &testMessageData, testString));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetPlausibilityFlags(&can_kShim, NULL_PTR, testString));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetPlausibilityFlags(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetPlausibilityFlags(&can_kShim, &testMessageData, testString);
}

void testCANTX_SetOtherErrorFlags(void) {
    uint64_t testMessageData = 0u;
    uint8_t testString       = 0u;
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetOtherErrorFlags(NULL_PTR, &testMessageData, testString));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetOtherErrorFlags(&can_kShim, NULL_PTR, testString));
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetOtherErrorFlags(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetPlausibilityFlags(NULL_PTR, &testMessageData, testString));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetPlausibilityFlags(&can_kShim, NULL_PTR, testString));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_SetPlausibilityFlags(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test with balancing active */
    can_kShim.pTableBalancingControl->nrBalancedCells[testString] = 1u;
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_SetOtherErrorFlags(&can_kShim, &testMessageData, testString);

    /* Reset balancing */
    can_kShim.pTableBalancingControl->nrBalancedCells[testString] = 0u;
}

void testCANTX_BuildStringStateMessage(void) {
    uint64_t testMessageData = 0u;
    uint8_t testString       = 0u;

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildStringStateMessage(NULL_PTR, &testMessageData, testString));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildStringStateMessage(&can_kShim, NULL_PTR, testString));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANTX_BuildStringStateMessage(&can_kShim, &testMessageData, BS_NR_OF_STRINGS + 1u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Call function under test */
    TEST_CANTX_BuildStringStateMessage(&can_kShim, &testMessageData, testString);
}

void testCANTX_StringState(void) {
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CANTX_STRING_STATE_ID,
        .idType     = CANTX_STRING_STATE_ID_TYPE,
        .dlc        = CANTX_STRING_STATE_DLC,
        .endianness = CANTX_STRING_STATE_ENDIANNESS,
    };
    uint8_t testCanData[CANTX_STRING_STATE_DLC] = {0u};
    uint8_t muxId                               = 0u;

    /* ======= Assertion tests ============================================= */
    CAN_MESSAGE_PROPERTIES_s testFalseIdMessage = {
        .id         = CANTX_BMS_STATE_DETAILS_ID,
        .idType     = CANTX_STRING_STATE_ID_TYPE,
        .dlc        = CANTX_STRING_STATE_DLC,
        .endianness = CANTX_STRING_STATE_ENDIANNESS,
    };
    CAN_MESSAGE_PROPERTIES_s testFalseIdTypeMessage = {
        .id         = CANTX_STRING_STATE_ID,
        .idType     = CAN_EXTENDED_IDENTIFIER_29_BIT,
        .dlc        = CANTX_STRING_STATE_DLC,
        .endianness = CANTX_STRING_STATE_ENDIANNESS,
    };
    CAN_MESSAGE_PROPERTIES_s testFalseDlcMessage = {
        .id         = CANTX_STRING_STATE_ID,
        .idType     = CANTX_STRING_STATE_ID_TYPE,
        .dlc        = CAN_DEFAULT_DLC - 1u,
        .endianness = CANTX_STRING_STATE_ENDIANNESS,
    };
    /* ======= AT1/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringState(testFalseIdMessage, testCanData, &muxId, &can_kShim));
    /* ======= AT2/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringState(testFalseIdTypeMessage, testCanData, &muxId, &can_kShim));
    /* ======= AT3/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringState(testFalseDlcMessage, testCanData, &muxId, &can_kShim));

    /* ======= AT4/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringState(testMessage, NULL_PTR, &muxId, &can_kShim));
    /* ======= AT5/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringState(testMessage, testCanData, NULL_PTR, &can_kShim));
    /* ======= AT6/7 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringState(testMessage, testCanData, &muxId, NULL_PTR));
    /* ======= AT7/7 ======= */
    uint8_t errorMuxId = BS_NR_OF_STRINGS;
    TEST_ASSERT_FAIL_ASSERT(CANTX_StringState(testMessage, testCanData, &errorMuxId, &can_kShim));

    /* ======= Routine tests =============================================== */
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

    /* ======= RT1/2: Call function under test */
    /* test string 0 */
    muxId = 0u;
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

    /* ======= RT2/2: Call function under test */
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
