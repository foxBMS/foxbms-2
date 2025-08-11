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
 * @file    test_soa_cfg.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test for the configuration for SOX
 * @details Tests Current Limit and Current on open String
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbattery_cell_cfg.h"
#include "Mockbms.h"

#include "soa_cfg.h"

#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/task/config")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testDummy(void) {
}

void testSOA_IsPackCurrentLimitViolated(void) {
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(0u, BMS_CHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(0u, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(0u, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(0u, BMS_AT_REST));

    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA, BMS_CHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA, BMS_AT_REST));

    TEST_ASSERT_EQUAL(true, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA + 1u, BMS_CHARGING));
    TEST_ASSERT_EQUAL(true, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA + 1u, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA + 1u, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(BS_MAXIMUM_PACK_CURRENT_mA + 1u, BMS_AT_REST));

    TEST_ASSERT_EQUAL(true, SOA_IsPackCurrentLimitViolated(UINT32_MAX, BMS_CHARGING));
    TEST_ASSERT_EQUAL(true, SOA_IsPackCurrentLimitViolated(UINT32_MAX, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(UINT32_MAX, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsPackCurrentLimitViolated(UINT32_MAX, BMS_AT_REST));
}

void testSOA_IsStringCurrentLimitViolated(void) {
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(0u, BMS_CHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(0u, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(0u, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(0u, BMS_AT_REST));

    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA, BMS_CHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA, BMS_AT_REST));

    TEST_ASSERT_EQUAL(true, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA + 1u, BMS_CHARGING));
    TEST_ASSERT_EQUAL(true, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA + 1u, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA + 1u, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(BS_MAXIMUM_STRING_CURRENT_mA + 1u, BMS_AT_REST));

    TEST_ASSERT_EQUAL(true, SOA_IsStringCurrentLimitViolated(UINT32_MAX, BMS_CHARGING));
    TEST_ASSERT_EQUAL(true, SOA_IsStringCurrentLimitViolated(UINT32_MAX, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(UINT32_MAX, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsStringCurrentLimitViolated(UINT32_MAX, BMS_AT_REST));
}

void testSOA_IsCellCurrentLimitViolated(void) {
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(0u, BMS_CHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(0u, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(0u, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(0u, BMS_AT_REST));

    uint32_t chargeLimit    = BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK * BC_CURRENT_MAX_CHARGE_MSL_mA;
    uint32_t dischargeLimit = BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK * BC_CURRENT_MAX_DISCHARGE_MSL_mA;

    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(chargeLimit, BMS_CHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(dischargeLimit, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(chargeLimit, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(chargeLimit, BMS_AT_REST));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(dischargeLimit, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(dischargeLimit, BMS_AT_REST));

    TEST_ASSERT_EQUAL(true, SOA_IsCellCurrentLimitViolated(chargeLimit + 1u, BMS_CHARGING));
    TEST_ASSERT_EQUAL(true, SOA_IsCellCurrentLimitViolated(dischargeLimit + 1u, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(chargeLimit + 1u, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(chargeLimit + 1u, BMS_AT_REST));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(dischargeLimit + 1u, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(dischargeLimit + 1u, BMS_AT_REST));

    TEST_ASSERT_EQUAL(true, SOA_IsCellCurrentLimitViolated(UINT32_MAX, BMS_CHARGING));
    TEST_ASSERT_EQUAL(true, SOA_IsCellCurrentLimitViolated(UINT32_MAX, BMS_DISCHARGING));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(UINT32_MAX, BMS_RELAXATION));
    TEST_ASSERT_EQUAL(false, SOA_IsCellCurrentLimitViolated(UINT32_MAX, BMS_AT_REST));
}

void testSOA_IsCurrentOnOpenString(void) {
    TEST_ASSERT_FAIL_ASSERT(SOA_IsCurrentOnOpenString(BMS_CHARGING, BS_NR_OF_STRINGS));

    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringPrecharging_ExpectAndReturn(0u, false);
    TEST_ASSERT_EQUAL(true, SOA_IsCurrentOnOpenString(BMS_CHARGING, 0u));

    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringPrecharging_ExpectAndReturn(0u, false);
    TEST_ASSERT_EQUAL(false, SOA_IsCurrentOnOpenString(BMS_CHARGING, 0u));

    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringPrecharging_ExpectAndReturn(0u, true);
    TEST_ASSERT_EQUAL(false, SOA_IsCurrentOnOpenString(BMS_CHARGING, 0u));

    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringPrecharging_ExpectAndReturn(0u, false);
    TEST_ASSERT_EQUAL(true, SOA_IsCurrentOnOpenString(BMS_DISCHARGING, 0u));

    BMS_IsStringClosed_ExpectAndReturn(0u, true);
    BMS_IsStringPrecharging_ExpectAndReturn(0u, false);
    TEST_ASSERT_EQUAL(false, SOA_IsCurrentOnOpenString(BMS_DISCHARGING, 0u));

    BMS_IsStringClosed_ExpectAndReturn(0u, false);
    BMS_IsStringPrecharging_ExpectAndReturn(0u, true);
    TEST_ASSERT_EQUAL(false, SOA_IsCurrentOnOpenString(BMS_DISCHARGING, 0u));

    TEST_ASSERT_EQUAL(false, SOA_IsCurrentOnOpenString(BMS_RELAXATION, 0u));
}
