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
 * @file    test_soh_debug.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for SOH module responsible for calculation of state-of-health
 * @details Tests initializiation and calculation of health
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"

#include "state_estimation.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("soc_debug.c")
TEST_SOURCE_FILE("soe_debug.c")
TEST_SOURCE_FILE("soh_debug.c")

TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testSE_InitializeStateOfHealth(void) {
    static DATA_BLOCK_SOH_s se_tableSohEstimation = {.header.uniqueId = DATA_BLOCK_ID_SOH};
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfHealth(NULL_PTR, 0u));
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfHealth(&se_tableSohEstimation, BS_NR_OF_STRINGS));

    SE_InitializeStateOfHealth(&se_tableSohEstimation, 0u);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, se_tableSohEstimation.averageSoh_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, se_tableSohEstimation.minimumSoh_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, se_tableSohEstimation.maximumSoh_perc[0]);
}

void testSE_CalculateStateOfHealth(void) {
    static DATA_BLOCK_SOH_s se_tableSohEstimation = {.header.uniqueId = DATA_BLOCK_ID_SOH};
    TEST_ASSERT_FAIL_ASSERT(SE_CalculateStateOfHealth(NULL_PTR));

    SE_CalculateStateOfHealth(&se_tableSohEstimation);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, se_tableSohEstimation.averageSoh_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(49.9f, se_tableSohEstimation.minimumSoh_perc[0]);
    TEST_ASSERT_EQUAL_FLOAT(50.1f, se_tableSohEstimation.maximumSoh_perc[0]);
}
