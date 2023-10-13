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
 * @file    test_adi_ades1830_afe.c
 * @author  foxBMS Team
 * @date    2020-08-10 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_sys_dma.h"
#include "Mockadi_ades183x.h"
#include "Mockadi_ades183x_cfg.h"

#include "afe.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades183x_afe.c")

TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/ades1830")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/pec")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
ADI_STATE_s adi_stateBase = {0};

const uint8_t testStringId = 1u;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testAFE_TriggerIc(void) {
    ADI_MeasurementCycle_Expect(&adi_stateBase);
    TEST_ASSERT_EQUAL(STD_OK, AFE_TriggerIc());
}

void testAFE_Initialize(void) {
    ADI_ActivateInterfaceBoard_Expect();
    TEST_ASSERT_EQUAL(STD_OK, AFE_Initialize());
}

void testAFE_RequestEepromRead(void) {
    TEST_ASSERT_EQUAL(STD_OK, AFE_RequestEepromRead(testStringId));
}

void testAFE_RequestEepromWrite(void) {
    TEST_ASSERT_EQUAL(STD_OK, AFE_RequestEepromWrite(testStringId));
}

void testAFE_RequestTemperatureRead(void) {
    TEST_ASSERT_EQUAL(STD_OK, AFE_RequestTemperatureRead(testStringId));
}

void testAFE_RequestBalancingFeedbackRead(void) {
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_RequestBalancingFeedbackRead(testStringId));
}

void testAFE_RequestOpenWireCheck(void) {
    TEST_ASSERT_EQUAL(STD_OK, AFE_RequestOpenWireCheck(testStringId));
}

void testAFE_StartMeasurement(void) {
    ADI_MakeRequest_ExpectAndReturn(AFE_START_REQUEST, STD_OK);
    TEST_ASSERT_EQUAL(STD_OK, AFE_StartMeasurement());
}

void testAFE_IsFirstMeasurementCycleFinished(void) {
    ADI_IsFirstMeasurementCycleFinished_ExpectAndReturn(&adi_stateBase, true);
    TEST_ASSERT_TRUE(AFE_IsFirstMeasurementCycleFinished());
}

void testAFE_RequestIoRead(void) {
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_RequestIoRead(testStringId));
}

void testAFE_RequestIoWrite(void) {
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_RequestIoWrite(testStringId));
}
