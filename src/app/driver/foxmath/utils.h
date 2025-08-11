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
 * @file    utils.h
 * @author  foxBMS Team
 * @date    2022-11-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  UTIL
 *
 * @brief   Utilities
 * @details Some utility macros and functions
 */

#ifndef FOXBMS__UTILS_H_
#define FOXBMS__UTILS_H_

/*========== Includes =======================================================*/

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** two byte bit mask */
#define UTIL_BIT_MASK_LSB (0xFFu)

/** shift zero bytes (0 positions) */
#define UTIL_SHIFT_ZERO_BYTES (0u)

/** shift one byte (8 positions) */
#define UTIL_SHIFT_ONE_BYTE (8u)

/** shift two bytes (16 positions) */
#define UTIL_SHIFT_TWO_BYTES (16u)

/** shift three bytes (24 positions) */
#define UTIL_SHIFT_THREE_BYTES (24u)

/** shift four bytes (32 positions) */
#define UTIL_SHIFT_FOUR_BYTES (32u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Extract characters from a string
 * @details - Extracts up to 8 characters of a String
 *          - Characters are extracted consecutively
 *          - If the end of the string is reached the function ends,
 *            this is even if the number of requested number of chars
 *            wasn't reached
 * @param pExtractedCharacters 64-bit variable where characters are written
 * @param pString String that chars will be extracted from
 * @param stringLength Length of the string
 * @param startChar First char that will be extracted
 * @param numberOfChars Number of Chars that will be extracted
 */
extern void UTIL_ExtractCharactersFromString(
    uint64_t *pExtractedCharacters,
    const char *pString,
    uint8_t stringLength,
    uint8_t startChar,
    uint8_t numberOfChars);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__UTILS_H_ */
