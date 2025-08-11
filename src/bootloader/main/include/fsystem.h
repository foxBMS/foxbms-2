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
#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/* Macro of operating mode of processor */
/* https://developer.arm.com/documentation/ddi0460/d/Programmers-Model/Program-status-registers/The-M-bits?lang=en */
#define FSYS_M_BITS_USER_MODE   (0x10u)
#define FSYS_M_BITS_SYSTEM_MODE (0x1Fu)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Detect and return the current operation mode
 * @details The function is implemented in assembler code in operation_modes.
 */
extern uint32_t DetectOperationMode(void);

/**
 * @brief   Switch back to user mode
 * @details This macro is used after raising the privileges with
 *          #FSYS_RaisePrivilegeToSystemMode().
 *          Failure to call this macro may lead to unintended system behavior.
 */
#ifndef UNITY_UNIT_TEST
#pragma FUNC_ALWAYS_INLINE(FSYS_SwitchToUserMode)
static inline void FSYS_SwitchToUserMode(void) {
    /* Check the user mode before changing to user mode */
    uint32_t userMode = DetectOperationMode();
    if (userMode != FSYS_M_BITS_USER_MODE) {
        /* Change the user mode to user mode */
        __asm(" CPS #0x10");
    }
}
#else
#define FSYS_SwitchToUserMode()
#endif

/* AXIVION Disable Style Generic-DoxygenCommentInHeader: Function has a doxygen
 * comment, but since the '#pragma' AXIVION does not detect it, we annotate it
 */
/* AXIVION Disable Style MisraC2012Directive-1.1: 'pragma' required to tell the
 * TI ARM CGT compiler, that this is an interrupt function
 * (see SPNU151V-January1998-RevisedFebruary2020: 5.11.29 The SWI_ALIAS Pragma)
 */
/* AXIVION Disable Style MisraC2012-1.2: Function is implemented in assembler
 * and this is the way to tell it the TI compiler (see src\bootloader\main\
 * operation_modes.asm::switch_operation_mode)
 */
/* AXIVION Disable Style MisraC2012-8.6: Function definition is in assembler
 * (see src\bootloader\main\operation_modes.asm::switch_operation_mode)
 */
/**
 * @brief   Raise privilege
 * @details This alias is mapped to an ASM function and raises to a privileged
 *          processor state if the system is currently in user mode.
 */
#pragma SWI_ALIAS(FSYS_RaisePrivilegeToSystemModeSWI, 1);
extern void FSYS_RaisePrivilegeToSystemModeSWI(void);

#pragma FUNC_ALWAYS_INLINE(FSYS_RaisePrivilegeToSystemMode)
static inline STD_RETURN_TYPE_e FSYS_RaisePrivilegeToSystemMode(void) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    bool gotoNext            = true;

    /* Check the user mode before changing to system mode */
    uint32_t userMode = DetectOperationMode();
    if (userMode == FSYS_M_BITS_SYSTEM_MODE) {
        retVal   = STD_OK;
        gotoNext = false;
    }
    if (gotoNext) {
        /* Change the user mode to system mode */
        FSYS_RaisePrivilegeToSystemModeSWI();
        /* Check the user mode after changing to system mode */
        userMode = DetectOperationMode();
        if (userMode == FSYS_M_BITS_SYSTEM_MODE) {
            retVal = STD_OK;
        }
    }

    return retVal;
}
/* AXIVION Enable Style MisraC2012-8.6: */
/* AXIVION Enable Style MisraC2012-1.2: */
/* AXIVION Enable Style MisraC2012Directive-1.1: */
/* AXIVION Enable Style Generic-DoxygenCommentInHeader: */

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif
#endif /* FOXBMS__FSYSTEM_H_ */
