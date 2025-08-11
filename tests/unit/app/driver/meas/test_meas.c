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
 * @file    test_meas.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the meas module
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe.h"

#include "battery_system_cfg.h"

#include "meas.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/meas")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testMEAS_Control(void) {
    AFE_TriggerIc_ExpectAndReturn(STD_OK);
    MEAS_Control();
}

void testMEAS_RequestTemperatureRead(void) {
    TEST_ASSERT_FAIL_ASSERT(MEAS_RequestTemperatureRead(BS_NR_OF_STRINGS));

    AFE_RequestTemperatureRead_ExpectAndReturn(0u, STD_OK);
    MEAS_RequestTemperatureRead(0u);
}

void testMEAS_RequestBalancingFeedbackRead(void) {
    TEST_ASSERT_FAIL_ASSERT(MEAS_RequestBalancingFeedbackRead(BS_NR_OF_STRINGS));

    AFE_RequestBalancingFeedbackRead_ExpectAndReturn(0u, STD_OK);
    MEAS_RequestBalancingFeedbackRead(0u);
}

void testMEAS_RequestEepromRead(void) {
    TEST_ASSERT_FAIL_ASSERT(MEAS_RequestEepromRead(BS_NR_OF_STRINGS));

    AFE_RequestEepromRead_ExpectAndReturn(0u, STD_OK);
    MEAS_RequestEepromRead(0u);
}

void testMEAS_RequestEepromWrite(void) {
    TEST_ASSERT_FAIL_ASSERT(MEAS_RequestEepromWrite(BS_NR_OF_STRINGS));

    AFE_RequestEepromWrite_ExpectAndReturn(0u, STD_OK);
    MEAS_RequestEepromWrite(0u);
}

void testMEAS_IsFirstMeasurementCycleFinished(void) {
    AFE_IsFirstMeasurementCycleFinished_ExpectAndReturn(STD_OK);
    MEAS_IsFirstMeasurementCycleFinished();
}

void testMEAS_StartMeasurement(void) {
    AFE_StartMeasurement_ExpectAndReturn(STD_OK);
    MEAS_StartMeasurement();
}

void testMEAS_Initialize(void) {
    AFE_Initialize_ExpectAndReturn(STD_OK);
    MEAS_Initialize();
}

void testMEAS_RequestOpenWireCheck(void) {
    TEST_ASSERT_FAIL_ASSERT(MEAS_RequestOpenWireCheck(BS_NR_OF_STRINGS));

    AFE_RequestOpenWireCheck_ExpectAndReturn(0u, STD_OK);
    MEAS_RequestOpenWireCheck(0u);
}
