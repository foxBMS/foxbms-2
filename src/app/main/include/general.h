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
 * @file    general.h
 * @author  foxBMS Team
 * @date    2019-09-24 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup GENERAL_CONF
 * @prefix  NONE
 *
 * @brief   General macros and definitions for the whole platform.
 *
 */

#ifndef FOXBMS__GENERAL_H_
#define FOXBMS__GENERAL_H_

/*========== Includes =======================================================*/
#include "HL_sys_common.h"

#include "fassert.h"
#include "fstd_types.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * @brief   largest pin number that exists in TMS570LC4357
 * @details Checked in the data sheet spnu563a: The largest pin number that is
 *          used is 31.
*/
#define LARGEST_PIN_NUMBER (31u)

/**
 * @brief   maximum number of channels measured by the ADC1
 * @details Checked in the data sheet spnu563a: ADC1 supports 32 channels
 */
#define ADC_ADC1_MAX_NR_CHANNELS (32U)

/**
 * @brief   sets a bit to 1u
 * @param[in,out]   register    register on which to set the bit
 * @param[in]       bit         number of the bit that should be set to 1u
 */
#define SETBIT(register, bit) ((register) |= (uint32)((uint32)1U << (bit)))
/**
 * @brief   clears a bit to 0u
 * @param[in,out]   register    register on which to clear the bit
 * @param[in]       bit         number of the bit that should be cleared to 0u
 */
#define CLEARBIT(register, bit) ((register) &= ~(uint32)((uint32)1U << (bit)))

/**
 * @brief Allows functions to generate warnings in GCC for unused returns.
 *
 * This attribute allows to mark that a function return value must be used.
 * The compiler will generate a warning when the return value of a function
 * with this marker is not used in subsequent code.
 */
#define must_check_return __attribute__((warn_unused_result))

/** This attribute tells the compiler that the function should always be inlined */
#define always_inline __attribute__((always_inline))

/** allow implementations to be weak for unit tests */
#ifdef UNITY_UNIT_TEST
#define UNIT_TEST_WEAK_IMPL __attribute__((weak))
#else
#define UNIT_TEST_WEAK_IMPL
#endif

/* assert that the basic datatypes in fstd_types.h are intact */
/* AXIVION Disable Style MisraC2012-10.4: These assertions have to check the actual values of the enums and defines. */
static_assert(false == 0, "false seems to have been modified.");
static_assert(true != false, "true seems to have been modified.");
static_assert(true == 1, "true seems to have been modified.");

static_assert(STD_OK == 0, "STD_OK seems to have been modified.");
static_assert(STD_OK != STD_NOT_OK, "STD_OK or STD_NOT_OK seem to have been modified.");
static_assert(STD_NOT_OK == 1, "STD_NOT_OK seems to have been modified.");
/* AXIVION Enable Style MisraC2012-10.4: */

/**
 * Internal macros for the implementation of #REPEAT_U(). Do not use outside.
 * @{
 */
/* AXIVION Disable Style Generic-NoUnsafeMacro MisraC2012Directive-4.9: Due to the nature of these macros
   it is impossible to wrap the REPEAT_Uxu(x) token in parentheses. With these
   function-like Macros repeating a token is implemented. */
#define REPEAT_U1u(x)  (x)
#define REPEAT_U2u(x)  REPEAT_U1u(x), (x)
#define REPEAT_U3u(x)  REPEAT_U2u(x), (x)
#define REPEAT_U4u(x)  REPEAT_U3u(x), (x)
#define REPEAT_U5u(x)  REPEAT_U4u(x), (x)
#define REPEAT_U6u(x)  REPEAT_U5u(x), (x)
#define REPEAT_U7u(x)  REPEAT_U6u(x), (x)
#define REPEAT_U8u(x)  REPEAT_U7u(x), (x)
#define REPEAT_U9u(x)  REPEAT_U8u(x), (x)
#define REPEAT_U10u(x) REPEAT_U9u(x), (x)
#define REPEAT_U11u(x) REPEAT_U10u(x), (x)
#define REPEAT_U12u(x) REPEAT_U11u(x), (x)
#define REPEAT_U13u(x) REPEAT_U12u(x), (x)
#define REPEAT_U14u(x) REPEAT_U13u(x), (x)
#define REPEAT_U15u(x) REPEAT_U14u(x), (x)
#define REPEAT_U16u(x) REPEAT_U15u(x), (x)
#define REPEAT_U17u(x) REPEAT_U16u(x), (x)
#define REPEAT_U18u(x) REPEAT_U17u(x), (x)
#define REPEAT_U19u(x) REPEAT_U18u(x), (x)
#define REPEAT_U20u(x) REPEAT_U19u(x), (x)
#define REPEAT_U21u(x) REPEAT_U20u(x), (x)
#define REPEAT_U22u(x) REPEAT_U21u(x), (x)
#define REPEAT_U23u(x) REPEAT_U22u(x), (x)
#define REPEAT_U24u(x) REPEAT_U23u(x), (x)
#define REPEAT_U25u(x) REPEAT_U24u(x), (x)
#define REPEAT_U26u(x) REPEAT_U25u(x), (x)
#define REPEAT_U27u(x) REPEAT_U26u(x), (x)
#define REPEAT_U28u(x) REPEAT_U27u(x), (x)
#define REPEAT_U29u(x) REPEAT_U28u(x), (x)
#define REPEAT_U30u(x) REPEAT_U29u(x), (x)
#define REPEAT_U31u(x) REPEAT_U30u(x), (x)
#define REPEAT_U32u(x) REPEAT_U31u(x), (x)
#define REPEAT_U33u(x) REPEAT_U32u(x), (x)
#define REPEAT_U34u(x) REPEAT_U33u(x), (x)
#define REPEAT_U35u(x) REPEAT_U34u(x), (x)
#define REPEAT_U36u(x) REPEAT_U35u(x), (x)
#define REPEAT_U37u(x) REPEAT_U36u(x), (x)
#define REPEAT_U38u(x) REPEAT_U37u(x), (x)
#define REPEAT_U39u(x) REPEAT_U38u(x), (x)
#define REPEAT_U40u(x) REPEAT_U39u(x), (x)
#define REPEAT_U41u(x) REPEAT_U40u(x), (x)
#define REPEAT_U42u(x) REPEAT_U41u(x), (x)
#define REPEAT_U43u(x) REPEAT_U42u(x), (x)
#define REPEAT_U44u(x) REPEAT_U43u(x), (x)
#define REPEAT_U45u(x) REPEAT_U44u(x), (x)
#define REPEAT_U46u(x) REPEAT_U45u(x), (x)
#define REPEAT_U47u(x) REPEAT_U46u(x), (x)
#define REPEAT_U48u(x) REPEAT_U47u(x), (x)
#define REPEAT_U49u(x) REPEAT_U48u(x), (x)
#define REPEAT_U50u(x) REPEAT_U49u(x), (x)
#define REPEAT_U51u(x) REPEAT_U50u(x), (x)
#define REPEAT_U52u(x) REPEAT_U51u(x), (x)
#define REPEAT_U53u(x) REPEAT_U52u(x), (x)
#define REPEAT_U54u(x) REPEAT_U53u(x), (x)
#define REPEAT_U55u(x) REPEAT_U54u(x), (x)
#define REPEAT_U56u(x) REPEAT_U55u(x), (x)
#define REPEAT_U57u(x) REPEAT_U56u(x), (x)
#define REPEAT_U58u(x) REPEAT_U57u(x), (x)
#define REPEAT_U59u(x) REPEAT_U58u(x), (x)
#define REPEAT_U60u(x) REPEAT_U59u(x), (x)
#define REPEAT_U61u(x) REPEAT_U60u(x), (x)
#define REPEAT_U62u(x) REPEAT_U61u(x), (x)
#define REPEAT_U63u(x) REPEAT_U62u(x), (x)
#define REPEAT_U64u(x) REPEAT_U63u(x), (x)
#define REPEAT_U65u(x) REPEAT_U64u(x), (x)
#define REPEAT_U66u(x) REPEAT_U65u(x), (x)
#define REPEAT_U67u(x) REPEAT_U66u(x), (x)
#define REPEAT_U68u(x) REPEAT_U67u(x), (x)
#define REPEAT_U69u(x) REPEAT_U68u(x), (x)
#define REPEAT_U70u(x) REPEAT_U69u(x), (x)
#define REPEAT_U71u(x) REPEAT_U70u(x), (x)
#define REPEAT_U72u(x) REPEAT_U71u(x), (x)
#define REPEAT_U73u(x) REPEAT_U72u(x), (x)
#define REPEAT_U74u(x) REPEAT_U73u(x), (x)
#define REPEAT_U75u(x) REPEAT_U74u(x), (x)
#define REPEAT_U76u(x) REPEAT_U75u(x), (x)
#define REPEAT_U77u(x) REPEAT_U76u(x), (x)
#define REPEAT_U78u(x) REPEAT_U77u(x), (x)
#define REPEAT_U79u(x) REPEAT_U78u(x), (x)
#define REPEAT_U80u(x) REPEAT_U79u(x), (x)
#define REPEAT_U81u(x) REPEAT_U80u(x), (x)
#define REPEAT_U82u(x) REPEAT_U81u(x), (x)
#define REPEAT_U83u(x) REPEAT_U82u(x), (x)
#define REPEAT_U84u(x) REPEAT_U83u(x), (x)
#define REPEAT_U85u(x) REPEAT_U84u(x), (x)
#define REPEAT_U86u(x) REPEAT_U85u(x), (x)
#define REPEAT_U87u(x) REPEAT_U86u(x), (x)
#define REPEAT_U88u(x) REPEAT_U87u(x), (x)
#define REPEAT_U89u(x) REPEAT_U88u(x), (x)
#define REPEAT_U90u(x) REPEAT_U89u(x), (x)
#define REPEAT_U91u(x) REPEAT_U90u(x), (x)
#define REPEAT_U92u(x) REPEAT_U91u(x), (x)
#define REPEAT_U93u(x) REPEAT_U92u(x), (x)
#define REPEAT_U94u(x) REPEAT_U93u(x), (x)
#define REPEAT_U95u(x) REPEAT_U94u(x), (x)
#define REPEAT_U96u(x) REPEAT_U95u(x), (x)
#define REPEAT_U97u(x) REPEAT_U96u(x), (x)
#define REPEAT_U98u(x) REPEAT_U97u(x), (x)
#define REPEAT_U99u(x) REPEAT_U98u(x), (x)

/* AXIVION Disable Style MisraC2012-20.10: Usage allowed as long as remarks in documentation are honored. */
#define REPEAT_Ux(x, n) REPEAT_U##n(x)
/* AXIVION Enable Style Generic-NoUnsafeMacro MisraC2012Directive-4.9 MisraC2012-20.10: */
/**@}*/

/** Maximum number of supported repetitions in #REPEAT_U(). Adapt if you change implementation.*/
#define REPEAT_MAXIMUM_REPETITIONS (99u)

/**
 * @brief   Macro that helps to generate a series of literals (for array initializers).
 * @details This macro generates a series of literals for array initializers.
 *          This can be used for initializing arrays to arbitrary non-null values
 *          when their size is defined with a macro. If the array shall be initialized
 *          to null the standard {0} should be used.
 *
 * @param   x   token that should be repeated, e.g. true
 * @param   n   Times that it should be repeated (stripped of parenthesis with
 *              #STRIP() and described as unsigned integer literal) (maximum 16,
 *              #REPEAT_MAXIMUM_REPETITIONS, repetitions)
 *
 *          Example usage:
  \verbatim
  #define ARRAY_SIZE (4u)
  bool variable[ARRAY_SIZE] = {REPEAT_U(false, STRIP(ARRAY_SIZE))};
  \endverbatim
 *          This will expand to:
  \verbatim
  bool variable[ARRAY_SIZE] = {false, false, false, false};
  \endverbatim
 */
/* AXIVION Disable Style MisraC2012Directive-4.9: Function-like macro needed for this feature. */
#define REPEAT_U(x, n) REPEAT_Ux(x, n)
/* AXIVION Enable Style MisraC2012Directive-4.9: */

/** Internal helper macros for #STRIP(). Do not use outside.
 * @{
 */
/* AXIVION Disable Style MisraC2012Directive-4.9 MisraC2012-20.7 Generic-NoUnsafeMacro: Function-like macro needed for
   this feature. Parenthesis stripping is intended here. */
#define GET_ARGS(...)   __VA_ARGS__
#define STRIP_PARENS(x) x
/**@}*/
/** Strips a token of its surrounding parenthesis. */
#define STRIP(x) STRIP_PARENS(GET_ARGS x)
/* AXIVION Enable Style MisraC2012Directive-4.9 MisraC2012-20.7 Generic-NoUnsafeMacro: */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__GENERAL_H_ */
