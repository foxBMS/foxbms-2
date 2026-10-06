/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_redundancy-validation.c
 * @author  foxBMS Team
 * @date    2026-07-22 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the redundancy-validation module
 * @details Tests the redundant measurement value validation for cell voltage
 *          and cell temperature measurement values.
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "redundancy_cfg.h"

#include "fstd_types.h"
#include "redundancy-validation.h"
#include "test_assert_helper.h"

#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test extern function #MCR_CheckCellVoltage
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pCellVoltage -> assert
 *          - Routine validation:
 *            - RT1/4: difference == 0 (within tolerance) -> STD_OK, average returned
 *            - RT2/4: difference == tolerance (boundary, within) -> STD_OK, average returned
 *            - RT3/4: difference == tolerance + 1 (positive) -> STD_NOT_OK, average returned
 *            - RT4/4: difference == tolerance + 1 (negative) -> STD_NOT_OK, average returned
 */
void testMCR_CheckCellVoltage(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(MCR_CheckCellVoltage(2000, 2000, NULL_PTR));

    /* ======= Routine tests =============================================== */
    int16_t cellVoltage = 0;

    /* ======= RT1/4: Test implementation: difference == 0 -> STD_OK */
    /* ======= RT1/4: call function under test */
    STD_RETURN_TYPE_e retval0 = MCR_CheckCellVoltage(2000, 2000, &cellVoltage);
    /* ======= RT1/4: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, retval0);
    TEST_ASSERT_EQUAL(2000, cellVoltage);

    /* ======= RT2/4: Test implementation: difference == tolerance -> STD_OK */
    /* ======= RT2/4: call function under test */
    STD_RETURN_TYPE_e retval1 =
        MCR_CheckCellVoltage(2000, (int16_t)(2000 + MCR_CELL_VOLTAGE_TOLERANCE_mV), &cellVoltage);
    /* ======= RT2/4: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, retval1);
    TEST_ASSERT_EQUAL((int16_t)((2000 + (2000 + MCR_CELL_VOLTAGE_TOLERANCE_mV)) / 2), cellVoltage);

    /* ======= RT3/4: Test implementation: difference == tolerance + 1 (positive) -> STD_NOT_OK */
    /* ======= RT3/4: call function under test */
    STD_RETURN_TYPE_e retval2 =
        MCR_CheckCellVoltage(2000, (int16_t)(2000 + MCR_CELL_VOLTAGE_TOLERANCE_mV + 1), &cellVoltage);
    /* ======= RT3/4: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, retval2);
    TEST_ASSERT_EQUAL((int16_t)((2000 + (2000 + MCR_CELL_VOLTAGE_TOLERANCE_mV + 1)) / 2), cellVoltage);

    /* ======= RT4/4: Test implementation: difference == tolerance + 1 (negative) -> STD_NOT_OK */
    /* ======= RT4/4: call function under test */
    STD_RETURN_TYPE_e retval3 =
        MCR_CheckCellVoltage(2000, (int16_t)(2000 - MCR_CELL_VOLTAGE_TOLERANCE_mV - 1), &cellVoltage);
    /* ======= RT4/4: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, retval3);
    TEST_ASSERT_EQUAL((int16_t)((2000 + (2000 - MCR_CELL_VOLTAGE_TOLERANCE_mV - 1)) / 2), cellVoltage);
}

/**
 * @brief   Test extern function #MCR_CheckCellTemperature
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: NULL_PTR for pCellTemperature -> assert
 *          - Routine validation:
 *            - RT1/4: difference == 0 (within tolerance) -> STD_OK, average returned
 *            - RT2/4: difference == tolerance (boundary, within) -> STD_OK, average returned
 *            - RT3/4: difference == tolerance + 1 (positive) -> STD_NOT_OK, average returned
 *            - RT4/4: difference == tolerance + 1 (negative) -> STD_NOT_OK, average returned
 */
void testMCR_CheckCellTemperature(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(MCR_CheckCellTemperature(250, 250, NULL_PTR));

    /* ======= Routine tests =============================================== */
    int16_t cellTemperature = 0;

    /* ======= RT1/4: Test implementation: difference == 0 -> STD_OK */
    /* ======= RT1/4: call function under test */
    STD_RETURN_TYPE_e retval0 = MCR_CheckCellTemperature(250, 250, &cellTemperature);
    /* ======= RT1/4: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, retval0);
    TEST_ASSERT_EQUAL(250, cellTemperature);

    /* ======= RT2/4: Test implementation: difference == tolerance -> STD_OK */
    /* ======= RT2/4: call function under test */
    STD_RETURN_TYPE_e retval1 =
        MCR_CheckCellTemperature(250, (int16_t)(250 + MCR_CELL_TEMPERATURE_TOLERANCE_dK), &cellTemperature);
    /* ======= RT2/4: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, retval1);
    TEST_ASSERT_EQUAL((int16_t)((250 + (250 + MCR_CELL_TEMPERATURE_TOLERANCE_dK)) / 2), cellTemperature);

    /* ======= RT3/4: Test implementation: difference == tolerance + 1 (positive) -> STD_NOT_OK */
    /* ======= RT3/4: call function under test */
    STD_RETURN_TYPE_e retval2 =
        MCR_CheckCellTemperature(250, (int16_t)(250 + MCR_CELL_TEMPERATURE_TOLERANCE_dK + 1), &cellTemperature);
    /* ======= RT3/4: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, retval2);
    TEST_ASSERT_EQUAL((int16_t)((250 + (250 + MCR_CELL_TEMPERATURE_TOLERANCE_dK + 1)) / 2), cellTemperature);

    /* ======= RT4/4: Test implementation: difference == tolerance + 1 (negative) -> STD_NOT_OK */
    /* ======= RT4/4: call function under test */
    STD_RETURN_TYPE_e retval3 =
        MCR_CheckCellTemperature(250, (int16_t)(250 - MCR_CELL_TEMPERATURE_TOLERANCE_dK - 1), &cellTemperature);
    /* ======= RT4/4: test output verification */
    TEST_ASSERT_EQUAL(STD_NOT_OK, retval3);
    TEST_ASSERT_EQUAL((int16_t)((250 + (250 - MCR_CELL_TEMPERATURE_TOLERANCE_dK - 1)) / 2), cellTemperature);
}
