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
 * @file    test_vishay_ntcle413e2103f102l.c
 * @author  foxBMS Team
 * @date    2021-11-02 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the vishay_ntcle413e2103f102l module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "foxmath.h"
#include "test_assert_helper.h"
#include "vishay_ntcle413e2103f102l.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/ts")
TEST_INCLUDE_PATH("../../src/app/driver/ts/vishay/ntcle413e2103f102l")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testTS_Vis02GetTemperatureFromLutFixedValues(void) {
    /* Voltage higher than supply - expected failure - temp on negative boundary */
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, TS_Vis02GetTemperatureFromLut(4000u));
    /* Voltage is zero - expected failure - temp on positive boundary */
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, TS_Vis02GetTemperatureFromLut(0u));
    /* Resistance is high, slightly out of range - expected failure - temp on negative boundary */
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, TS_Vis02GetTemperatureFromLut(2855u));
    /* Resistance is low, slightly out of range - expected failure - temp on positive boundary */
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, TS_Vis02GetTemperatureFromLut(235u));
    /* Resistance is in range - expected success - check expected temp */
    TEST_ASSERT_EQUAL_INT16(-38, TS_Vis02GetTemperatureFromLut(2842u));
    TEST_ASSERT_EQUAL_INT16(103, TS_Vis02GetTemperatureFromLut(248u));
}

/** Polynomial is not implemented for this sensor */
void testTS_Vis02GetTemperatureFromPolynomialFixedValues(void) {
    TEST_ASSERT_FAIL_ASSERT(TS_Vis02GetTemperatureFromPolynomial(1u));
}
