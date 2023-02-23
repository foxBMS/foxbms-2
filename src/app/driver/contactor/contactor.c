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
 * @file    contactor.c
 * @author  foxBMS Team
 * @date    2020-02-11 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  CONT
 *
 * @brief   Driver for the contactors.
 *
 */

/*========== Includes =======================================================*/
#include "contactor.h"

#include "diag.h"
#include "fassert.h"
#include "fstd_types.h"
#include "mcu.h"
#include "sps.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Reads the feedback of all contactors
 * @details Gathers the feedback of all contactors from the configured feedback
 *          source (#CONT_FEEDBACK_TYPE_e). If #CONT_HAS_NO_FEEDBACK is
 *          configured,it is assumed that the contactor has reached the
 *          requested state.
 */
static void CONT_GetFeedbackOfAllContactors(void);

/** conducts an initialization test of the contactor registry and asserts at failure */
static void CONT_InitializationCheckOfContactorRegistry(void);

/*========== Static Function Implementations ================================*/
static void CONT_GetFeedbackOfAllContactors(void) {
    /* Iterate over all contactors */
    for (CONT_CONTACTOR_INDEX contactor = 0; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        /* Use different feedback sources depending on the individual contactor configuration */
        if (cont_contactorStates[contactor].feedbackPinType == CONT_HAS_NO_FEEDBACK) {
            /* no feedback: assume set value is true */
            cont_contactorStates[contactor].feedback = cont_contactorStates[contactor].currentSet;
        } else if (cont_contactorStates[contactor].feedbackPinType == CONT_FEEDBACK_THROUGH_CURRENT) {
            /* feedback from current: ask SPS driver for feedback */
            cont_contactorStates[contactor].feedback =
                SPS_GetChannelCurrentFeedback(cont_contactorStates[contactor].spsChannel);
        } else if (CONT_FEEDBACK_NORMALLY_OPEN == cont_contactorStates[contactor].feedbackPinType) {
            cont_contactorStates[contactor].feedback =
                SPS_GetChannelPexFeedback(cont_contactorStates[contactor].spsChannel, true);
        } else {
            /* CONT_FEEDBACK_NORMALLY_CLOSED */
            cont_contactorStates[contactor].feedback =
                SPS_GetChannelPexFeedback(cont_contactorStates[contactor].spsChannel, false);
        }
    }
}

static void CONT_InitializationCheckOfContactorRegistry(void) {
    /* Iterate over each contactor and compare found name with resolved name (if a name is used double this would
     always return the first entry and we would have the second entry from iterating here) */
    for (CONT_CONTACTOR_INDEX contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        /* TODO: add check that only one contactor of each type is configured for each string */

        /* Check that a configured SPS channel number is not higher than the number of available SPS channels */
        FAS_ASSERT(cont_contactorStates[contactor].spsChannel < SPS_NR_OF_AVAILABLE_SPS_CHANNELS);

        /* every contactor channel has to be affiliated with contactor */
        const SPS_CHANNEL_AFFILIATION_e channelAffiliation =
            SPS_GetChannelAffiliation(cont_contactorStates[contactor].spsChannel);
        FAS_ASSERT(SPS_AFF_CONTACTOR == channelAffiliation);
    }
}

/*========== Extern Function Implementations ================================*/
extern void CONT_CheckFeedback(void) {
    /* Get the feedback from all contactors from the configured feedback sources */
    CONT_GetFeedbackOfAllContactors();
    /* Check the contactor feedback */
    for (CONT_CONTACTOR_INDEX contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        /* TODO: add check that only one contactor of each type is configured for each string */
        DIAG_EVENT_e feedbackStatus = DIAG_EVENT_NOT_OK;
        if (cont_contactorStates[contactor].currentSet == cont_contactorStates[contactor].feedback) {
            /* Set feedback status to okay, if expected and actual contactor state match */
            feedbackStatus = DIAG_EVENT_OK;
        }
        switch (cont_contactorStates[contactor].type) {
            case CONT_PLUS:
                DIAG_Handler(
                    DIAG_ID_STRING_PLUS_CONTACTOR_FEEDBACK,
                    feedbackStatus,
                    DIAG_STRING,
                    (uint8_t)cont_contactorStates[contactor].stringIndex);
                break;
            case CONT_MINUS:
                DIAG_Handler(
                    DIAG_ID_STRING_MINUS_CONTACTOR_FEEDBACK,
                    feedbackStatus,
                    DIAG_STRING,
                    (uint8_t)cont_contactorStates[contactor].stringIndex);
                break;
            case CONT_PRECHARGE:
                DIAG_Handler(
                    DIAG_ID_PRECHARGE_CONTACTOR_FEEDBACK,
                    feedbackStatus,
                    DIAG_STRING,
                    (uint8_t)cont_contactorStates[contactor].stringIndex);
                break;
            default:
                /* Type: CONT_UNDEFINED -> trap */
                FAS_ASSERT(FAS_TRAP);
        }
    }
}

extern STD_RETURN_TYPE_e CONT_OpenContactor(uint8_t stringNumber, CONT_TYPE_e contactor) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(contactor != CONT_UNDEFINED);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* Iterate over contactor array, find the correct one and request state change via SPS module */
    for (uint8_t contactorIndex = 0u; contactorIndex < BS_NR_OF_CONTACTORS; contactorIndex++) {
        if (((BS_STRING_ID_e)stringNumber == cont_contactorStates[contactorIndex].stringIndex) &&
            (contactor == cont_contactorStates[contactorIndex].type)) {
            /* Set set state in cont_contactorStates array */
            cont_contactorStates[contactorIndex].currentSet = CONT_SWITCH_OFF;
            /* Request contactor state via SPS module */
            SPS_RequestContactorState(cont_contactorStates[contactorIndex].spsChannel, SPS_CHANNEL_OFF);
            retval = STD_OK;
            break;
        }
    }
    return retval;
}

extern STD_RETURN_TYPE_e CONT_CloseContactor(uint8_t stringNumber, CONT_TYPE_e contactor) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(contactor != CONT_UNDEFINED);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* Iterate over contactor array, find the correct one and request state change via SPS module */
    for (uint8_t contactorIndex = 0u; contactorIndex < BS_NR_OF_CONTACTORS; contactorIndex++) {
        if (((BS_STRING_ID_e)stringNumber == cont_contactorStates[contactorIndex].stringIndex) &&
            (contactor == cont_contactorStates[contactorIndex].type)) {
            /* Set set state in cont_contactorStates array */
            cont_contactorStates[contactorIndex].currentSet = CONT_SWITCH_ON;
            /* Request contactor state via SPS module */
            SPS_RequestContactorState(cont_contactorStates[contactorIndex].spsChannel, SPS_CHANNEL_ON);
            retval = STD_OK;
            break;
        }
    }
    return retval;
}

extern STD_RETURN_TYPE_e CONT_ClosePrecharge(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    /* Check if passed string has configured a precharge contactor */
    if (bs_stringsWithPrecharge[stringNumber] == BS_STRING_WITH_PRECHARGE) {
        retVal = CONT_CloseContactor(stringNumber, CONT_PRECHARGE);
    }
    return retVal;
}

extern STD_RETURN_TYPE_e CONT_OpenPrecharge(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    /* Check if passed string has configured a precharge contactor */
    if (bs_stringsWithPrecharge[stringNumber] == BS_STRING_WITH_PRECHARGE) {
        retVal = CONT_OpenContactor(stringNumber, CONT_PRECHARGE);
    }
    return retVal;
}

extern void CONT_OpenAllPrechargeContactors(void) {
    /* Iterate over all contactors and open all precharge contactors */
    for (uint8_t contactorIndex = 0u; contactorIndex < BS_NR_OF_CONTACTORS; contactorIndex++) {
        /* Set off a request to open all precharge contactors regardless of
         * their current state as we want to reach a safe state */
        if (cont_contactorStates[contactorIndex].type == CONT_PRECHARGE) {
            /* Request opening via SPS module */
            SPS_RequestContactorState(cont_contactorStates[contactorIndex].spsChannel, SPS_CHANNEL_OFF);
            cont_contactorStates[contactorIndex].currentSet = CONT_SWITCH_OFF;
        }
    }
}

extern CONT_ELECTRICAL_STATE_TYPE_e CONT_GetContactorState(uint8_t stringNumber, CONT_TYPE_e contactorType) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(contactorType != CONT_UNDEFINED);
    CONT_ELECTRICAL_STATE_TYPE_e feedback = CONT_SWITCH_UNDEFINED;
    /* Iterate over all contactors and return the requested feedback */
    for (uint8_t contactorIndex = 0u; contactorIndex < BS_NR_OF_CONTACTORS; contactorIndex++) {
        if ((cont_contactorStates[contactorIndex].stringIndex == (BS_STRING_ID_e)stringNumber) &&
            (contactorType == cont_contactorStates[contactorIndex].type)) {
            feedback = cont_contactorStates[contactorIndex].feedback;
            break;
        }
    }
    return feedback;
}

extern void CONT_Initialize(void) {
    CONT_InitializationCheckOfContactorRegistry();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CONT_InitializationCheckOfContactorRegistry() {
    CONT_InitializationCheckOfContactorRegistry();
}
#endif
