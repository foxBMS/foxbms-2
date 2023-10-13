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
 * @file    test_can_cbs_rx_aerosol-sensor.c
 * @author  foxBMS Team
 * @date    2023-08-31 (date of creation)
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
#include "Mockcan.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockimd.h"
#include "Mockos.h"

#include "database_cfg.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_rx_aerosol-sensor.c")

TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
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
    can_tableAerosolSensor.sensorStatus                   = 0u;
    can_tableAerosolSensor.photoelectricError             = false;
    can_tableAerosolSensor.supplyOvervoltageError         = false;
    can_tableAerosolSensor.supplyUndervoltageError        = false;
    can_tableAerosolSensor.particulateMatterConcentration = 0u;
    can_tableAerosolSensor.crcCheckCode                   = 0u;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing CANRX_HandleAerosolSensorErrors
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for kpkCanShim -> assert
 *            - AT2/2: invalid value for signalData -> assert
 *          - Routine validation:
 *            - RT1/4: Function sets expected values in the struct
 *            - RT2/4: Function sets expected values in the struct
 *            - RT3/4: Function sets expected values in the struct
 *            - RT4/4: Function sets expected values in the struct
 */
void testCANRX_HandleAerosolSensorErrors(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleAerosolSensorErrors(NULL_PTR, 0u));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleAerosolSensorErrors(&can_kShim, 100u));

    /* ======= Routine tests =============================================== */
    /* reset the error table from previous test */
    can_kShim.pTableAerosolSensor->photoelectricError      = false;
    can_kShim.pTableAerosolSensor->supplyOvervoltageError  = false;
    can_kShim.pTableAerosolSensor->supplyUndervoltageError = false;

    /* ======= RT1/4: Test implementation */
    /* ======= RT1/4: call function under test */
    TEST_CANRX_HandleAerosolSensorErrors(&can_kShim, 0u);
    /* ======= RT1/4: test output verification */
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->photoelectricError);
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->supplyOvervoltageError);
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->supplyUndervoltageError);

    /* ======= RT2/4: Test implementation */
    /* ======= RT2/4: call function under test */
    TEST_CANRX_HandleAerosolSensorErrors(&can_kShim, 1u);
    /* ======= RT2/4: test output verification */
    TEST_ASSERT_TRUE(can_kShim.pTableAerosolSensor->photoelectricError);
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->supplyOvervoltageError);
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->supplyUndervoltageError);

    /* ======= RT3/4: Test implementation */
    /* reset the error table from previous test */
    can_kShim.pTableAerosolSensor->photoelectricError      = false;
    can_kShim.pTableAerosolSensor->supplyOvervoltageError  = false;
    can_kShim.pTableAerosolSensor->supplyUndervoltageError = false;
    /* ======= RT3/4: call function under test */
    TEST_CANRX_HandleAerosolSensorErrors(&can_kShim, 2u);
    /* ======= RT3/4: test output verification */
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->photoelectricError);
    TEST_ASSERT_TRUE(can_kShim.pTableAerosolSensor->supplyOvervoltageError);
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->supplyUndervoltageError);

    /* ======= RT4/4: Test implementation */
    /* reset the error table from previous test */
    can_kShim.pTableAerosolSensor->photoelectricError      = false;
    can_kShim.pTableAerosolSensor->supplyOvervoltageError  = false;
    can_kShim.pTableAerosolSensor->supplyUndervoltageError = false;
    /* ======= RT4/4: call function under test */
    TEST_CANRX_HandleAerosolSensorErrors(&can_kShim, 3u);
    /* ======= RT4/4: test output verification */
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->photoelectricError);
    TEST_ASSERT_FALSE(can_kShim.pTableAerosolSensor->supplyOvervoltageError);
    TEST_ASSERT_TRUE(can_kShim.pTableAerosolSensor->supplyUndervoltageError);
}

/**
 * @brief   Testing CANRX_HandleAerosolSensorStatus
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for kpkCanShim -> assert
 *            - AT2/2: invalid value for signalData -> assert
 *          - Routine validation:
 *            - RT1/2: Function calls expected subroutines
 *            - RT2/2: Function sets expected values in the struct and calls
 *                     expected subroutines
 */
void testCANRX_HandleAerosolSensorStatus(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleAerosolSensorStatus(NULL_PTR, 0u));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_HandleAerosolSensorStatus(&can_kShim, 100u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    /* ======= RT1/2: call function under test */
    TEST_CANRX_HandleAerosolSensorStatus(&can_kShim, 0u);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(0, can_kShim.pTableAerosolSensor->sensorStatus);

    /* ======= RT2/2: Test implementation */
    DIAG_Handler_ExpectAndReturn(DIAG_ID_AEROSOL_ALERT, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u, DIAG_HANDLER_RETURN_OK);
    /* ======= RT2/2: call function under test */
    TEST_CANRX_HandleAerosolSensorStatus(&can_kShim, 1u);
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(1u, can_kShim.pTableAerosolSensor->sensorStatus);
}

/**
 * @brief   Testing CANRX_SetParticulateMatterConcentration
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function sets expected values in the struct
 */
void testCANRX_SetParticulateMatterConcentration(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetParticulateMatterConcentration(NULL_PTR, 0u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: call function under test */
    TEST_CANRX_SetParticulateMatterConcentration(&can_kShim, 4u);
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(4u, can_kShim.pTableAerosolSensor->particulateMatterConcentration);
}

/**
 * @brief   Testing CANRX_SetAerosolSensorCrcCheckCode
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutines
 */
void test_CANRX_SetAerosolSensorCrcCheckCode(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_SetAerosolSensorCrcCheckCode(NULL_PTR, 0u));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: call function under test */
    TEST_CANRX_SetAerosolSensorCrcCheckCode(&can_kShim, 4);
    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL(4u, can_kShim.pTableAerosolSensor->crcCheckCode);
}

/**
 * @brief   Testing CANRX_AerosolSensor
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/5: invalid message id for message -> assert
 *            - AT2/5: invalid message idType for message -> assert
 *            - AT3/5: invalid message DLC for message -> assert
 *            - AT4/5: NULL_PTR for kpkCanData -> assert
 *            - AT5/5: NULL_PTR for kpkCanShim -> assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutines
 */
void testCANRX_AerosolSensor(void) {
    /* ======= Assertion tests ============================================= */
    uint8_t canData[CAN_MAX_DLC] = {0};

    /* ======= AT1/5 ======= */
    CAN_MESSAGE_PROPERTIES_s testMessageInvalidId = {
        .id         = CAN_MAX_11BIT_ID,                /* invalid message id */
        .idType     = CANRX_AEROSOL_SENSOR_ID_TYPE,    /* valid id type */
        .dlc        = CAN_DEFAULT_DLC,                 /* valid dlc */
        .endianness = CANRX_AEROSOL_SENSOR_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_AerosolSensor(testMessageInvalidId, canData, &can_kShim));

    /* ======= AT2/5 ======= */
    CAN_MESSAGE_PROPERTIES_s testMessageInvalidIdType = {
        .id         = CANRX_AEROSOL_SENSOR_ID,         /* valid message id */
        .idType     = CAN_EXTENDED_IDENTIFIER_29_BIT,  /* invalid id type */
        .dlc        = CAN_DEFAULT_DLC,                 /* valid dlc */
        .endianness = CANRX_AEROSOL_SENSOR_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_AerosolSensor(testMessageInvalidIdType, canData, &can_kShim));

    /* ======= AT3/5 ======= */
    CAN_MESSAGE_PROPERTIES_s testMessageInvalidDlc = {
        .id         = CANRX_AEROSOL_SENSOR_ID,         /* valid message id */
        .idType     = CANRX_AEROSOL_SENSOR_ID_TYPE,    /* valid id type */
        .dlc        = CAN_DEFAULT_DLC + 1u,            /* invalid dlc */
        .endianness = CANRX_AEROSOL_SENSOR_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_AerosolSensor(testMessageInvalidDlc, canData, &can_kShim));

    /* ======= AT4/5 ======= */
    CAN_MESSAGE_PROPERTIES_s validTestMessage = {
        .id         = CANRX_AEROSOL_SENSOR_ID,         /* valid message id */
        .idType     = CANRX_AEROSOL_SENSOR_ID_TYPE,    /* valid id type */
        .dlc        = CAN_DEFAULT_DLC,                 /* valid dlc */
        .endianness = CANRX_AEROSOL_SENSOR_ENDIANNESS, /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_AerosolSensor(validTestMessage, NULL_PTR, &can_kShim));

    /* ======= AT5/5 ======= */
    TEST_ASSERT_FAIL_ASSERT(CANRX_AerosolSensor(validTestMessage, canData, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ignore reads that are not used in this test */
    DATA_Read1DataBlock_IgnoreAndReturn(STD_OK);
    DATA_Write1DataBlock_IgnoreAndReturn(STD_OK);
    /* ======= RT1/1: call function under test */
    CANRX_AerosolSensor(validTestMessage, canData, &can_kShim);
}
