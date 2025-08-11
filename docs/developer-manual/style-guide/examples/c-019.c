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
 * @file    c-019.c
 * @author  foxBMS Team
 * @date    2021-06-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup GUIDELINES
 * @prefix  ABC
 *
 * @brief   Example code to show the application of the C coding guidelines
 * @details This code implements an example for C:019
 */

/*========== Includes =======================================================*/

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/* Initialization example for structs */
typedef struct {
    float x;
    float y;
    float z;
} ABC_POINT_s;

typedef struct {
    ABC_POINT_s point;
    uint32_t timestamp;
    uint32_t previousTimestamp;
    uint8_t name[16];
} ABC_PATH_s;

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/* Initialization for variables */
uint8_t abc_myVar0  = 10u;
uint16_t abc_myVar1 = 10u;
uint32_t abc_myVar2 = 10u;
uint64_t abc_myVar3 = 10uLL;
int8_t abc_myVar4   = -10;
int16_t abc_myVar5  = -10;
int32_t abc_myVar6  = -10;
int64_t abc_myVar7  = -10LL;

/* Initialization example for arrays */
uint8_t abc_myArrayA[]  = {1, 2, 3}; /* Array has type uint8_t[3] and holds 1,2,3 */
uint8_t abc_myArrayC[5] = {0};       /* Array has type uint8_t[5] and holds 0,0,0,0,0 */

uint8_t abc_myArrayD[4][3] = {
    /* array of 4 arrays of 3 uint8_t each (4x3 matrix) */
    {1},       /* row 0 initialized to {1, 0, 0} */
    {0, 1},    /* row 1 initialized to {0, 1, 0} */
    {[2] = 1}, /* row 2 initialized to {0, 0, 1} */
}; /* row 3 initialized to {0, 0, 0} */

ABC_POINT_s abc_myPoint = {1.2, 1.3}; /* p.x=1.2, p.y=1.3, p.z=0.0 */

ABC_PATH_s abc_myPath = {80.0f, 127.0f, -37.0f, 0, 1, "test"}; /* 80.0f initializes myPath.point.x  */
                                                               /* 127.0f initializes myPath.point.y */
                                                               /* -37.0f initializes myPath.point.z */
                                                               /* 0 initializes myPath.timestamp    */
                                                               /* 1 initializes ex.in_u.a8[3]       */
                                                               /* "test" initializes name[0-3]      */
                                                               /* name[4-15] are set to 0           */

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
