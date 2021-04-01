/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    foxmath.c
 * @author  foxBMS Team
 * @date    2018-01-18 (date of creation)
 * @updated 2018-01-18 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MATH
 *
 * @brief   mathlib function implementations
 *
 */

/*========== Includes =======================================================*/
#include "foxmath.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern float MATH_linearInterpolation(float x1, float y1, float x2, float y2, float x_interpolate) {
    float y_interpolate = 0.0f;
    float slope         = 0.0f;

    if (x1 != x2) {
        /* Calculate slope */
        slope = (y2 - y1) / (x2 - x1);
    } else {
        /* x values are identical -> no interpolation possible: return y1 value */
        slope = 0;
    }
    /* Interpolate starting from x1/y1 */
    y_interpolate = y1 + (slope * (x_interpolate - x1));

    return y_interpolate;
}

extern uint16_t MATH_swapBytes_uint16_t(uint16_t val) {
    return (val << 8) | (val >> 8);
}

extern uint32_t MATH_swapBytes_uint32_t(uint32_t val) {
    val = ((val << 8) & 0xFF00FF00u) | ((val >> 8) & 0xFF00FFu);
    return (val << 16) | (val >> 16);
}

extern uint64_t MATH_swapBytes_uint64_t(uint64_t val) {
    val = ((val << 8) & 0xFF00FF00FF00FF00ull) | ((val >> 8) & 0x00FF00FF00FF00FFull);
    val = ((val << 16) & 0xFFFF0000FFFF0000ull) | ((val >> 16) & 0x0000FFFF0000FFFFull);
    return (val << 32) | (val >> 32);
}

extern float MATH_minimumOfTwoFloats(float value1, float value2) {
    return fminf(value1, value2);
}

extern int32_t MATH_AbsInt32(int32_t value) {
    int32_t absValue = INT32_MAX;
    if (value != INT32_MIN) {
        absValue = labs(value);
    }
    return absValue;
}

extern int64_t MATH_AbsInt64(int64_t value) {
    int64_t absValue = INT64_MAX;
    if (value != INT64_MIN) {
        absValue = llabs(value);
    }
    return absValue;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
