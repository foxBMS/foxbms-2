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
 * @file    c-013.c
 * @author  foxBMS Team
 * @date    2021-06-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup GUIDELINES
 * @prefix  NONE
 *
 * @brief   Example code to show the application of the C coding guidelines
 * @details This code implements an example for C:013
 *
 */

/*========== Includes =======================================================*/
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
static uint8_t x1 = 0;
static uint8_t x2 = 0;
static uint8_t x3 = 0;
static uint8_t y1 = 0;
static uint8_t y2 = 0;
static uint8_t y3 = 0;
static uint8_t z1 = 0;
static uint8_t z2 = 0;
static uint8_t z3 = 0;

static uint8_t scores[5] = {0};
static uint8_t bases[5]  = {0};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/* clang-format off */
static void ABC_TransformMyWidget(
    uint8_t x1, uint8_t x2, uint8_t x3,
    uint8_t y1, uint8_t y2, uint8_t y3,
    uint8_t z1, uint8_t z2, uint8_t z3);
/* clang-format on */

static int16_t ABC_DoSomething(int16_t var, int16_t x, int16_t y, int16_t z);

/* clang-format off */
static void ABC_TransformMyWidget(
    uint8_t x1, uint8_t x2, uint8_t x3,
    uint8_t y1, uint8_t y2, uint8_t y3,
    uint8_t z1, uint8_t z2, uint8_t z3) {
}
/* clang-format on */

/*========== Static Function Implementations ================================*/
static int16_t ABC_DoSomething(int16_t var, int16_t x, int16_t y, int16_t z) {
    return 0;
}

int16_t ABC_SomeFunction(uint8_t x, uint8_t y, uint8_t z) {
    /* use variable to capture value, before passing to function */
    int16_t myHeuristic = scores[x] * y + bases[x];
    int16_t result      = ABC_DoSomething(myHeuristic, x, y, z);

    /* place confusing argument on own commented line and all other parameters
       also on an own line */
    result += ABC_DoSomething(
        scores[x] * y + bases[x], /* Score heuristic. */
        x,
        y,
        z);

    /* Its a matrix, and therefore it makes sense to format the arguments as a matrix */
    /* clang-format off */
    ABC_TransformMyWidget(x1, x2, x3,
                          y1, y2, y3,
                          z1, z2, z3);
    /* clang-format on */
    return result;
}

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
