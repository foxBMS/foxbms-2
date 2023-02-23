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
 * @file    c-018.c
 * @author  foxBMS Team
 * @date    2021-06-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup GUIDELINES
 * @prefix  ABC
 *
 * @brief   Example code to show the application of the C coding guidelines
 * @details This code implements an example for C:018
 *
 */

/*========== Includes =======================================================*/
#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
typedef struct {
    uint32_t value;
} ABC_INIT_STRUCT_s;

typedef uint8_t ABC_SOME_FUNCTION_TYPE_f(void); /* typedef of a function type */

/*========== Static Constant and Variable Definitions =======================*/
static uint32_t *abc_pMyPointer         = NULL_PTR; /* static uint32_t pointer */
static ABC_INIT_STRUCT_s abc_initStruct = {0};      /* static init struct */
static uint32_t abc_myVariable          = 0;        /* local uint32_t variable */

/*========== Extern Constant and Variable Definitions =======================*/
ABC_INIT_STRUCT_s *abc_pInitStruct = NULL_PTR; /* local pointer to some init struct */
ABC_SOME_FUNCTION_TYPE_f *abc_fpMyFunction;    /* local function pointer using a typedef */

/*========== Static Function Prototypes =====================================*/
static void ABC_AssignSomeValue(void);

/*========== Static Function Implementations ================================*/
static void ABC_AssignSomeValue(void) {
    abc_pMyPointer = &abc_myVariable;
    abc_myVariable = *abc_pMyPointer;
    abc_myVariable = abc_initStruct.value;
    abc_myVariable = abc_pInitStruct->value;
}

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
