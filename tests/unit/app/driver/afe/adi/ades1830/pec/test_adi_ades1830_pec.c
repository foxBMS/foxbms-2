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
 * @file    test_adi_ades1830_pec.c
 * @author  foxBMS Team
 * @date    2022-11-04 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of ADI PEC computations
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "adi_ades183x_pec.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades183x_pec.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/ades1830")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/pec")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/spi")

/*========== Definitions and Implementations for Unit Test ==================*/

/* Test for PEC15 computation on command */
uint8_t pec15Byte2Test0[2u] = {0x12u, 0x34u};
uint8_t pec15Byte2Test1[2u] = {0xBEu, 0xEFu};
uint8_t pec15Byte2Test2[2u] = {0x00u, 0x01u};
uint8_t pec15Byte2Test3[2u] = {0x07u, 0x23u};
uint8_t pec15Byte2Test4[2u] = {0x03u, 0x60u};
/* Test for PEC15 computation on 6 bytes of data */
uint8_t pec15Byte6Test0[6u] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu};
uint8_t pec15Byte6Test1[6u] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
uint8_t pec15Byte6Test2[6u] = {0xBEu, 0xEFu, 0xBEu, 0xEFu, 0xBEu, 0xEFu};
uint8_t pec15Byte6Test3[6u] = {0xA0u, 0xB0u, 0xC0u, 0xD0u, 0xE0u, 0xF0u};
uint8_t pec15Byte6Test4[6u] = {0x01u, 0x2Eu, 0x9Bu, 0x33u, 0xACu, 0x55u};

/* Test for PEC10 computation on 6 bytes of data + 6 bits of command counter */
uint8_t pec10Byte7Test0[7u] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xDEu};
uint8_t pec10Byte7Test1[7u] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0x00u};
uint8_t pec10Byte7Test2[7u] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
uint8_t pec10Byte7Test3[7u] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0x00u};
uint8_t pec10Byte7Test4[7u] = {0xBEu, 0xEFu, 0xBEu, 0xEFu, 0xBEu, 0xEFu, 0x35u};
uint8_t pec10Byte7Test5[7u] = {0xBEu, 0xEFu, 0xBEu, 0xEFu, 0xBEu, 0xEFu, 0x00u};
/* Test for PEC10 if command counter bits are handled correctly */
uint8_t pec10Byte7Test6[7u] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0x00u};
uint8_t pec10Byte7Test7[7u] = {0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xFFu};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testAdiPec15(void) {
    TEST_ASSERT_EQUAL(0x1DD4u, ADI_Pec15(2u, pec15Byte2Test0));
    TEST_ASSERT_EQUAL(0xFD2Cu, ADI_Pec15(2u, pec15Byte2Test1));
    TEST_ASSERT_EQUAL(0x3D6Eu, ADI_Pec15(2u, pec15Byte2Test2));
    TEST_ASSERT_EQUAL(0xB9E4u, ADI_Pec15(2u, pec15Byte2Test3));
    TEST_ASSERT_EQUAL(0xF46Cu, ADI_Pec15(2u, pec15Byte2Test4));

    TEST_ASSERT_EQUAL(0xBC24u, ADI_Pec15(6u, pec15Byte6Test0));
    TEST_ASSERT_EQUAL(0x664Cu, ADI_Pec15(6u, pec15Byte6Test1));
    TEST_ASSERT_EQUAL(0x845Au, ADI_Pec15(6u, pec15Byte6Test2));
    TEST_ASSERT_EQUAL(0x792Cu, ADI_Pec15(6u, pec15Byte6Test3));
    TEST_ASSERT_EQUAL(0xD522u, ADI_Pec15(6u, pec15Byte6Test4));
}

void testAdiPec10(void) {
    TEST_ASSERT_EQUAL(0x1CCu, ADI_Pec10(6u, pec10Byte7Test0, false));
    TEST_ASSERT_EQUAL(0x53u, ADI_Pec10(6u, pec10Byte7Test0, true));
    TEST_ASSERT_EQUAL(0x1CCu, ADI_Pec10(6u, pec10Byte7Test1, false));
    TEST_ASSERT_EQUAL(0x1CCu, ADI_Pec10(6u, pec10Byte7Test1, true));

    TEST_ASSERT_EQUAL(0x196u, ADI_Pec10(6u, pec10Byte7Test2, false));
    TEST_ASSERT_EQUAL(0xFEu, ADI_Pec10(6u, pec10Byte7Test2, true));
    TEST_ASSERT_EQUAL(0x196u, ADI_Pec10(6u, pec10Byte7Test3, false));
    TEST_ASSERT_EQUAL(0x196u, ADI_Pec10(6u, pec10Byte7Test3, true));

    TEST_ASSERT_EQUAL(0x3F3u, ADI_Pec10(6u, pec10Byte7Test4, false));
    TEST_ASSERT_EQUAL(0x1B7u, ADI_Pec10(6u, pec10Byte7Test4, true));
    TEST_ASSERT_EQUAL(0x3F3u, ADI_Pec10(6u, pec10Byte7Test5, false));
    TEST_ASSERT_EQUAL(0x3F3u, ADI_Pec10(6u, pec10Byte7Test5, true));

    /* pec10Test6: command counter bits set to 0 and taken into account because the receive */
    /* parameter is set to true */
    /* pec10Test7: command counter bits not set to 0 in data but must be ignored and replaced by 0 */
    /* because the receive parameter is set to false */
    /* So both function calls must return the same result */
    TEST_ASSERT_EQUAL(ADI_Pec10(6u, pec10Byte7Test6, true), ADI_Pec10(6u, pec10Byte7Test7, false));
}

void testAdiPec15Asserts(void) {
    /* invalid data length */
    TEST_ASSERT_FAIL_ASSERT(ADI_Pec15(0, pec15Byte2Test0));
    /* invalid data */
    TEST_ASSERT_FAIL_ASSERT(ADI_Pec15(1, NULL_PTR));
}

void testAdiPec10Asserts(void) {
    /* invalid data length */
    TEST_ASSERT_FAIL_ASSERT(ADI_Pec10(0, pec10Byte7Test0, true));
    /* invalid data */
    TEST_ASSERT_FAIL_ASSERT(ADI_Pec10(1, NULL_PTR, true));
}
