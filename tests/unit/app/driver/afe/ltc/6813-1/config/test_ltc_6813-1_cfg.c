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
 * @file    test_ltc_6813-1_cfg.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests of the LTC LTC6813-1 configuration
 * @details Tests get voltage input index from cell block index
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mocktsi.h"

#include "ltc_6813-1_cfg.h"

#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("ltc_6813-1_cfg.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/6813-1/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common")
TEST_INCLUDE_PATH("../../src/app/driver/afe/ltc/common/config")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testLTC_GetVoltageInputIndexFromCellBlockIndex(void) {
    /* Assertion Test 1: Invalid index */
    TEST_ASSERT_FAIL_ASSERT(LTC_GetVoltageInputIndexFromCellBlockIndex(BS_NR_OF_CELL_BLOCKS));

    /* Assertion Test 2: Valid index */
    TEST_ASSERT_PASS_ASSERT(LTC_GetVoltageInputIndexFromCellBlockIndex(0u));

    /* Routine test 1: As defined in ltc-6813-1_cfg.c */
    for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
        TEST_ASSERT_EQUAL_UINT8(LTC_GetVoltageInputIndexFromCellBlockIndex(cb), cb);
    }
}
