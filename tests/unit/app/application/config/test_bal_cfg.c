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
 * @file    test_bal_cfg.c
 * @author  foxBMS Team
 * @date    2022-02-26 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test for the balancing configuration
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockos.h"

#include "bal_cfg.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    BAL_SetBalancingThreshold(BAL_DEFAULT_THRESHOLD_mV);
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/** test #BAL_SetBalancingThreshold() and #BAL_GetBalancingThreshold_mV() */
void testSetAndGetBalancingThreshold(void) {
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    int32_t expectedThreshold_mV = 50;
    BAL_SetBalancingThreshold(expectedThreshold_mV);
    TEST_ASSERT_EQUAL(expectedThreshold_mV, BAL_GetBalancingThreshold_mV());

    expectedThreshold_mV = 150;
    BAL_SetBalancingThreshold(expectedThreshold_mV);
    TEST_ASSERT_EQUAL(expectedThreshold_mV, BAL_GetBalancingThreshold_mV());
}

/** test #BAL_SetBalancingThreshold() with maximum values */
void testSetBalancingThresholdMaxValue(void) {
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    int32_t expectedThreshold_mV = BAL_MAXIMUM_THRESHOLD_mV + 1;
    BAL_SetBalancingThreshold(expectedThreshold_mV);
    TEST_ASSERT_EQUAL(expectedThreshold_mV - 1, BAL_GetBalancingThreshold_mV());
}

/** test #BAL_SetBalancingThreshold() with minimum values */
void testSetBalancingThresholdMinValue(void) {
    OS_EnterTaskCritical_Ignore();
    OS_ExitTaskCritical_Ignore();
    int32_t expectedThreshold_mV = BAL_MINIMUM_THRESHOLD_mV - 1;
    BAL_SetBalancingThreshold(expectedThreshold_mV);
    TEST_ASSERT_EQUAL(expectedThreshold_mV + 1, BAL_GetBalancingThreshold_mV());
}
