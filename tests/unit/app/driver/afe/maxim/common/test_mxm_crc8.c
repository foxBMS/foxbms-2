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
 * @file    test_mxm_crc8.c
 * @author  foxBMS Team
 * @date    2020-03-13 (date of creation)
 * @updated 2020-04-27 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  MXM
 *
 * @brief   Test for the Maxim CRC8 implementation.
 *
 * @details def
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "mxm_crc8.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCRC8SelfCheckShouldPass(void) {
    TEST_ASSERT_PASS_ASSERT(TEST_ASSERT_EQUAL_HEX8(STD_OK, MXM_CRC8SelfTest()));
}

void testCRC8Sequence1(void) {
    uint16_t test_sequence[4] = {0x02u, 0x12u, 0xB1u, 0xB2u};
    TEST_ASSERT_EQUAL(0xC4u, MXM_CRC8(test_sequence, 4));
}

void testCRC8Sequence2(void) {
    uint16_t test_sequence[3] = {0x03u, 0x12u, 0x00u};
    TEST_ASSERT_EQUAL(0xCBu, MXM_CRC8(test_sequence, 3));
}

void testCRC8Sequence3(void) {
    uint16_t test_sequence[10] = {0x02u, 0x5Bu, 0x12u, 0x42u, 0xFFu, 0xD3u, 0x13u, 0x77u, 0xA1u, 0x31u};
    TEST_ASSERT_EQUAL(0x7Eu, MXM_CRC8(test_sequence, 10));
}

void testCRC8Sequence4(void) {
    uint16_t test_sequence[3] = {0x03u, 0x66u, 0x00u};
    TEST_ASSERT_EQUAL(0x43u, MXM_CRC8(test_sequence, 3));
}
