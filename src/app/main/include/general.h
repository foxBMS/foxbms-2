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
 * @updated 2021-03-24 (date of last update)
 * @ingroup GENERAL_CONF
 * @prefix  NONE
 *
 * @brief   TODO
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
 * @details Checked in the datasheet spnu563a: The largest pin number that is
 *          used is 31.
*/
#define LARGEST_PIN_NUMBER (31u)

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
static_assert(false == 0, "false seems to have been modified.");
static_assert(true != false, "true seems to have been modified.");
static_assert(true == 1, "true seems to have been modified.");

static_assert(STD_OK == 0, "STD_OK seems to have been modified.");
static_assert(STD_OK != STD_NOT_OK, "STD_OK or STD_NOT_OK seem to have been modified.");
static_assert(STD_NOT_OK == 1, "STD_NOT_OK seems to have been modified.");

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__GENERAL_H_ */
