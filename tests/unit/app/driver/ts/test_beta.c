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
 * @file    test_beta.c
 * @author  foxBMS Team
 * @date    2020-03-13 (date of creation)
 * @updated 2021-03-22 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  BETA
 *
 * @brief   Test of the beta.c module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "beta.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testBETA_TemperatureFromResistanceFixedValues(void) {
    TEST_ASSERT_EQUAL_INT16(2318, BETA_TemperatureFromResistance(42.0f));
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, BETA_TemperatureFromResistance(0.0f));
    TEST_ASSERT_EQUAL_INT16(-2302, BETA_TemperatureFromResistance(__FLT_MAX__));

    /* BETA_TemperatureFromResistance returns FLT_MAX for calls with an invalid resistance */
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, BETA_TemperatureFromResistance(-1.0f));
}

void testBETA_ResistanceFromTemperatureFixedValues(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.005f, 0.0f, BETA_ResistanceFromTemperature(INT16_MIN));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.05f, BETA_ResistanceFromTemperature(INT16_MAX));
    TEST_ASSERT_EQUAL_FLOAT(681.714111f, BETA_ResistanceFromTemperature(1000));
}

void testBETA_GetTemperatureFromBetaFixedValues(void) {
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, BETA_GetTemperatureFromBeta(UINT16_MAX));
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, BETA_GetTemperatureFromBeta(0u));
    TEST_ASSERT_EQUAL_INT16(525, BETA_GetTemperatureFromBeta(1000u));
    TEST_ASSERT_EQUAL_INT16(260, BETA_GetTemperatureFromBeta(2000u));
}
