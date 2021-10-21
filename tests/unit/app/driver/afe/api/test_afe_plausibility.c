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
 * @file    test_afe_plausibility.c
 * @author  foxBMS Team
 * @date    2020-07-13 (date of creation)
 * @updated 2021-01-15 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the afe_plausibility.c module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mocktsi.h"

#include "afe_plausibility.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/** generic implementation of cell voltage limits */
AFE_PLAUSIBILITY_VALUES_s testGenericLimits = {
    .maximumPlausibleVoltage_mV = 5000,
    .minimumPlausibleVoltage_mV = 0,
};

/** generic implementation of cell voltage limits with negative limits */
AFE_PLAUSIBILITY_VALUES_s testGenericLimitsNegative = {
    .maximumPlausibleVoltage_mV = 5000,
    .minimumPlausibleVoltage_mV = -5000,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/** check sanity-check of voltage plausibility function
 *
 * limits may not be the same and must be ascending
 */
void testAFE_PlausibilityCheckVoltageMeasurementRangeLimitSanity(void) {
    AFE_PLAUSIBILITY_VALUES_s limitsEqualValues = {
        .maximumPlausibleVoltage_mV = 42,
        .minimumPlausibleVoltage_mV = 42,
    };

    AFE_PLAUSIBILITY_VALUES_s limitsDescendingOrder = {
        .maximumPlausibleVoltage_mV = INT16_MIN,
        .minimumPlausibleVoltage_mV = INT16_MAX,
    };

    AFE_PLAUSIBILITY_VALUES_s limitsAscendingOrder = {
        .maximumPlausibleVoltage_mV = INT16_MAX,
        .minimumPlausibleVoltage_mV = INT16_MIN,
    };

    TEST_ASSERT_FAIL_ASSERT(AFE_PlausibilityCheckVoltageMeasurementRange(0, limitsEqualValues));
    TEST_ASSERT_FAIL_ASSERT(AFE_PlausibilityCheckVoltageMeasurementRange(0, limitsDescendingOrder));
    TEST_ASSERT_PASS_ASSERT(
        TEST_ASSERT_EQUAL(STD_OK, AFE_PlausibilityCheckVoltageMeasurementRange(0, limitsAscendingOrder)));
}

/** equivalence input classes for voltage check:
 *
 *  cell voltage value:
 *     * in-range-value: e.g. 3500
 *     * out-of-upper-range-value: e.g. INT16_MAX
 *     * out-of-lower-range-value: e.g. INT16_MIN
 *     * just to be sure: 0
 *
 *  limits:
 *     * negative values
 *     * positive values
 *     * zero
 */
void testAFE_PlausibilityCheckVoltageMeasurementRange(void) {
    TEST_ASSERT_EQUAL(STD_OK, AFE_PlausibilityCheckVoltageMeasurementRange(3500, testGenericLimits));
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_PlausibilityCheckVoltageMeasurementRange(INT16_MAX, testGenericLimits));
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_PlausibilityCheckVoltageMeasurementRange(INT16_MIN, testGenericLimits));
    TEST_ASSERT_EQUAL(STD_OK, AFE_PlausibilityCheckVoltageMeasurementRange(0, testGenericLimits));

    /* test around the upper threshold with #testGenericLimits */
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimits.maximumPlausibleVoltage_mV + 1, testGenericLimits));
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(testGenericLimits.maximumPlausibleVoltage_mV, testGenericLimits));
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimits.maximumPlausibleVoltage_mV - 1, testGenericLimits));

    /* test around the lower threshold with #testGenericLimits */
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimits.minimumPlausibleVoltage_mV + 1, testGenericLimits));
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(testGenericLimits.minimumPlausibleVoltage_mV, testGenericLimits));
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimits.minimumPlausibleVoltage_mV - 1, testGenericLimits));

    /* test around the upper threshold with #testGenericLimitsNegative */
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimitsNegative.maximumPlausibleVoltage_mV + 1, testGenericLimitsNegative));
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimitsNegative.maximumPlausibleVoltage_mV, testGenericLimitsNegative));
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimitsNegative.maximumPlausibleVoltage_mV - 1, testGenericLimitsNegative));

    /* test around the lower threshold with #testGenericLimitsNegative */
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimitsNegative.minimumPlausibleVoltage_mV + 1, testGenericLimitsNegative));
    TEST_ASSERT_EQUAL(
        STD_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimitsNegative.minimumPlausibleVoltage_mV, testGenericLimitsNegative));
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        AFE_PlausibilityCheckVoltageMeasurementRange(
            testGenericLimitsNegative.minimumPlausibleVoltage_mV - 1, testGenericLimitsNegative));
}

/** check if the function handles bad input from the API as expected */
void testAFE_PlausibilityCheckTempMinMaxBadInputFromAPI(void) {
    TSI_GetMaximumPlausibleTemperature_ExpectAndReturn(50);
    TSI_GetMinimumPlausibleTemperature_ExpectAndReturn(51);
    TEST_ASSERT_FAIL_ASSERT(AFE_PlausibilityCheckTempMinMax(0));

    TSI_GetMaximumPlausibleTemperature_ExpectAndReturn(50);
    TSI_GetMinimumPlausibleTemperature_ExpectAndReturn(50);
    TEST_ASSERT_PASS_ASSERT(AFE_PlausibilityCheckTempMinMax(0));

    TSI_GetMaximumPlausibleTemperature_ExpectAndReturn(-50);
    TSI_GetMinimumPlausibleTemperature_ExpectAndReturn(-51);
    TEST_ASSERT_PASS_ASSERT(AFE_PlausibilityCheckTempMinMax(0));

    TSI_GetMaximumPlausibleTemperature_ExpectAndReturn(51);
    TSI_GetMinimumPlausibleTemperature_ExpectAndReturn(50);
    TEST_ASSERT_PASS_ASSERT(AFE_PlausibilityCheckTempMinMax(0));
}

/** check if the function handles plausible and implausible input as expected
   equivalence classes:
   * below lower limit -->  FAIL
   * on lower limit -->     PASS
   * between lower and upper limit -->      PASS
   * on upper limit -->     PASS
   * above upper limit -->  FAIL
 */
void testAFE_PlausibilityCheckTempMinMaxVerify(void) {
    const int16_t upperLimit_ddegC = 100;
    const int16_t lowerLimit_ddegC = -10;
    TSI_GetMaximumPlausibleTemperature_IgnoreAndReturn(upperLimit_ddegC);
    TSI_GetMinimumPlausibleTemperature_IgnoreAndReturn(lowerLimit_ddegC);

    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_PlausibilityCheckTempMinMax(INT16_MIN));
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_PlausibilityCheckTempMinMax(lowerLimit_ddegC - 1));
    TEST_ASSERT_EQUAL(STD_OK, AFE_PlausibilityCheckTempMinMax(lowerLimit_ddegC));
    TEST_ASSERT_EQUAL(STD_OK, AFE_PlausibilityCheckTempMinMax((lowerLimit_ddegC + upperLimit_ddegC) / 2));
    TEST_ASSERT_EQUAL(STD_OK, AFE_PlausibilityCheckTempMinMax(upperLimit_ddegC));
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_PlausibilityCheckTempMinMax(upperLimit_ddegC + 1));
    TEST_ASSERT_EQUAL(STD_NOT_OK, AFE_PlausibilityCheckTempMinMax(INT16_MAX));
}
