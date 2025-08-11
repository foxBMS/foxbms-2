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
 * @file    boot.h
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  BOOT
 *
 * @brief   Header for the boot.c that implements the booting workflow of
 *          bootloader
 * @details TODO
 */

#ifndef FOXBMS__BOOT_H_
#define FOXBMS__BOOT_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include "boot_helper.h"
#include "fstd_types.h"

#include <stdbool.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Get the boot state machine's state.
 * @return  BOOT_FSM_STATES_e type.
 */
extern BOOT_FSM_STATES_e BOOT_GetBootState(void);

/**
 * @brief   Get the boot state machine's state during load.
 * @return  BOOT_FSM_STATES_e type.
 */
extern BOOT_FSM_STATES_e BOOT_GetBootStateDuringLoad(void);

/**
 * @brief   Get the boot state machine's state during error state.
 * @return  BOOT_FSM_STATES_e type.
 */
extern BOOT_FSM_STATES_e BOOT_GetBootStateDuringError(void);

/**
 * @brief   Jump into the last flashed program
 * @return  #STD_NOT_OK if there is some error appearing; if no error, the
 *          flashed program will be run.
 */
extern STD_RETURN_TYPE_e BOOT_JumpInToLastFlashedProgram(void);

/**
 * @brief   Check if there is a program available at the place it supposed to
 *          be and the program has been validated once.
 * @return  true, if the condition can be met; false, if not.
 */
extern bool BOOT_IsProgramAvailableAndValidated(void);

/**
 * @brief   Reset the boot process
 * @details Reset all relevant variables, all states, and erase the sector.
 * @return  #STD_OK if this boot process has been successfully reset,
 *          #STD_NOT_OK if not.
 */
extern STD_RETURN_TYPE_e BOOT_ResetBootloader(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_BOOT_WriteAndValidateCurrentSector(void);
extern uint8_t boot_numOfCurrentCrcValidation;
#endif

#endif /* FOXBMS__BOOT_H_ */
