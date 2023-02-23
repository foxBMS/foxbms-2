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
 * @file    general.h
 * @author  foxBMS Team
 * @date    2019-09-24 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup GENERAL_CONF
 * @prefix  GEN
 *
 * @brief   General macros and definitions for the whole platform.
 *
 */

#ifndef FOXBMS__GENERAL_H_
#define FOXBMS__GENERAL_H_

/*========== Includes =======================================================*/
#include "fassert.h"
#include "fstd_types.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * @brief   sets a bit to 1u
 * @param[in,out]   register    register on which to set the bit
 * @param[in]       bit         number of the bit that should be set to 1u
 */
#define GEN_SET_BIT(register, bit) ((register) |= (uint32)((uint32)1U << (bit)))
/**
 * @brief   clears a bit to 0u
 * @param[in,out]   register    register on which to clear the bit
 * @param[in]       bit         number of the bit that should be cleared to 0u
 */
#define GEN_CLEAR_BIT(register, bit) ((register) &= ~(uint32)((uint32)1U << (bit)))

/**
 * @brief Allows functions to generate warnings in GCC for unused returns.
 *
 * This attribute allows to mark that a function return value must be used.
 * The compiler will generate a warning when the return value of a function
 * with this marker is not used in subsequent code.
 */
#define GEN_MUST_CHECK_RETURN __attribute__((warn_unused_result))

/* AXIVION Next Codeline Style MisraC2012-1.2: function-inlining is sometimes necessary for performance reasons */
/** This attribute tells the compiler that the function should always be inlined */
#define GEN_ALWAYS_INLINE __attribute__((always_inline))

/** allow implementations to be weak for unit tests */
#ifdef UNITY_UNIT_TEST
#define UNIT_TEST_WEAK_IMPL __attribute__((weak))
#else
#define UNIT_TEST_WEAK_IMPL
#endif

/* assert that the basic datatypes in fstd_types.h are intact */
/* AXIVION Disable Style MisraC2012-10.4: These assertions have to check the actual values of the enums and defines. */
FAS_STATIC_ASSERT(false == 0, "false seems to have been modified.");
FAS_STATIC_ASSERT(true != false, "true seems to have been modified.");
FAS_STATIC_ASSERT(true == 1, "true seems to have been modified.");

FAS_STATIC_ASSERT(STD_OK == 0, "STD_OK seems to have been modified.");
FAS_STATIC_ASSERT(STD_OK != STD_NOT_OK, "STD_OK or STD_NOT_OK seem to have been modified.");
FAS_STATIC_ASSERT(STD_NOT_OK == 1, "STD_NOT_OK seems to have been modified.");
/* AXIVION Enable Style MisraC2012-10.4: */

/**
 * Internal macros for the implementation of #GEN_REPEAT_U(). Do not use outside.
 * @{
 */
/* AXIVION Disable Style Generic-NoUnsafeMacro MisraC2012Directive-4.9: Due to the nature of these macros
   it is impossible to wrap the REPEAT_Uxu(x) token in parentheses. With these
   function-like Macros repeating a token is implemented. */
/* us lowercase 'u' suffix to emphasize the unsignedness of the 'GEN_REPEAT_U*u' macros. */
#define GEN_REPEAT_U1u(x)  (x)
#define GEN_REPEAT_U2u(x)  GEN_REPEAT_U1u(x), (x)
#define GEN_REPEAT_U3u(x)  GEN_REPEAT_U2u(x), (x)
#define GEN_REPEAT_U4u(x)  GEN_REPEAT_U3u(x), (x)
#define GEN_REPEAT_U5u(x)  GEN_REPEAT_U4u(x), (x)
#define GEN_REPEAT_U6u(x)  GEN_REPEAT_U5u(x), (x)
#define GEN_REPEAT_U7u(x)  GEN_REPEAT_U6u(x), (x)
#define GEN_REPEAT_U8u(x)  GEN_REPEAT_U7u(x), (x)
#define GEN_REPEAT_U9u(x)  GEN_REPEAT_U8u(x), (x)
#define GEN_REPEAT_U10u(x) GEN_REPEAT_U9u(x), (x)
#define GEN_REPEAT_U11u(x) GEN_REPEAT_U10u(x), (x)
#define GEN_REPEAT_U12u(x) GEN_REPEAT_U11u(x), (x)
#define GEN_REPEAT_U13u(x) GEN_REPEAT_U12u(x), (x)
#define GEN_REPEAT_U14u(x) GEN_REPEAT_U13u(x), (x)
#define GEN_REPEAT_U15u(x) GEN_REPEAT_U14u(x), (x)
#define GEN_REPEAT_U16u(x) GEN_REPEAT_U15u(x), (x)
#define GEN_REPEAT_U17u(x) GEN_REPEAT_U16u(x), (x)
#define GEN_REPEAT_U18u(x) GEN_REPEAT_U17u(x), (x)
#define GEN_REPEAT_U19u(x) GEN_REPEAT_U18u(x), (x)
#define GEN_REPEAT_U20u(x) GEN_REPEAT_U19u(x), (x)
#define GEN_REPEAT_U21u(x) GEN_REPEAT_U20u(x), (x)
#define GEN_REPEAT_U22u(x) GEN_REPEAT_U21u(x), (x)
#define GEN_REPEAT_U23u(x) GEN_REPEAT_U22u(x), (x)
#define GEN_REPEAT_U24u(x) GEN_REPEAT_U23u(x), (x)
#define GEN_REPEAT_U25u(x) GEN_REPEAT_U24u(x), (x)
#define GEN_REPEAT_U26u(x) GEN_REPEAT_U25u(x), (x)
#define GEN_REPEAT_U27u(x) GEN_REPEAT_U26u(x), (x)
#define GEN_REPEAT_U28u(x) GEN_REPEAT_U27u(x), (x)
#define GEN_REPEAT_U29u(x) GEN_REPEAT_U28u(x), (x)
#define GEN_REPEAT_U30u(x) GEN_REPEAT_U29u(x), (x)
#define GEN_REPEAT_U31u(x) GEN_REPEAT_U30u(x), (x)
#define GEN_REPEAT_U32u(x) GEN_REPEAT_U31u(x), (x)
#define GEN_REPEAT_U33u(x) GEN_REPEAT_U32u(x), (x)
#define GEN_REPEAT_U34u(x) GEN_REPEAT_U33u(x), (x)
#define GEN_REPEAT_U35u(x) GEN_REPEAT_U34u(x), (x)
#define GEN_REPEAT_U36u(x) GEN_REPEAT_U35u(x), (x)
#define GEN_REPEAT_U37u(x) GEN_REPEAT_U36u(x), (x)
#define GEN_REPEAT_U38u(x) GEN_REPEAT_U37u(x), (x)
#define GEN_REPEAT_U39u(x) GEN_REPEAT_U38u(x), (x)
#define GEN_REPEAT_U40u(x) GEN_REPEAT_U39u(x), (x)
#define GEN_REPEAT_U41u(x) GEN_REPEAT_U40u(x), (x)
#define GEN_REPEAT_U42u(x) GEN_REPEAT_U41u(x), (x)
#define GEN_REPEAT_U43u(x) GEN_REPEAT_U42u(x), (x)
#define GEN_REPEAT_U44u(x) GEN_REPEAT_U43u(x), (x)
#define GEN_REPEAT_U45u(x) GEN_REPEAT_U44u(x), (x)
#define GEN_REPEAT_U46u(x) GEN_REPEAT_U45u(x), (x)
#define GEN_REPEAT_U47u(x) GEN_REPEAT_U46u(x), (x)
#define GEN_REPEAT_U48u(x) GEN_REPEAT_U47u(x), (x)
#define GEN_REPEAT_U49u(x) GEN_REPEAT_U48u(x), (x)
#define GEN_REPEAT_U50u(x) GEN_REPEAT_U49u(x), (x)
#define GEN_REPEAT_U51u(x) GEN_REPEAT_U50u(x), (x)
#define GEN_REPEAT_U52u(x) GEN_REPEAT_U51u(x), (x)
#define GEN_REPEAT_U53u(x) GEN_REPEAT_U52u(x), (x)
#define GEN_REPEAT_U54u(x) GEN_REPEAT_U53u(x), (x)
#define GEN_REPEAT_U55u(x) GEN_REPEAT_U54u(x), (x)
#define GEN_REPEAT_U56u(x) GEN_REPEAT_U55u(x), (x)
#define GEN_REPEAT_U57u(x) GEN_REPEAT_U56u(x), (x)
#define GEN_REPEAT_U58u(x) GEN_REPEAT_U57u(x), (x)
#define GEN_REPEAT_U59u(x) GEN_REPEAT_U58u(x), (x)
#define GEN_REPEAT_U60u(x) GEN_REPEAT_U59u(x), (x)
#define GEN_REPEAT_U61u(x) GEN_REPEAT_U60u(x), (x)
#define GEN_REPEAT_U62u(x) GEN_REPEAT_U61u(x), (x)
#define GEN_REPEAT_U63u(x) GEN_REPEAT_U62u(x), (x)
#define GEN_REPEAT_U64u(x) GEN_REPEAT_U63u(x), (x)
#define GEN_REPEAT_U65u(x) GEN_REPEAT_U64u(x), (x)
#define GEN_REPEAT_U66u(x) GEN_REPEAT_U65u(x), (x)
#define GEN_REPEAT_U67u(x) GEN_REPEAT_U66u(x), (x)
#define GEN_REPEAT_U68u(x) GEN_REPEAT_U67u(x), (x)
#define GEN_REPEAT_U69u(x) GEN_REPEAT_U68u(x), (x)
#define GEN_REPEAT_U70u(x) GEN_REPEAT_U69u(x), (x)
#define GEN_REPEAT_U71u(x) GEN_REPEAT_U70u(x), (x)
#define GEN_REPEAT_U72u(x) GEN_REPEAT_U71u(x), (x)
#define GEN_REPEAT_U73u(x) GEN_REPEAT_U72u(x), (x)
#define GEN_REPEAT_U74u(x) GEN_REPEAT_U73u(x), (x)
#define GEN_REPEAT_U75u(x) GEN_REPEAT_U74u(x), (x)
#define GEN_REPEAT_U76u(x) GEN_REPEAT_U75u(x), (x)
#define GEN_REPEAT_U77u(x) GEN_REPEAT_U76u(x), (x)
#define GEN_REPEAT_U78u(x) GEN_REPEAT_U77u(x), (x)
#define GEN_REPEAT_U79u(x) GEN_REPEAT_U78u(x), (x)
#define GEN_REPEAT_U80u(x) GEN_REPEAT_U79u(x), (x)
#define GEN_REPEAT_U81u(x) GEN_REPEAT_U80u(x), (x)
#define GEN_REPEAT_U82u(x) GEN_REPEAT_U81u(x), (x)
#define GEN_REPEAT_U83u(x) GEN_REPEAT_U82u(x), (x)
#define GEN_REPEAT_U84u(x) GEN_REPEAT_U83u(x), (x)
#define GEN_REPEAT_U85u(x) GEN_REPEAT_U84u(x), (x)
#define GEN_REPEAT_U86u(x) GEN_REPEAT_U85u(x), (x)
#define GEN_REPEAT_U87u(x) GEN_REPEAT_U86u(x), (x)
#define GEN_REPEAT_U88u(x) GEN_REPEAT_U87u(x), (x)
#define GEN_REPEAT_U89u(x) GEN_REPEAT_U88u(x), (x)
#define GEN_REPEAT_U90u(x) GEN_REPEAT_U89u(x), (x)
#define GEN_REPEAT_U91u(x) GEN_REPEAT_U90u(x), (x)
#define GEN_REPEAT_U92u(x) GEN_REPEAT_U91u(x), (x)
#define GEN_REPEAT_U93u(x) GEN_REPEAT_U92u(x), (x)
#define GEN_REPEAT_U94u(x) GEN_REPEAT_U93u(x), (x)
#define GEN_REPEAT_U95u(x) GEN_REPEAT_U94u(x), (x)
#define GEN_REPEAT_U96u(x) GEN_REPEAT_U95u(x), (x)
#define GEN_REPEAT_U97u(x) GEN_REPEAT_U96u(x), (x)
#define GEN_REPEAT_U98u(x) GEN_REPEAT_U97u(x), (x)
#define GEN_REPEAT_U99u(x) GEN_REPEAT_U98u(x), (x)

/* AXIVION Disable Style MisraC2012-20.10: Usage allowed as long as remarks in documentation are honored. */
#define GEN_REPEAT_Ux(x, n) GEN_REPEAT_U##n(x)
/* AXIVION Enable Style Generic-NoUnsafeMacro MisraC2012Directive-4.9 MisraC2012-20.10: */
/**@}*/

/** Maximum number of supported repetitions in #GEN_REPEAT_U(). Adapt if you change implementation.*/
#define GEN_REPEAT_MAXIMUM_REPETITIONS (99u)

/**
 * @brief   Macro that helps to generate a series of literals (for array initializers).
 * @details This macro generates a series of literals for array initializers.
 *          This can be used for initializing arrays to arbitrary non-null values
 *          when their size is defined with a macro. If the array shall be initialized
 *          to null the standard {0} should be used.
 *
 * @param   x   token that should be repeated, e.g. true
 * @param   n   Times that it should be repeated (stripped of parenthesis with
 *              #GEN_STRIP() and described as unsigned integer literal) (maximum 16,
 *              #GEN_REPEAT_MAXIMUM_REPETITIONS, repetitions)
 *
 *          Example usage:
  \verbatim
  #define ARRAY_SIZE (4u)
  bool variable[ARRAY_SIZE] = {GEN_REPEAT_U(false, GEN_STRIP(ARRAY_SIZE))};
  \endverbatim
 *          This will expand to:
  \verbatim
  bool variable[ARRAY_SIZE] = {false, false, false, false};
  \endverbatim
 */
/* AXIVION Next Codeline Style MisraC2012Directive-4.9: Function-like macro needed for this feature. */
#define GEN_REPEAT_U(x, n) GEN_REPEAT_Ux(x, n)

/** Internal helper macros for #GEN_STRIP(). Do not use outside.
 * @{
 */
/* AXIVION Disable Style MisraC2012Directive-4.9 MisraC2012-20.7 Generic-NoUnsafeMacro CertC-PRE01: Function-like macro
   needed for this feature. Parenthesis stripping is intended here. */
#define GEN_GET_ARGS(...)   __VA_ARGS__
#define GEN_STRIP_PARENS(x) x
/**@}*/
/** Strips a token of its surrounding parenthesis. */
#define GEN_STRIP(x) GEN_STRIP_PARENS(GEN_GET_ARGS x)
/* AXIVION Enable Style MisraC2012Directive-4.9 MisraC2012-20.7 Generic-NoUnsafeMacro: */

/** Defines the word size in bytes of the platform */
#if defined(__TI_COMPILER_VERSION__) && defined(__ARM_32BIT_STATE) && defined(__TMS470__)
#define GEN_BYTES_PER_WORD (4u)
#elif defined(UNITY_UNIT_TEST)
/* since this define only affects the task size, it can be safely set in unit
   tests to the value that is used in the embedded platform */
#define GEN_BYTES_PER_WORD (4u)
#else
#warning "Unspecified platform default to 4 bytes per word."
#define GEN_BYTES_PER_WORD (4u)
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__GENERAL_H_ */
