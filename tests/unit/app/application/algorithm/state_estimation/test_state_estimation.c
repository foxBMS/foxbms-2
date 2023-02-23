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
 * @file    test_state_estimation.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for SOH module responsible for calculation of state-of-health
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"

#include "soc_none.h"
#include "soe_none.h"
#include "soh_none.h"
#include "state_estimation.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/** test invalid input on interfaces */
void testInvalidInput(void) {
    DATA_BLOCK_SOX_s table_test = {.header.uniqueId = DATA_BLOCK_ID_SOX};
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeSoc(true, BS_NR_OF_STRINGS));

    TEST_ASSERT_FAIL_ASSERT(SE_InitializeSoe(true, BS_NR_OF_STRINGS));

    TEST_ASSERT_FAIL_ASSERT(SE_InitializeSoh(BS_NR_OF_STRINGS));

    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfCharge(NULL_PTR, true, BS_NR_OF_STRINGS - 1u));
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfCharge(&table_test, true, BS_NR_OF_STRINGS));

    TEST_ASSERT_FAIL_ASSERT(SE_CalculateStateOfCharge(NULL_PTR));

    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfEnergy(NULL_PTR, true, BS_NR_OF_STRINGS - 1u));
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfEnergy(&table_test, true, BS_NR_OF_STRINGS));

    TEST_ASSERT_FAIL_ASSERT(SE_CalculateStateOfEnergy(NULL_PTR));

    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfHealth(NULL_PTR, BS_NR_OF_STRINGS - 1u));
    TEST_ASSERT_FAIL_ASSERT(SE_InitializeStateOfHealth(&table_test, BS_NR_OF_STRINGS));

    TEST_ASSERT_FAIL_ASSERT(SE_CalculateStateOfHealth(NULL_PTR));
}
