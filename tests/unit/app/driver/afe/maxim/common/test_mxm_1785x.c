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
 * @file    test_mxm_1785x.c
 * @author  foxBMS Team
 * @date    2020-07-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  MXM
 *
 * @brief   Test for the Maxim 1785x driver.
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe_plausibility.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfassert.h"
#include "Mockmxm_17841b.h"
#include "Mockmxm_41b_register_map.h"
#include "Mockmxm_battery_management.h"
#include "Mockmxm_cfg.h"
#include "Mockmxm_crc8.h"
#include "Mockmxm_registry.h"
#include "Mockos.h"
#include "Mocktsi.h"

#include "database_cfg.h"

#include "mxm_1785x.h"
#include "mxm_1785x_tools.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("mxm_1785x.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/maxim/common")
TEST_INCLUDE_PATH("../../src/app/driver/afe/maxim/common/config")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_OPEN_WIRE_s mxm_tableOpenWire = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};

static MXM_MONITORING_INSTANCE_s mxm_instance = {
    .state                 = MXM_STATEMACHINE_STATES_UNINITIALIZED,
    .operationSubstate     = MXM_INIT_ENTRY,
    .allowStartup          = false,
    .operationRequested    = false,
    .firstMeasurementDone  = false,
    .stopRequested         = false,
    .openWireRequested     = false,
    .undervoltageAlert     = false,
    .dcByte                = MXM_DC_EMPTY,
    .mxmVoltageCellCounter = 0,
    .highest5xDevice       = 0,
    .requestStatus5x       = MXM_5X_STATE_UNSENT,
    .batteryCmdBuffer =
        {
            .regAddress    = (MXM_REG_NAME_e)0x00,
            .lsb           = 0x00,
            .msb           = 0x00,
            .deviceAddress = 0x00,
            .blockSize     = 0,
            .model         = MXM_MODEL_ID_MAX17852,
        },
    .resultSelfCheck = STD_NOT_OK,
    .selfCheck =
        {
            .crc                      = STD_NOT_OK,
            .conversion               = STD_NOT_OK,
            .firstSetBit              = STD_NOT_OK,
            .extractValueFromRegister = STD_NOT_OK,
            .parseVoltageReadall      = STD_NOT_OK,
            .addressSpaceChecker      = STD_NOT_OK,
            .fmeaStatusASCI           = STD_NOT_OK,
        },
    .pOpenWire_table = &mxm_tableOpenWire,
};

const bool mxm_allowSkippingPostInitSelfCheck = true;

/** dummy implementation for the Operation state machine */
extern void MXM_StateMachineOperation(MXM_MONITORING_INSTANCE_s *pState) {
}

/** local variable that allows to change the reported model id on a per test basis*/
static MXM_MODEL_ID_e modelForTest = MXM_MODEL_ID_MAX17852;

/** dummy implementation for the model id */
extern MXM_MODEL_ID_e MXM_GetModelIdOfDaisyChain(void) {
    return modelForTest;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    mxm_instance.state             = MXM_STATEMACHINE_STATES_UNINITIALIZED;
    mxm_instance.operationSubstate = MXM_INIT_ENTRY;
    mxm_instance.requestStatus5x   = MXM_5X_STATE_UNSENT;

    modelForTest = MXM_MODEL_ID_MAX17852;

    /* delete rx buffer */
    for (uint16_t i = 0u; i < MXM_RX_BUFFER_LENGTH; i++) {
        mxm_instance.rxBuffer[i] = 0u;
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testMXM_ParseVoltageReadAllTest(void) {
    TEST_ASSERT_PASS_ASSERT(TEST_ASSERT_EQUAL(STD_OK, TEST_MXM_ParseVoltageReadAllTest(&mxm_instance)));
}

/** This test aims to provoke the assertion that checks if the battery system fits the internal configuration */
void testMXM_ConstructBalancingBufferTest(void) {
    /* skip this test if the BMS configuration does not fit to this test
       (we cannot run with more cells than Maxim supports) */
    if ((BS_NR_OF_CELL_BLOCKS_PER_MODULE > MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE)) {
        TEST_PASS_MESSAGE("This test is skipped due to the configuration of the BMS.");
    }
    TEST_ASSERT_FAIL_ASSERT(MXM_ConstructBalancingBuffer(NULL_PTR));
    DATA_BLOCK_BALANCING_CONTROL_s balancingTable = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
    MXM_BALANCING_STATE_s balancingInstance       = {.pBalancingControl_table = &balancingTable};
    TEST_ASSERT_PASS_ASSERT(MXM_ConstructBalancingBuffer(&balancingInstance));
}

void testTEST_MXM_ParseVoltageReadAllNullPointer(void) {
    uint8_t voltRxBuffer           = 0;
    MXM_DATA_STORAGE_s dataStorage = {0};

    TEST_ASSERT_FAIL_ASSERT(TEST_MXM_ParseVoltageReadAll(NULL_PTR, 0u, &dataStorage, 0u));

    TEST_ASSERT_FAIL_ASSERT(TEST_MXM_ParseVoltageReadAll(&voltRxBuffer, 0u, NULL_PTR, 0u));
}

void testMXM_ParseVoltageReadAllTestNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_MXM_ParseVoltageReadAllTest(NULL_PTR));
}

void testMXM_ParseVoltagesIntoDBNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_MXM_ParseVoltagesIntoDB(NULL_PTR));
}

void testMXM_MonGetVoltagesNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ASSERT_EQUAL(0u, MXM_MonGetVoltages(NULL_PTR, 0u)));
}

void testMXM_HandleStateWriteallNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_HandleStateWriteall(NULL_PTR, 0u));
}

void testMXM_HandleStateWriteallInvalidRequestStatus(void) {
    /* set request status to an invalid value */
    mxm_instance.requestStatus5x = 9999;
    TEST_ASSERT_FAIL_ASSERT(MXM_HandleStateWriteall(&mxm_instance, 42u));
}

void testMXM_HandleStateWriteallEntry(void) {
    TEST_ASSERT_EQUAL(MXM_5X_STATE_UNSENT, mxm_instance.requestStatus5x);
    MXM_5XSetStateRequest_ExpectAndReturn(
        mxm_instance.pInstance5X,
        MXM_STATEMACH_5X_WRITEALL,
        mxm_instance.batteryCmdBuffer,
        &mxm_instance.requestStatus5x,
        STD_OK);
    TEST_ASSERT_PASS_ASSERT(MXM_HandleStateWriteall(&mxm_instance, 42u));
}

void testMXM_HandleStateWriteallUnprocessed(void) {
    mxm_instance.requestStatus5x = MXM_5X_STATE_UNPROCESSED;
    TEST_ASSERT_PASS_ASSERT(MXM_HandleStateWriteall(&mxm_instance, 42u));
}

void testMXM_HandleStateWriteallError(void) {
    mxm_instance.requestStatus5x = MXM_5X_STATE_ERROR;
    OS_GetTickCount_ExpectAndReturn(0u);
    TEST_ASSERT_PASS_ASSERT(MXM_HandleStateWriteall(&mxm_instance, 42u));
    TEST_ASSERT_EQUAL(MXM_5X_STATE_UNSENT, mxm_instance.requestStatus5x);
}

void testMXM_HandleStateWriteallProcessed(void) {
    mxm_instance.requestStatus5x = MXM_5X_STATE_PROCESSED;
    TEST_ASSERT_PASS_ASSERT(MXM_HandleStateWriteall(&mxm_instance, 42u));
    TEST_ASSERT_EQUAL(MXM_5X_STATE_UNSENT, mxm_instance.requestStatus5x);
    TEST_ASSERT_EQUAL(42, mxm_instance.operationSubstate);
}

void testMXM_HandleStateReadallNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ASSERT_EQUAL(false, MXM_HandleStateReadall(NULL_PTR, 0u, 0u)));
}

void testMXM_HandleStateReadallInvalidRequestStatus(void) {
    /* set request status to an invalid value */
    mxm_instance.requestStatus5x = 9999;
    TEST_ASSERT_FAIL_ASSERT(TEST_ASSERT_EQUAL(false, MXM_HandleStateReadall(&mxm_instance, 21u, 42u)));
}

void testMXM_HandleStateReadallUnsent(void) {
    TEST_ASSERT_EQUAL(MXM_5X_STATE_UNSENT, mxm_instance.requestStatus5x);
    const uint8_t registerAddress = 21u;
    /* recreate a local variant of the expected command buffer */
    MXM_5X_COMMAND_PAYLOAD_s commandBuffer = {
        .regAddress = registerAddress,
        .lsb        = 0,
        .msb        = 0,
        .model      = modelForTest,
    };
    MXM_5XSetStateRequest_ExpectAndReturn(
        mxm_instance.pInstance5X, MXM_STATEMACH_5X_READALL, commandBuffer, &mxm_instance.requestStatus5x, STD_OK);
    bool functionReturnValue;
    TEST_ASSERT_PASS_ASSERT(functionReturnValue = MXM_HandleStateReadall(&mxm_instance, registerAddress, 42u));
    TEST_ASSERT_EQUAL(false, functionReturnValue);
}

void testMXM_HandleStateReadallUnprocessed(void) {
    mxm_instance.requestStatus5x = MXM_5X_STATE_UNPROCESSED;
    bool functionReturnValue;
    TEST_ASSERT_PASS_ASSERT(functionReturnValue = MXM_HandleStateReadall(&mxm_instance, 21u, 42u));
    TEST_ASSERT_EQUAL(false, functionReturnValue);
}

void testMXM_HandleStateReadallError(void) {
    mxm_instance.requestStatus5x = MXM_5X_STATE_ERROR;
    OS_GetTickCount_ExpectAndReturn(0u);
    bool functionReturnValue;
    TEST_ASSERT_PASS_ASSERT(functionReturnValue = MXM_HandleStateReadall(&mxm_instance, 21u, 42u));
    TEST_ASSERT_EQUAL(false, functionReturnValue);
    TEST_ASSERT_EQUAL(MXM_5X_STATE_UNSENT, mxm_instance.requestStatus5x);
}

void testMXM_HandleStateReadallProcessed(void) {
    mxm_instance.requestStatus5x = MXM_5X_STATE_PROCESSED;
    MXM_5XGetRXBuffer_ExpectAndReturn(mxm_instance.pInstance5X, mxm_instance.rxBuffer, MXM_RX_BUFFER_LENGTH, STD_OK);
    MXM_5XGetLastDCByte_ExpectAndReturn(mxm_instance.pInstance5X, 84u);
    bool functionReturnValue;
    TEST_ASSERT_PASS_ASSERT(functionReturnValue = MXM_HandleStateReadall(&mxm_instance, 21u, 42u));
    TEST_ASSERT_EQUAL(true, functionReturnValue);
    TEST_ASSERT_EQUAL(MXM_5X_STATE_UNSENT, mxm_instance.requestStatus5x);
    TEST_ASSERT_EQUAL(42u, mxm_instance.operationSubstate);
    TEST_ASSERT_EQUAL(84u, mxm_instance.dcByte);
}

void testMXM_ProcessOpenWireNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_ProcessOpenWire(NULL_PTR));
}

void testMXM_ProcessOpenWire1SatelliteAlternatingPattern(void) {
    /* with only one module, the entry of the LSB should be in the third
        position of the RX buffer and the MSB in the fourth */
    mxm_instance.rxBuffer[2] = 85;  /* 0b01010101 */
    mxm_instance.rxBuffer[3] = 170; /* 0b10101010 */
    /* simulate 1 satellite */
    MXM_5XGetNumberOfSatellites_ExpectAndReturn(mxm_instance.pInstance5X, 1);
    /* don't care about the database call */
    DATA_Write1DataBlock_ExpectAndReturn(mxm_instance.pOpenWire_table, STD_OK);
    TEST_ASSERT_PASS_ASSERT(MXM_ProcessOpenWire(&mxm_instance));

    /* check for the injected pattern */
    for (uint8_t i = 0u; i < 8; i = i + 2) {
        TEST_ASSERT_EQUAL(1, mxm_instance.pOpenWire_table->openWire[0][i]);
    }
    for (uint8_t i = 1u; i < 8; i = i + 2) {
        TEST_ASSERT_EQUAL(0, mxm_instance.pOpenWire_table->openWire[0][i]);
    }
    for (uint8_t i = 8u; i < MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE; i = i + 2) {
        TEST_ASSERT_EQUAL(0, mxm_instance.pOpenWire_table->openWire[0][i]);
    }
    for (uint8_t i = 9u; i < MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE; i = i + 2) {
        TEST_ASSERT_EQUAL(1, mxm_instance.pOpenWire_table->openWire[0][i]);
    }
}

void testMXM_StateMachineNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_StateMachine(NULL_PTR));
}
