/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    utils.c
 * @author  foxBMS Team
 * @date    2018-01-18 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  UTIL
 *
 * @brief   Util function implementations
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "utils.h"

#include "fassert.h"
#include "fstd_types.h"
#include "os.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define UTIL_PSEUDO_RANDOM_MULTIPLIER (0x015A4E35u)
#define UTIL_PSEUDO_RANDOM_LEFT_SHIFT (16u)
#define UTIL_PSEUDO_RANDOM_AND        (0x7FFFu)

/*========== Static Constant and Variable Definitions =======================*/
/** Stores the value for the next random number/seed between the calls */
static uint32_t util_nextRandomNumber = 0u;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void UTIL_ExtractCharactersFromString(
    uint64_t *pExtractedCharacters,
    const char *pkString,
    uint8_t stringLength,
    uint8_t startChar,
    uint8_t numberOfChars) {
    FAS_ASSERT(pExtractedCharacters != NULL_PTR);
    FAS_ASSERT(pkString != NULL_PTR);
    FAS_ASSERT(startChar < stringLength);
    FAS_ASSERT(numberOfChars <= stringLength);

    /* Calculate last character to be extracted */
    uint8_t lastChar = startChar + numberOfChars;
    if (lastChar > stringLength) {
        lastChar = stringLength;
    }

    /* Extract characters */
    for (uint8_t i = startChar; i < lastChar; i++) {
        *pExtractedCharacters = *pExtractedCharacters << UTIL_SHIFT_ONE_BYTE;
        *pExtractedCharacters += (uint8_t)pkString[i];
    }
}

extern void UTIL_SeedRandomNumber(uint32_t seed) {
    /* AXIVION Routine Generic-MissingParameterAssert: seed: parameter accepts whole range */
    util_nextRandomNumber = seed;
}

extern uint32_t UTIL_GetPseudoRandomNumber(void) {
    /* Use OS_GetTickCount() to get some randomness into the increment */
    uint32_t increment = OS_GetTickCount();
    /* Currently only fixed operations. Needs further improvement. */
    util_nextRandomNumber = (UTIL_PSEUDO_RANDOM_MULTIPLIER * util_nextRandomNumber) + increment;
    return ((uint32_t)(util_nextRandomNumber >> UTIL_PSEUDO_RANDOM_LEFT_SHIFT) & UTIL_PSEUDO_RANDOM_AND);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint32_t UTIL_GetSeed() {
    return util_nextRandomNumber;
}
#endif
