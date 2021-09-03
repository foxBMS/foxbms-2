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
 * @file    foxmath.h
 * @author  foxBMS Team
 * @date    2018-01-18 (date of creation)
 * @updated 2021-08-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MATH
 *
 * @brief   math library for often used math functions
 *
 * @details This header file contains the different math function declarations.
 *          Currently the following functions are supported:
 *          - Slope
 *          - Linear interpolation
 *
 */

#ifndef FOXBMS__FOXMATH_H_
#define FOXBMS__FOXMATH_H_

/*========== Includes =======================================================*/
#include "general.h"

/* AXIVION Disable Style Generic-LocalInclude: foxmath is intended as a library and therefore includes all useful libraries */
#include <float.h>
#include <math.h>
#include <stdlib.h>
/* AXIVION Enable Style Generic-LocalInclude: */

/*========== Macros and Definitions =========================================*/

/** Add defines for unit conversions */
#define UNIT_CONVERSION_FACTOR_1000_INTEGER (1000)
#define UNIT_CONVERSION_FACTOR_10_FLOAT     (10.0f)
#define UNIT_CONVERSION_FACTOR_100_FLOAT    (100.0f)
#define UNIT_CONVERSION_FACTOR_1000_FLOAT   (1000.0f)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief:  self test for math functions that can be called at startup
 *
 *@details: This self test is intended to be called at startup. It will assert
 *          if one of the assumptions is violated. This function can be used
 *          to make sure that features of foxmath that are currently not used
 *          by the code are working nevertheless as expected.
 */
extern void MATH_StartupSelfTest(void);

/**
 * @brief   Linear inter-/extrapolates a third point according to two given points
 *
 * @param   x1:               x-value of point 1
 * @param   y1:               y-value of point 1
 * @param   x2:               x-value of point 2
 * @param   y2:               y-value of point 2
 * @param   x_interpolate:    x value of interpolation point
 *
 * @return  interpolated value (float)
 */
extern float MATH_LinearInterpolation(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const float x_interpolate);

/**
 * @brief Swap bytes of uint16_t value
 * @param   val:    value to swap bytes: 0x1234 -> 0x3412
 */
extern uint16_t MATH_SwapBytesUint16_t(const uint16_t val);

/**
 * @brief   Swap bytes of uint32_t value
 * @param   val:    value to swap bytes: 0x12345678 -> 0x78563412
 */
extern uint32_t MATH_SwapBytesUint32_t(const uint32_t val);

/**
 * @brief   Swap bytes  of uint64_t value
 * @param   val:    value to swap bytes: 0x1122334455667788 -> 0x8877665544332211
 */
extern uint64_t MATH_SwapBytesUint64_t(const uint64_t val);

/**
 * @brief   Returns the minimum of the passed float values
 * @param[in] value1   value 1
 * @param[in] value2   value 2
 * @return  minimum value
 */
extern float MATH_MinimumOfTwoFloats(const float value1, const float value2);

/**
 * @brief   Returns the minimum of the passed uint8_t values
 * @param[in] value1   value 1
 * @param[in] value2   value 2
 * @return  minimum value
 */
extern uint8_t MATH_MinimumOfTwoUint8_t(const uint8_t value1, const uint8_t value2);

/**
 * @brief   Returns the minimum of the passed uint16_t values
 * @param[in] value1   value 1
 * @param[in] value2   value 2
 * @return  minimum value
 */
extern uint16_t MATH_MinimumOfTwoUint16_t(const uint16_t value1, const uint16_t value2);

/**
 * @brief   Returns the absolute value of passed int32_t value
 * @param[in] value   integer value
 * @return  absolute value or INT32_MAX if INT32_MIN is passed
 */
extern int32_t MATH_AbsInt32_t(const int32_t value);

/**
 * @brief   Returns the absolute value of passed int64_t value
 * @param[in] value   integer value
 * @return  absolute value or INT64_MAX if INT64_MIN is passed
 */
extern int64_t MATH_AbsInt64_t(const int64_t value);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__FOXMATH_H_ */
