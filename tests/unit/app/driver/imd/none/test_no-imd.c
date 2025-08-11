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
 * @file    test_no-imd.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the dummy insulation monitoring driver
 * @details Tests dummy init and functions
 *
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "Mockdatabase.h"

#include "database_cfg.h"

#include "no-imd.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/imd/none")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

DATA_BLOCK_INSULATION_MONITORING_s noimd_tableInsulationValues = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};

/*========== Test Cases =====================================================*/
void testImdDummyInitialize(void) {
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_IMD_ENABLE, TEST_NOIMD_Initialize());
}

void testNOIMD_EnableIMD(void) {
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_RUNNING, TEST_NOIMD_EnableImd());
}

void testImdDummyFunctions(void) {
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_RUNNING, TEST_NOIMD_MeasureInsulation(&noimd_tableInsulationValues));
}

void testNOIMD_DiableImd(void) {
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_IMD_ENABLE, TEST_NOIMD_DisableImd());
}

void testIMD_ProcessInitializationState(void) {
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_IMD_ENABLE, IMD_ProcessInitializationState());
}

void testIMD_ProcessEnableState(void) {
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_RUNNING, IMD_ProcessEnableState());
}

void testIMD_ProcessRunningState(void) {
    TEST_ASSERT_FAIL_ASSERT(IMD_ProcessRunningState(NULL_PTR));
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_RUNNING, IMD_ProcessRunningState(&noimd_tableInsulationValues));
}

void testIMD_ProcessShutdownState(void) {
    TEST_ASSERT_EQUAL(IMD_FSM_STATE_IMD_ENABLE, IMD_ProcessShutdownState());
}
