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
 * @file    fsystem.h
 * @author  foxBMS Team
 * @date    2020-07-21 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup MAIN
 * @prefix  FSYS
 *
 * @brief   Function to switch between user mode and privilege mode
 * @details TODO
 */

#ifndef FOXBMS__FSYSTEM_H_
#define FOXBMS__FSYSTEM_H_

/*========== Includes =======================================================*/
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* AXIVION Disable Style Generic-DoxygenCommentInHeader: Function has a doxygen
 * comment, but since the '#pragma' AXIVION does not detect it, we annotate it
 */
/* AXIVION Disable Style MisraC2012Directive-1.1: 'pragma' required to tell the
 * TI ARM CGT compiler, that this is an interrupt function
 * (see SPNU151V-January1998-RevisedFebruary2020: 5.11.29 The SWI_ALIAS Pragma)
 */
/* AXIVION Disable Style MisraC2012-1.2: Function is implemented in assembler
 * and this is the way to tell it the TI compiler (see
 * src\os\freertos\portable\ccs\arm_cortex-r5\portasm.asm::swiRaisePrivilege)
 */
/* AXIVION Disable Style MisraC2012-8.6: Function definition is in assembler
 * (see
 * src\os\freertos\portable\ccs\arm_cortex-r5\portasm.asm::swiRaisePrivilege)
 */
/**
 * @brief   Raise privilege
 * @details This alias is mapped to an ASM function and raises to a privileged
 *          processor state if the system is currently in user mode.
 *          This is done by the following ASM code:
 *
 *          * Copy the contents of a CP14 or CP15 coprocessor register (in this
 *          case SPSR -- Saved Program Status Register) into local register r12:
 *          \verbatim mrs     r12, spsr \endverbatim
 *
 *          * Perform bitwise AND on the contents of r12 and 0x0F
 *          * write to r0
 *          * update condition code flag on result of operation
 *          \verbatim ands    r0, r12, #0x0F      ; return value \endverbatim
 *
 *          * Perform a logical OR on the contents of r12 and 0x1F, if equal to
 *          condition code flag from previous instruction
 *          * write to r12
 *          \verbatim orreq   r12, r12, #0x1F \endverbatim
 *
 *          * Conditionally (equal on previous instruction condition code flag)
 *          load immediate result from r12 into control field mask byte,
 *          PSR[7:0] (privileged software execution) from SPSR
 *          \verbatim msreq   spsr_c, r12 \endverbatim
 *
 *          * Branch and exchange instruction set
 *          * Target address is contained in r14
 *          \verbatim bx      r14 \endverbatim
 *
 *          It is important to issue #FSYS_SwitchToUserMode() after the
 *          privileged mode is no longer needed. Otherwise the system would
 *          stay in privileged mode.
 *
 *          The datatype of the return value has to be reevaluated when changing
 *          platform and compiler.
 * @return  return value is zero if caller was in user mode (type: long)
 */
#pragma SWI_ALIAS(FSYS_RaisePrivilege, 1);
extern long FSYS_RaisePrivilege(void);
/* AXIVION Enable Style MisraC2012-8.6: */
/* AXIVION Enable Style MisraC2012-1.2: */
/* AXIVION Enable Style MisraC2012Directive-1.1: */
/* AXIVION Enable Style Generic-DoxygenCommentInHeader: */

/**
 * @brief   Switch back to user mode
 * @details This macro is used after raising the privileges with
 *          #FSYS_RaisePrivilege(). Failure to call this macro may lead to
 *          unintended system behavior.
 */
#ifndef UNITY_UNIT_TEST
#pragma FUNC_ALWAYS_INLINE(FSYS_SwitchToUserMode)
static inline void FSYS_SwitchToUserMode(void) {
    __asm(" CPS #0x10");
}

#else
#define FSYS_SwitchToUserMode()
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__FSYSTEM_H_ */
