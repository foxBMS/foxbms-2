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
 * @file    test_mxm_bitextract.c
 * @author  foxBMS Team
 * @date    2020-04-07 (date of creation)
 * @updated 2020-05-14 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test for test_mxm_bitextract.c
 *
 * @details def
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "mxm_bitextract.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/*
 *  tests for write function
 */
void testWriteValuePositionZero(void) {
    uint8_t reg = 0u;
    TEST_ASSERT_EQUAL(1u, mxm_41bWriteValue(1u, 1, 0, reg));
}

void testWriteValuePositionOne(void) {
    uint8_t reg = 0u;
    TEST_ASSERT_EQUAL(2u, mxm_41bWriteValue(1u, 1, 1, reg));
}

void testWriteValuePositionOneWithBitZeroInRegister(void) {
    uint8_t reg = 1u;
    TEST_ASSERT_EQUAL(3u, mxm_41bWriteValue(1u, 1, 1, reg));
}

void testWriteValueTXPreambles(void) {
    uint8_t reg = 0u;
    TEST_ASSERT_EQUAL((1u << 5u), mxm_41bWriteValue(1u, 1, MXM_41B_TX_PREAMBLES, reg));
}

void testWriteValueKeepAlive(void) {
    uint8_t reg = 0u;
    TEST_ASSERT_EQUAL(0xFu, mxm_41bWriteValue(0xFu, 4, MXM_41B_KEEP_ALIVE, reg));
}

void testWriteValueRXErrorInt(void) {
    uint8_t reg = 0u;
    TEST_ASSERT_EQUAL(128u, mxm_41bWriteValue(1u, 1, MXM_41B_RX_ERROR, reg));
}

void testWriteValueOverflowInt(void) {
    uint8_t reg = 0u;
    TEST_ASSERT_EQUAL(8u, mxm_41bWriteValue(1u, 1, MXM_41B_RX_OVERFLOW_INT_ENABLE, reg));
}

/*
 *  tests for read function
 */
void testReadValuePositionZero(void) {
    uint8_t reg = 1u;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, 0));
}

void testReadValuePositionZeroWithMoreBits(void) {
    uint8_t reg = 0xFu;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, 0));
}

void testReadValuePositionOne(void) {
    uint8_t reg = 2u;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, 1));
}

void testReadValuePositionOneWithMoreBits(void) {
    uint8_t reg = 7u;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, 1));
}

void testReadValueTXPreambles(void) {
    uint8_t reg = 0b100000;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, MXM_41B_TX_PREAMBLES));
}

void testReadValueKeepAlive(void) {
    uint8_t reg = 0b1111;
    TEST_ASSERT_EQUAL(0xFu, mxm_41bReadValue(reg, 4, MXM_41B_KEEP_ALIVE));
}

void testReadValueKeepAliveOne(void) {
    uint8_t reg = 0b1;
    TEST_ASSERT_EQUAL(0x1u, mxm_41bReadValue(reg, 4, MXM_41B_KEEP_ALIVE));
}

void testReadValueKeepAliveFiveBits(void) {
    uint8_t reg = 0b11111;
    TEST_ASSERT_EQUAL(0xFu, mxm_41bReadValue(reg, 4, MXM_41B_KEEP_ALIVE));
}

void testReadValueRXError(void) {
    uint8_t reg = 0b10000000;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, MXM_41B_RX_ERROR));
}

void testReadValueRXOverflowStatus(void) {
    uint8_t reg = 0b1000;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, MXM_41B_RX_OVERFLOW_INT_ENABLE));
}

void testReadValueRXBusy(void) {
    uint8_t reg = 0b100000;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, MXM_41B_RX_BUSY_STATUS));
}

void testReadValueRXEmptyStatus(void) {
    uint8_t reg = 0b1;
    TEST_ASSERT_EQUAL(1u, mxm_41bReadValue(reg, 1, MXM_41B_RX_EMPTY_STATUS));
}
