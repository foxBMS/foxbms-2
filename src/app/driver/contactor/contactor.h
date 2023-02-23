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
 * @file    contactor.h
 * @author  foxBMS Team
 * @date    2020-02-11 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  CONT
 *
 * @brief   Headers for the driver for the contactors.
 *
 */

#ifndef FOXBMS__CONTACTOR_H_
#define FOXBMS__CONTACTOR_H_

/*========== Includes =======================================================*/
#include "contactor_cfg.h"

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Returns the current contactor state.
 * @details This function is used in the functioning of the CONT state machine.
 * @param   stringNumber string in which the contactor is placed
 * @param   contactorType contactor type for which the feedback is requested
 * @return  current state, taken from #CONT_ELECTRICAL_STATE_TYPE_e
 */
extern CONT_ELECTRICAL_STATE_TYPE_e CONT_GetContactorState(uint8_t stringNumber, CONT_TYPE_e contactorType);

/**
 * @brief   Opens the contactor
 * @details This function makes an open state request to the specific contactor
 * @param stringNumber    String addressed
 * @param contactor       contactor addressed
 */
extern STD_RETURN_TYPE_e CONT_OpenContactor(uint8_t stringNumber, CONT_TYPE_e contactor);

/**
 * @brief   Closes the contactor
 * @details This function makes an close state request to the specific contactor
 * @param stringNumber    String addressed
 * @param contactor       contactor addressed
 */
extern STD_RETURN_TYPE_e CONT_CloseContactor(uint8_t stringNumber, CONT_TYPE_e contactor);

/**
 * @brief   Closes precharge.
 * @details This function makes a close state request to the precharge
 *          contactor.
 * @param stringNumber    String addressed
 * @return #STD_OK if requested contactor exists and close requested, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e CONT_ClosePrecharge(uint8_t stringNumber);

/**
 * @brief   Opens precharge.
 * @details This function makes an open state request to the precharge
 *          contactor.
 * @param stringNumber    String addressed
 */
extern STD_RETURN_TYPE_e CONT_OpenPrecharge(uint8_t stringNumber);

/**
 * @brief   Open all currently closed precharge contactors
 * @details This function iterates over all contactors and opens all currently
 *          closed precharge contactors
 * @return  none (void)
 */
extern void CONT_OpenAllPrechargeContactors(void);

/**
 * @brief   checks the feedback of all contactors
 * @details makes a DIAG entry for each contactor when the feedback does not
 *          match the set value
 */
extern void CONT_CheckFeedback(void);

/**
 * @brief   initializes the contactor module
 */
extern void CONT_Initialize(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CONT_InitializationCheckOfContactorRegistry(void);
#endif

#endif /* FOXBMS__CONTACTOR_H_ */
