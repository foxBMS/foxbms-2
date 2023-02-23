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
 * @file    test_ltc_pec.c
 * @author  foxBMS Team
 * @date    2022-11-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the ltc_pec.c module in ltc
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "fstd_types.h"
#include "ltc_pec.h"
#include "test_assert_helper.h"

TEST_FILE("ltc_pec.c")

/*========== Definitions and Implementations for Unit Test ==================*/

/* test for PEC computation on command */
uint8_t byte2Test0[2u] = {0x12u, 0x34u};
uint8_t byte2Test1[2u] = {0xBEu, 0xEFu};
uint8_t byte2Test2[2u] = {0x00u, 0x01u};
uint8_t byte2Test3[2u] = {0x07u, 0x23u};
uint8_t byte2Test4[2u] = {0x03u, 0x60u};
/* test for PEC computation on 6 bytes of data */
uint8_t byte6Test0[6u] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu};
uint8_t byte6Test1[6u] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
uint8_t byte6Test2[6u] = {0xBEu, 0xEFu, 0xBEu, 0xEFu, 0xBEu, 0xEFu};
uint8_t byte6Test3[6u] = {0xA0u, 0xB0u, 0xC0u, 0xD0u, 0xE0u, 0xF0u};
uint8_t byte6Test4[6u] = {0x01u, 0x2Eu, 0x9Bu, 0x33u, 0xACu, 0x55u};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testLtcPec15(void) {
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(2u, byte2Test0), 0x1DD4u);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(2u, byte2Test1), 0xFD2Cu);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(2u, byte2Test2), 0x3D6Eu);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(2u, byte2Test3), 0xB9E4u);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(2u, byte2Test4), 0xF46Cu);

    TEST_ASSERT_EQUAL(LTC_CalculatePec15(6u, byte6Test0), 0xBC24u);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(6u, byte6Test1), 0x664Cu);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(6u, byte6Test2), 0x845Au);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(6u, byte6Test3), 0x792Cu);
    TEST_ASSERT_EQUAL(LTC_CalculatePec15(6u, byte6Test4), 0xD522u);
}

void testLtcPec15InvalidInput(void) {
    /* invalid data length */
    TEST_ASSERT_FAIL_ASSERT(LTC_CalculatePec15(0, byte2Test0));
    /* invalid data */
    TEST_ASSERT_FAIL_ASSERT(LTC_CalculatePec15(1, NULL_PTR));
}
