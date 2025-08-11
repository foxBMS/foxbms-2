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
 * @file    foxmath.c
 * @author  foxBMS Team
 * @date    2018-01-18 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  MATH
 *
 * @brief   Mathlib function implementations
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "foxmath.h"

#include "fassert.h"
#include "utils.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/* AXIVION Disable Style Generic-MissingParameterAssert: If not specified otherwise, functions in this lib are
   designed to take full range input. No assert needed. */

extern void MATH_StartupSelfTest(void) {
    /* The non-asserting branch of this function is not meaningful testable in
     * unit tests, therefore this branch is excluded in coverage analysis to
     * still reach full test coverage.
     * The passing test is still part of the unit test suite.
     * If something is changed in this function, this needs to be reevaluated.
     * See function
     * tests/unit/app/driver/foxmath/test_foxmath.c:testMATH_StartupSelfTest
     * for the actual unit test.
     */
    FAS_ASSERT(MATH_AbsInt64_t(INT64_MIN) == INT64_MAX); /* LCOV_EXCL_LINE */
    FAS_ASSERT(MATH_AbsInt32_t(INT32_MIN) == INT32_MAX); /* LCOV_EXCL_LINE */
}

extern float_t MATH_LinearInterpolation(
    const float_t x1,
    const float_t y1,
    const float_t x2,
    const float_t y2,
    const float_t x_interpolate) {
    float_t slope = 0.0f;

    if (fabsf(x1 - x2) >= FLT_EPSILON) {
        /* Calculate slope */
        slope = (y2 - y1) / (x2 - x1);
    }
    /* In the case that the if clause is not entered (x values are identical)
     * -> no interpolation possible: return y1 value
     * -> slope takes initialization value of 0.0f
     */

    /* Interpolate starting from x1/y1 */
    float_t y_interpolate = y1 + (slope * (x_interpolate - x1));

    return y_interpolate;
}

extern uint16_t MATH_SwapBytesUint16_t(const uint16_t val) {
    return (val << UTIL_SHIFT_ONE_BYTE) | (val >> UTIL_SHIFT_ONE_BYTE);
}

extern uint32_t MATH_SwapBytesUint32_t(const uint32_t val) {
    const uint32_t alternating2PatternStartFF = 0xFF00FF00u;
    const uint32_t alternating2PatternStart00 = 0x00FF00FFu;
    const uint32_t intermediate               = ((val << UTIL_SHIFT_ONE_BYTE) & alternating2PatternStartFF) |
                                  ((val >> UTIL_SHIFT_ONE_BYTE) & alternating2PatternStart00);
    return (intermediate << UTIL_SHIFT_TWO_BYTES) | (intermediate >> UTIL_SHIFT_TWO_BYTES);
}

extern uint64_t MATH_SwapBytesUint64_t(const uint64_t val) {
    const uint64_t alternating2PatternStartFF   = 0xFF00FF00FF00FF00uLL;
    const uint64_t alternating2PatternStart00   = 0x00FF00FF00FF00FFuLL;
    const uint64_t alternating4PatternStartFFFF = 0xFFFF0000FFFF0000uLL;
    const uint64_t alternating4PatternStart0000 = 0x0000FFFF0000FFFFuLL;

    uint64_t intermediate = ((val << UTIL_SHIFT_ONE_BYTE) & alternating2PatternStartFF) |
                            ((val >> UTIL_SHIFT_ONE_BYTE) & alternating2PatternStart00);
    intermediate = ((intermediate << UTIL_SHIFT_TWO_BYTES) & alternating4PatternStartFFFF) |
                   ((intermediate >> UTIL_SHIFT_TWO_BYTES) & alternating4PatternStart0000);
    return (intermediate << UTIL_SHIFT_FOUR_BYTES) | (intermediate >> UTIL_SHIFT_FOUR_BYTES);
}

extern float_t MATH_MinimumOfTwoFloats(const float_t value1, const float_t value2) {
    return fminf(value1, value2);
}

extern uint8_t MATH_MinimumOfTwoUint8_t(const uint8_t value1, const uint8_t value2) {
    uint8_t minimumValue = value1;
    if (minimumValue > value2) {
        minimumValue = value2;
    }
    return minimumValue;
}

extern uint16_t MATH_MinimumOfTwoUint16_t(const uint16_t value1, const uint16_t value2) {
    uint16_t minimumValue = value1;
    if (minimumValue > value2) {
        minimumValue = value2;
    }
    return minimumValue;
}

extern int32_t MATH_AbsInt32_t(const int32_t value) {
    int32_t absoluteValue = INT32_MAX;
    if (value != INT32_MIN) {
        absoluteValue = labs(value);
    }
    return absoluteValue;
}

extern int64_t MATH_AbsInt64_t(const int64_t value) {
    int64_t absoluteValue = INT64_MAX;
    if (value != INT64_MIN) {
        absoluteValue = llabs(value);
    }
    return absoluteValue;
}

/* AXIVION Enable Style Generic-MissingParameterAssert: */

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
