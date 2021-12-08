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
 * @file    test_vishay_ntcle317e4103sba.c
 * @author  foxBMS Team
 * @date    2021-11-02 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the vishay_ntcle317e4103sba module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "foxmath.h"
#include "test_assert_helper.h"
#include "vishay_ntcle317e4103sba.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testTS_Vis01GetTemperatureFromLutFixedValues(void) {
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, TS_Vis01GetTemperatureFromLut(4000u));
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, TS_Vis01GetTemperatureFromLut(0u));
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, TS_Vis01GetTemperatureFromLut(3000u));
    TEST_ASSERT_EQUAL_INT16(-548, TS_Vis01GetTemperatureFromLut(2967u));
    TEST_ASSERT_EQUAL_INT16(1232, TS_Vis01GetTemperatureFromLut(100u));
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, TS_Vis01GetTemperatureFromLut(10u));
}

/** Polynomial is not implemeneted for this sensor */
void testTS_Vis01GetTemperatureFromPolynomialFixedValues(void) {
    TEST_ASSERT_FAIL_ASSERT(TS_Vis01GetTemperatureFromPolynomial(1u));
}
