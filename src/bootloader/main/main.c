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
 * @file    main.c
 * @author  foxBMS Team
 * @date    2024-08-27 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup MAIN
 * @prefix  MAIN
 *
 * @brief   Main function
 * @details The main function implements a state machine to realize the
 *          workflow of the bootloader.
 */

/*========== Includes =======================================================*/
#include "main.h"

#include "boot_cfg.h"

#include "HL_gio.h"
#include "HL_pinmux.h"
#include "HL_rti.h"
#include "HL_system.h"

#include "boot.h"
#include "can.h"
#include "fstd_types.h"
#include "fstring.h"
#include "infinite-loop-helper.h"
#include "rti.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
#ifndef UNITY_UNIT_TEST
int main(void)
#else
int unit_test_main(void)
#endif
{
    /* Enable IRQ and FIQ Interrupt mode in CPSR register */
    _enable_interrupt_();

    /* Init some modules */
    muxInit();
    gioInit();
    CAN_Initialize();
    rtiInit();

    /* Copy the flash relevant .text and .const to RAM */
    memcpy(&main_textRunStartFlashC, &main_textLoadStartFlashC, (uint32_t)&main_textSizeFlashC);
    memcpy(&main_constRunStartFlashCfgC, &main_constLoadStartFlashCfgC, (uint32_t)&main_constSizeFlashCfgC);

    CAN_SendBootMessage();

    /* Start rti timer */
    RTI_ResetFreeRunningCount();
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);
    uint32_t startCounter   = RTI_GetFreeRunningCount();
    bool finishedOneAutoRun = false;

    while (FOREVER()) {
        /* Enable IRQ interrupt to ensure the incoming CAN message can be processed,
        because the IRQ could be potentially disabled by any sub function that is
        called in the following context. */
        _enable_IRQ_interrupt_();

        /* Check if timeout happens */
        if ((finishedOneAutoRun == false) && (RTI_IsTimeElapsed(startCounter, MAIN_TIME_OUT_us) == true)) {
            if (boot_state == BOOT_FSM_STATE_WAIT) {
                boot_state = BOOT_FSM_STATE_RUN;
                rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK0);
                RTI_ResetFreeRunningCount();
                finishedOneAutoRun = true;
            }
        }

        switch (boot_state) {
            case BOOT_FSM_STATE_WAIT:
                /* Get the boot type based on can fsm state */
                boot_state = BOOT_GetBootState();
                break;

            case BOOT_FSM_STATE_LOAD:
                /* Get the boot type during load, it can only be BOOT_FSM_STATE_ERROR,
                BOOT_FSM_STATE_RESET or BOOT_FSM_STATE_LOAD */
                boot_state = BOOT_GetBootStateDuringLoad();
                break;

            case BOOT_FSM_STATE_RUN:
                /* Load the program info from flash to variable, check if a program
                has been loaded to flash, and check the validity of it by comparing
                the saved CRC signature and current calculated CRC signature */
                if (BOOT_IsProgramAvailableAndValidated()) {
                    if (BOOT_JumpInToLastFlashedProgram() == STD_OK) {
                        /* This line can not be reached in real hardware, it is here
                    for testing */
                        boot_state = BOOT_FSM_STATE_WAIT;
                    } else {
                        boot_state = BOOT_FSM_STATE_ERROR;
                    }
                } else {
                    boot_state = BOOT_FSM_STATE_WAIT;
                }
                break;

            case BOOT_FSM_STATE_RESET:
                /* Reset the bootloader to its original state */
                if (BOOT_ResetBootloader() == STD_NOT_OK) {
                    boot_state = BOOT_FSM_STATE_ERROR;
                } else {
                    /* This line can not be reached in real hardware, it is here
                    for testing */
                    boot_state = BOOT_FSM_STATE_WAIT;
                }
                break;

            case BOOT_FSM_STATE_ERROR:
                /* Wait for can request to handle the error (until now only reset) */
                boot_state = BOOT_GetBootStateDuringError();
                break;

            default:
                /* If boot_state is any state except for the registered states,
                assign the boot_state the error FSM state. */
                boot_state = BOOT_FSM_STATE_ERROR;
                break;
        }
    }
#pragma diag_push
#pragma diag_suppress 112
    /* AXIVION Next Codeline Style MisraC2012-2.1: we shall never get here when running on the target */
    return 1;
#pragma diag_pop
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
