/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_mxm_registry.c
 * @author  foxBMS Team
 * @date    2020-07-16 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  MXM
 *
 * @brief   Test for the Maxim registry tool.
 *
 * @details def
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockfassert.h"
#include "Mockmxm_1785x.h"
#include "Mockmxm_basic_defines.h"

#include "foxmath.h"
#include "mxm_1785x_tools.h"
#include "mxm_registry.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/
static MXM_MONITORING_INSTANCE_s mxm_state = {
    .state                 = MXM_STATEMACHINE_STATES_UNINITIALIZED,
    .operationSubstate     = MXM_INIT_ENTRY,
    .allowStartup          = false,
    .operationRequested    = false,
    .firstMeasurementDone  = false,
    .stopRequested         = false,
    .openwireRequested     = false,
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
            .blocksize     = 0,
            .model         = MXM_MODEL_ID_MAX17852,
        },
    .resultSelfCheck = STD_NOT_OK,
    .selfCheck =
        {
            .crc                      = STD_NOT_OK,
            .conv                     = STD_NOT_OK,
            .firstSetBit              = STD_NOT_OK,
            .extractValueFromRegister = STD_NOT_OK,
            .parseVoltageReadall      = STD_NOT_OK,
            .addressSpaceChecker      = STD_NOT_OK,
            .fmeaStatusASCI           = STD_NOT_OK,
        },
};

static void rxBufferFillAscending(MXM_MONITORING_INSTANCE_s *pState) {
    TEST_ASSERT(pState != NULL_PTR);
    for (uint16_t i = 0u; i <= MXM_RX_BUFFER_LENGTH; i++) {
        pState->rxBuffer[i] = i;
    }
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    MXM_MonRegistryInit(&mxm_state);
    mxm_state.highest5xDevice = 0;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testRegistryInitiated(void) {
    /* check once: should be initiated from init */
    for (uint8_t i = 0u; i < MXM_MAXIMUM_NR_OF_MODULES; i++) {
        TEST_ASSERT_EQUAL(false, mxm_state.registry[i].connected);
        TEST_ASSERT_EQUAL(0u, mxm_state.registry[i].deviceAddress);
        TEST_ASSERT_EQUAL(0u, mxm_state.registry[i].deviceID);
        TEST_ASSERT_EQUAL(MXM_MODEL_ID_NONE, mxm_state.registry[i].model);
        TEST_ASSERT_EQUAL(MXM_siliconVersion_0, mxm_state.registry[i].siliconVersion);
    }

    /* set some values to the registry */
    mxm_state.registry[8].connected      = true;
    mxm_state.registry[15].deviceAddress = 42u;
    mxm_state.registry[3].siliconVersion = 3u;
    mxm_state.registry[30].model         = 0x358;
    mxm_state.registry[18].deviceID      = 1414;

    /* call init again */
    MXM_MonRegistryInit(&mxm_state);

    /* check a second time: should be initiated again */
    for (uint8_t i = 0u; i < MXM_MAXIMUM_NR_OF_MODULES; i++) {
        TEST_ASSERT_EQUAL(false, mxm_state.registry[i].connected);
        TEST_ASSERT_EQUAL(0u, mxm_state.registry[i].deviceAddress);
        TEST_ASSERT_EQUAL(0u, mxm_state.registry[i].deviceID);
        TEST_ASSERT_EQUAL(MXM_MODEL_ID_NONE, mxm_state.registry[i].model);
        TEST_ASSERT_EQUAL(MXM_siliconVersion_0, mxm_state.registry[i].siliconVersion);
    }
}

void testMXM_MonRegistryInitNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryInit(NULL_PTR));
}

void testMXM_MonRegistryConnectDevicesNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryConnectDevices(NULL_PTR, 0));
}

void testRegistryConnectDevices(void) {
    TEST_ASSERT_EQUAL(STD_OK, MXM_MonRegistryConnectDevices(&mxm_state, 0));
    TEST_ASSERT_EQUAL(false, mxm_state.registry[0].connected);
    TEST_ASSERT_EQUAL(STD_OK, MXM_MonRegistryConnectDevices(&mxm_state, 1));
    TEST_ASSERT_EQUAL(true, mxm_state.registry[0].connected);
    TEST_ASSERT_EQUAL(STD_OK, MXM_MonRegistryConnectDevices(&mxm_state, MXM_MAXIMUM_NR_OF_MODULES));
    TEST_ASSERT_EQUAL(true, mxm_state.registry[MXM_MAXIMUM_NR_OF_MODULES - 1u].connected);
}

void testRegistryConnectDevicesInvalidNumber(void) {
    TEST_ASSERT_EQUAL(STD_NOT_OK, MXM_MonRegistryConnectDevices(&mxm_state, (MXM_MAXIMUM_NR_OF_MODULES + 1u)));
}

void testMXM_MonRegistryGetHighestConnected5XDeviceNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryGetHighestConnected5XDevice(NULL_PTR));
}

void testHighest5X(void) {
    mxm_state.highest5xDevice = 42u;

    TEST_ASSERT_EQUAL(42u, MXM_MonRegistryGetHighestConnected5XDevice(&mxm_state));
}

void testMXM_MonRegistryParseIdIntoDevicesNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseIdIntoDevices(NULL_PTR, 3, MXM_REG_ID1));
}

void testMXM_MonRegistryParseIdIntoDevicesInvalidRegister(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 3, MXM_REG_STATUS1));
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 3, (MXM_REG_NAME_e)0));
}

void testMXM_MonRegistryParseIdIntoDevicesValidRegister(void) {
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 3, MXM_REG_ID1));
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 3, MXM_REG_ID2));
}

void testMXM_MonRegistryParseIdIntoDevicesCheckAscendingBufferId1(void) {
    /* fill the buffer with ascending values and check at few places that the
       parsing is consistent (always the same value) */
    rxBufferFillAscending(&mxm_state);
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseIdIntoDevices(
        &mxm_state, (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2u * highestDevice)), MXM_REG_ID1));
    TEST_ASSERT_EQUAL(6938, mxm_state.registry[0].deviceID);
    TEST_ASSERT_EQUAL(6424, mxm_state.registry[1].deviceID);
}

void testMXM_MonRegistryParseIdIntoDevicesWrongBufferLengthId1(void) {
    /* check if wrong buffer lengths are sanitized */
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 0, MXM_REG_ID1));
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 3, MXM_REG_ID1));
}

void testMXM_MonRegistryParseIdIntoDevicesCheckAscendingBufferId2(void) {
    /* fill the buffer with ascending values and check at few places that the
       parsing is consistent (always the same value) */
    rxBufferFillAscending(&mxm_state);
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseIdIntoDevices(
        &mxm_state, (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2u * highestDevice)), MXM_REG_ID2));
    TEST_ASSERT_EQUAL(454688768, mxm_state.registry[0].deviceID);
    TEST_ASSERT_EQUAL(421003264, mxm_state.registry[1].deviceID);
}

void testMXM_MonRegistryParseIdIntoDevicesWrongBufferLengthId2(void) {
    /* check if wrong buffer lengths are sanitized */
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 0, MXM_REG_ID2));
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseIdIntoDevices(&mxm_state, 3, MXM_REG_ID2));
}

void testMXM_MonRegistryParseVersionIntoDevicesNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseVersionIntoDevices(NULL_PTR, 0));
}

void testMXM_MonRegistryParseVersionIntoDevicesCheckAscendingBuffer(void) {
    /* fill the buffer with ascending values and check at few places that the
       parsing is consistent (always the same value) */
    rxBufferFillAscending(&mxm_state);
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    /* inject values so that satellite with highestDevice - 1u will report a model ID MXM_MODEL_ID_MAX17854 */
    mxm_state.rxBuffer[3u] = 0x85u;
    mxm_state.rxBuffer[2u] = 0x40u;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseVersionIntoDevices(
        &mxm_state, (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2u * highestDevice))));
    TEST_ASSERT_EQUAL(MXM_MODEL_ID_invalid, mxm_state.registry[0].model); /*!< invalid model ID */
    TEST_ASSERT_EQUAL(8, mxm_state.registry[1].siliconVersion);
    TEST_ASSERT_EQUAL(MXM_MODEL_ID_MAX17854, mxm_state.registry[highestDevice - 1u].model);
    TEST_ASSERT_EQUAL(0, mxm_state.registry[highestDevice - 1u].siliconVersion);
}

void testMXM_MonRegistryParseVersionIntoDevicesWrongBufferLength(void) {
    /* check if wrong buffer lengths are sanitized */
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseVersionIntoDevices(&mxm_state, 3));
}

/** call #MXM_MonRegistryParseStatusFmeaIntoDevices() with a null pointer */
void testMXM_MonRegistryParseStatusFmeaIntoDevicesNullPointer(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseStatusFmeaIntoDevices(NULL_PTR, 0));
}

void testMXM_MonRegistryParseStatusFmeaIntoDevicesCheckAscendingBuffer(void) {
    /* fill the buffer with ascending values and check at few places that the
       parsing is consistent (always the same value) */
    rxBufferFillAscending(&mxm_state);
    /* inject the "correct" register number (STATUS2) in the stream */
    mxm_state.rxBuffer[1u]      = MXM_REG_STATUS2;
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    const uint8_t messageLength = (BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2u * highestDevice));
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseStatusFmeaIntoDevices(&mxm_state, messageLength));
    TEST_ASSERT_EQUAL(0x1B1Au, mxm_state.registry[0u].registerStatus2);
    TEST_ASSERT_EQUAL(0x1918u, mxm_state.registry[1u].registerStatus2);
    TEST_ASSERT_EQUAL(0x0302u, mxm_state.registry[12u].registerStatus2);

    /* inject the "STATUS1" register number in the stream */
    mxm_state.rxBuffer[1u] = MXM_REG_STATUS1;
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseStatusFmeaIntoDevices(&mxm_state, messageLength));
    TEST_ASSERT_EQUAL(0x1B1Au, mxm_state.registry[0u].registerStatus1);
    TEST_ASSERT_EQUAL(0x1918u, mxm_state.registry[1u].registerStatus1);
    TEST_ASSERT_EQUAL(0x0302u, mxm_state.registry[12u].registerStatus1);

    /* inject the "STATUS3" register number in the stream */
    mxm_state.rxBuffer[1u] = MXM_REG_STATUS3;
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseStatusFmeaIntoDevices(&mxm_state, messageLength));
    TEST_ASSERT_EQUAL(0x1B1Au, mxm_state.registry[0u].registerStatus3);
    TEST_ASSERT_EQUAL(0x1918u, mxm_state.registry[1u].registerStatus3);
    TEST_ASSERT_EQUAL(0x0302u, mxm_state.registry[12u].registerStatus3);

    /* inject the "FMEA1" register number in the stream */
    mxm_state.rxBuffer[1u] = MXM_REG_FMEA1;
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseStatusFmeaIntoDevices(&mxm_state, messageLength));
    TEST_ASSERT_EQUAL(0x1B1Au, mxm_state.registry[0u].registerFmea1);
    TEST_ASSERT_EQUAL(0x1918u, mxm_state.registry[1u].registerFmea1);
    TEST_ASSERT_EQUAL(0x0302u, mxm_state.registry[12u].registerFmea1);

    /* inject the "FMEA2" register number in the stream */
    mxm_state.rxBuffer[1u] = MXM_REG_FMEA2;
    TEST_ASSERT_PASS_ASSERT(MXM_MonRegistryParseStatusFmeaIntoDevices(&mxm_state, messageLength));
    TEST_ASSERT_EQUAL(0x1B1Au, mxm_state.registry[0u].registerFmea2);
    TEST_ASSERT_EQUAL(0x1918u, mxm_state.registry[1u].registerFmea2);
    TEST_ASSERT_EQUAL(0x0302u, mxm_state.registry[12u].registerFmea2);
}

void testMXM_MonRegistryParseStatusFmeaIntoDevicesWrongBufferLength(void) {
    /* check if wrong buffer lengths are sanitized */
    const uint8_t highestDevice = 13u;
    mxm_state.highest5xDevice   = highestDevice;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    TEST_ASSERT_FAIL_ASSERT(MXM_MonRegistryParseStatusFmeaIntoDevices(&mxm_state, 3));
}

/** check that #MXM_CheckIfADeviceHasBeenReset() recognizes a set ALRTRST bit */
void testMXM_CheckIfADeviceHasBeenReset(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_CheckIfADeviceHasBeenReset(NULL_PTR));

    const uint8_t highestDevice = 10u;
    MXM_MonRegistryConnectDevices(&mxm_state, highestDevice);
    /* place fake cleared register values */
    for (uint8_t i = 0u; i < highestDevice; i++) {
        mxm_state.registry[i].registerStatus1 = 0u;
    }
    /* therefore, no device should be reset */
    TEST_ASSERT_EQUAL(false, MXM_CheckIfADeviceHasBeenReset(&mxm_state));

    /* fake one device being reset */
    mxm_state.registry[0u].registerStatus1 = 0x4000u;
    TEST_ASSERT_EQUAL(true, MXM_CheckIfADeviceHasBeenReset(&mxm_state));
}

/** check that invalid values trip #MXM_CheckIfADeviceIsConnected() */
void testMXM_CheckIfADeviceIsConnectedinvalidValues(void) {
    TEST_ASSERT_FAIL_ASSERT(MXM_CheckIfADeviceIsConnected(NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(MXM_CheckIfADeviceIsConnected(&mxm_state, MXM_MAXIMUM_NR_OF_MODULES + 1u));
}

/** check if #MXM_CheckIfADeviceIsConnected() does its job */
void testMXM_CheckIfADeviceIsConnected(void) {
    for (uint8_t checkedDevice = 0u; checkedDevice < MXM_MAXIMUM_NR_OF_MODULES; checkedDevice++) {
        mxm_state.registry[checkedDevice].connected = false;
        TEST_ASSERT_FALSE(MXM_CheckIfADeviceIsConnected(&mxm_state, checkedDevice));
        mxm_state.registry[checkedDevice].connected = true;
        TEST_ASSERT_TRUE(MXM_CheckIfADeviceIsConnected(&mxm_state, checkedDevice));
    }
}
