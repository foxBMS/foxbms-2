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
 * @file    test_foxmath.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2021-08-06 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the foxmath module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"

#include "foxmath.h"

/*========== Definitions and Implementations for Unit Test ==================*/
uint16_t val16;
uint32_t val32;
uint64_t val64;
float coord_x1;
float coord_x2;
float coord_y1;
float coord_y2;
float coord_x_interpolate;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    val16               = 0u;
    val32               = 0u;
    val64               = 0u;
    coord_x1            = 10.f;
    coord_y1            = 50.f;
    coord_x2            = 20.f;
    coord_y2            = 100.f;
    coord_x_interpolate = 15.f;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void test_linearInterpolation_X1EqualsX2() {
    coord_x1 = coord_x2;
    TEST_ASSERT_EQUAL(coord_y1, MATH_LinearInterpolation(coord_x1, coord_y1, coord_x2, coord_y2, coord_x_interpolate));
}

void test_linearInterpolation_interpolateBetweenX1AndX2() {
    TEST_ASSERT_EQUAL(75.0f, MATH_LinearInterpolation(10.f, 50.f, 20.f, 100.f, 15.f));
    TEST_ASSERT_EQUAL(87.0f, MATH_LinearInterpolation(10.f, 50.f, 20.f, 100.f, 17.5f));
    TEST_ASSERT_EQUAL(50.0f, MATH_LinearInterpolation(10.f, 50.f, 20.f, 100.f, 10.0001f));
}

void test_linearInterpolation_extrapolateFromX1AndX2() {
    TEST_ASSERT_EQUAL(100.0f, MATH_LinearInterpolation(10.f, 50.f, 20.f, 100.f, 20.1f));
    TEST_ASSERT_EQUAL(-100.0f, MATH_LinearInterpolation(10.f, 50.f, 20.f, 100.f, -20.f));
    TEST_ASSERT_EQUAL(16739465.0f, MATH_LinearInterpolation(10.f, 50.f, 20.f, 100.f, 3347893.0f));
}

void test_allZeros_swap16(void) {
    TEST_ASSERT_EQUAL(0u, MATH_SwapBytesUint16_t(val16));
}

void test_allZeros_swap32(void) {
    TEST_ASSERT_EQUAL(0u, MATH_SwapBytesUint32_t(val32));
}

void test_allZeros_swap64(void) {
    TEST_ASSERT_EQUAL(0u, MATH_SwapBytesUint64_t(val64));
}

void test_swap16(void) {
    val16 = 786u; /* any random value */
    TEST_ASSERT_EQUAL(4611u, MATH_SwapBytesUint16_t(val16));
}

void test_swap32(void) {
    val32 = 0xFFFF0000u; /* any random value */
    TEST_ASSERT_EQUAL(0x0000FFFFu, MATH_SwapBytesUint32_t(val32));
}

void test_swap64(void) {
    val64 = 123u; /* 0x007B */
    TEST_ASSERT_EQUAL(0x7B00000000000000u, MATH_SwapBytesUint64_t(val64));
}

void test_swap16_MAX(void) {
    val16 = UINT16_MAX;
    TEST_ASSERT_EQUAL(UINT16_MAX, MATH_SwapBytesUint16_t(val16));
}

void test_swap32_MAX(void) {
    val32 = UINT32_MAX;
    TEST_ASSERT_EQUAL(UINT32_MAX, MATH_SwapBytesUint32_t(val32));
}

void test_swap64_MAX(void) {
    val64 = UINT64_MAX;
    TEST_ASSERT_EQUAL(UINT64_MAX, MATH_SwapBytesUint64_t(val64));
}

void test_minimumOfTwoFloats(void) {
    float minimum = 0.0f;
    /* Test 1: all values are equal */
    minimum = MATH_MinimumOfTwoFloats(1.0f, 1.0f);
    TEST_ASSERT_EQUAL(1.0f, minimum);
    /* Test 2: the first value is the smallest */
    minimum = MATH_MinimumOfTwoFloats(1.0f, 2.0f);
    TEST_ASSERT_EQUAL(1.0f, minimum);
    /* Test 3: the last value is the smallest */
    minimum = MATH_MinimumOfTwoFloats(3.0f, 2.0f);
    TEST_ASSERT_EQUAL(2.0f, minimum);
    /* Test 4: the first value is the smallest */
    minimum = MATH_MinimumOfTwoFloats(-3.0f, 1.0f);
    TEST_ASSERT_EQUAL(-3.0f, minimum);
    /* Test 5: the first value is the smallest */
    minimum = MATH_MinimumOfTwoFloats(-3.0f, -1.0f);
    TEST_ASSERT_EQUAL(-3.0f, minimum);
}

void test_MATH_MinimumOfTwoUint8_t(void) {
    uint8_t value1 = 42u;
    uint8_t value2 = 67u;

    TEST_ASSERT_EQUAL_UINT8(value1, MATH_MinimumOfTwoUint8_t(value1, value2));
    TEST_ASSERT_EQUAL_UINT8(value1, MATH_MinimumOfTwoUint8_t(value2, value1));
    TEST_ASSERT_EQUAL_UINT8(value1, MATH_MinimumOfTwoUint8_t(value1, UINT8_MAX));
    TEST_ASSERT_EQUAL_UINT8(0u, MATH_MinimumOfTwoUint8_t(value1, 0u));
    TEST_ASSERT_EQUAL_UINT8(0u, MATH_MinimumOfTwoUint8_t(UINT8_MAX, 0u));
    TEST_ASSERT_EQUAL_UINT8(0u, MATH_MinimumOfTwoUint8_t(0u, UINT8_MAX));
}

void test_MATH_MinimumOfTwoUint16_t(void) {
    uint16_t value1 = 0xFF42u;
    uint16_t value2 = 0xFF67u;

    TEST_ASSERT_EQUAL_UINT16(value1, MATH_MinimumOfTwoUint16_t(value1, value2));
    TEST_ASSERT_EQUAL_UINT16(value1, MATH_MinimumOfTwoUint16_t(value2, value1));
    TEST_ASSERT_EQUAL_UINT16(value1, MATH_MinimumOfTwoUint16_t(value1, UINT16_MAX));
    TEST_ASSERT_EQUAL_UINT16(0u, MATH_MinimumOfTwoUint16_t(value1, 0u));
    TEST_ASSERT_EQUAL_UINT16(0u, MATH_MinimumOfTwoUint16_t(UINT16_MAX, 0u));
    TEST_ASSERT_EQUAL_UINT16(0u, MATH_MinimumOfTwoUint16_t(0u, UINT16_MAX));
}
