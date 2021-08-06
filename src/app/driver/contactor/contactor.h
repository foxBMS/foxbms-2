/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @updated 2021-03-24 (date of last update)
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

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Gets the latest value the contactors were set to.
 * @param[in]   name    name of the contactor
 * @return  returns CONT_SWITCH_OFF or CONT_SWITCH_ON
 */
extern CONT_ELECTRICAL_STATE_TYPE_e CONT_GetContactorSetValue(const CONT_NAMES_e name);

/**
 * @brief   Reads the feedback pin of every contactor and returns its current
 *          value (#CONT_SWITCH_OFF/#CONT_SWITCH_ON).
 * @details If the contactor has a feedback pin the measured feedback is
 *          returned. If the contactor has no feedback pin, it is assumed that
 *          after a certain time the contactor has reached the requested state.
 */
void CONT_GetContactorFeedback(void);

/**
 * @brief   Sets the contactor state to its requested state, if the contactor
 *          is at that time not in the requested state.
 * @details If the new state was already requested, but not reached (meaning
 *          the measured feedback does not return the requested state), there
 *          are two states: it can be still ok (#STD_OK), because the contactor
 *          has some time left to get physically in the requested state (passed
 *          time since the request is lower than the limit) or it can be not ok
 *          (#STD_NOT_OK), because there is timing violation, i.e. the
 *          contactor has surpassed the maximum time for getting in the
 *          requested state. It returns #STD_OK if the requested state was
 *          successfully set or if the contactor was at the requested state
 *          before.
 * @param   name
 * @param   requestedContactorState
 * @return  retVal (type: STD_RETURN_TYPE_e)
 */
extern STD_RETURN_TYPE_e CONT_SetContactorState(
    const CONT_NAMES_e name,
    CONT_ELECTRICAL_STATE_TYPE_e requestedContactorState);

/** @brief Substates of the CONT state machine */
typedef enum {
    CONT_ENTRY,
    CONT_SET_EDGE_LOW_INIT,
    CONT_SET_EDGE_HIGH,
    CONT_SET_EDGE_LOW,
    CONT_CHECK_REQUESTS,
} CONT_STATEMACH_SUB_e;

/** @brief Names for connected power paths */
typedef enum CONT_POWER_LINE_e {
    CONT_POWER_PATH_NONE, /*!< no power line is connected, contactors are open  */
    CONT_POWER_PATH_0,    /*!< power line 0, e.g. used for the power train      */
#if BS_SEPARATE_POWER_PATHS == 1
    CONT_POWER_PATH_1, /*!< power line 1, e.g. used for charging                */
#endif
} CONT_POWER_LINE_e;

/**
 * @brief   Gets the current state.
 * @details This function is used in the functioning of the CONT state machine.
 * @param   contactorNumber     contactor to be addressed
 * @return  current state, taken from #CONT_ELECTRICAL_STATE_TYPE_e
 */
extern CONT_ELECTRICAL_STATE_TYPE_e CONT_GetState(uint8_t contactorNumber);

/**
 * @brief   Closes the contactor of a string.
 * @details This function makes a close state request to the contactor of a
 *          specific string.
 * @param stringNumber    String addressed
 */
extern STD_RETURN_TYPE_e CONT_CloseString(uint8_t stringNumber);

/**
 * @brief   Opens the contactor of a string.
 * @details This function makes an open state request to the contactor of a
 *          specific string.
 * @param stringNumber    String addressed
 */
extern STD_RETURN_TYPE_e CONT_OpenString(uint8_t stringNumber);

/**
 * @brief   Closes precharge.
 * @details This function makes a close state request to the precharge
 *          contactor.
 * @param stringNumber    String addressed
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
 * @brief   checks the feedback of the contactors
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
extern CONT_CONTACTOR_INDEX TEST_CONT_ResolveContactorName(const CONT_NAMES_e name);
#endif /* UNITY_UNIT_TEST */

#endif /* FOXBMS__CONTACTOR_H_ */
