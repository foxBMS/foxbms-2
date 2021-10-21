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
 * @file    contactor.c
 * @author  foxBMS Team
 * @date    2020-02-11 (date of creation)
 * @updated 2021-10-01 (date of last update)
 * @ingroup DRIVERS
 * @prefix  CONT
 *
 * @brief   Driver for the contactors.
 *
 */

/*========== Includes =======================================================*/
#include "contactor.h"

#include "mcu.h"
#include "sps.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Resolves the contactor index from a contactor name
 * @details Searches in the contactor index by iteration from front to back for
 *          the contactor name and returns the first hit.
 * @param[in]   name    Name of the contactor
 * @returns     index number of the entry in the contactor array
 */
static CONT_CONTACTOR_INDEX CONT_ResolveContactorName(const CONT_NAMES_e name);

/**
 * @brief   Gets the contactor name from a contactor index
 * @param[in]   index   index number of the contactor
 * @returns     Returns the name of the contactor
 */
static CONT_NAMES_e CONT_GetContactorName(const CONT_CONTACTOR_INDEX index);

/**
 * @brief   Returns the sps channel index from a contactor index
 * @details Currently this implementation maps simply in ascending order
 */
static SPS_CHANNEL_INDEX CONT_GetSpsChannelIndexFromContactor(const CONT_CONTACTOR_INDEX contactor);

/**
 * @brief   Sets SPS channels according to contactors
 * @details Currently this implementation assumes simple ascending order.
 * @param[in]   contactor   index of the contactor that should be set
 */
static void CONT_SetSpsChannelFromContactor(const CONT_CONTACTOR_INDEX contactor);

/** conducts an initialization test of the contactor registry and asserts at failure */
static void CONT_InitializationCheckOfContactorRegistry(void);

/*========== Static Function Implementations ================================*/
static CONT_CONTACTOR_INDEX CONT_ResolveContactorName(const CONT_NAMES_e name) {
    CONT_CONTACTOR_INDEX contactor = 0u;
    bool hit                       = false;
    do {
        if (name == cont_contactorStates[contactor].name) {
            /* contactor found, therefore abort */
            hit = true;
        } else {
            /* continue and increase contactor counter */
            contactor++;
        }
    } while ((contactor < BS_NR_OF_CONTACTORS) && (true != hit));

    /* an unknown contactor may not exist */
    FAS_ASSERT(true == hit);

    return contactor;
}

static CONT_NAMES_e CONT_GetContactorName(const CONT_CONTACTOR_INDEX index) {
    FAS_ASSERT(index < BS_NR_OF_CONTACTORS);
    return cont_contactorStates[index].name;
}

static SPS_CHANNEL_INDEX CONT_GetSpsChannelIndexFromContactor(const CONT_CONTACTOR_INDEX contactor) {
    FAS_ASSERT(contactor < BS_NR_OF_CONTACTORS);
    return cont_contactorStates[contactor].spsChannel;
}

static void CONT_SetSpsChannelFromContactor(const CONT_CONTACTOR_INDEX contactor) {
    FAS_ASSERT(contactor < BS_NR_OF_CONTACTORS);
    switch (cont_contactorStates[contactor].currentSet) {
        case CONT_SWITCH_UNDEF:
        case CONT_SWITCH_OFF:
            SPS_RequestContactorState(CONT_GetSpsChannelIndexFromContactor(contactor), SPS_CHANNEL_OFF);
            break;
        case CONT_SWITCH_ON:
            SPS_RequestContactorState(CONT_GetSpsChannelIndexFromContactor(contactor), SPS_CHANNEL_ON);
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

static void CONT_InitializationCheckOfContactorRegistry(void) {
    /* iterate over each contactor and compare found name with resolved name (if a name is used double this would
     always return the first entry and we would have the second entry from iterating here) */
    for (CONT_CONTACTOR_INDEX contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        FAS_ASSERT(contactor == CONT_ResolveContactorName(cont_contactorStates[contactor].name));
        FAS_ASSERT(cont_contactorStates[contactor].name == CONT_GetContactorName(contactor));

        /* convention at the moment: sps channel index has to be the same as contactor index; this may change in
           future implementations */
        FAS_ASSERT(contactor == cont_contactorStates[contactor].spsChannel);

        /* every contactor channel has to be affiliated with contactor */
        FAS_ASSERT(SPS_AFF_CONTACTOR == SPS_GetChannelAffiliation(cont_contactorStates[contactor].spsChannel));
    }
}

/*========== Extern Function Implementations ================================*/
extern CONT_ELECTRICAL_STATE_TYPE_e CONT_GetContactorSetValue(const CONT_NAMES_e name) {
    CONT_CONTACTOR_INDEX contactorNumber                 = CONT_ResolveContactorName(name);
    CONT_ELECTRICAL_STATE_TYPE_e contactorSetInformation = cont_contactorStates[contactorNumber].currentSet;
    return contactorSetInformation;
}

extern void CONT_GetContactorFeedback(void) {
    for (CONT_CONTACTOR_INDEX contactor = 0; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        if (CONT_HAS_NO_FEEDBACK == cont_contactorStates[contactor].feedbackPinType) {
            /* no feedback: assume set value is true */
            cont_contactorStates[contactor].feedback = cont_contactorStates[contactor].currentSet;
        } else if (CONT_FEEDBACK_THROUGH_CURRENT == cont_contactorStates[contactor].feedbackPinType) {
            /* feedback from current: ask SPS driver for feedback */
            cont_contactorStates[contactor].feedback =
                SPS_GetChannelCurrentFeedback(CONT_GetSpsChannelIndexFromContactor(contactor));
        } else if (CONT_FEEDBACK_NORMALLY_OPEN == cont_contactorStates[contactor].feedbackPinType) {
            cont_contactorStates[contactor].feedback =
                SPS_GetChannelPexFeedback(CONT_GetSpsChannelIndexFromContactor(contactor), true);
        } else {
            /* CONT_FEEDBACK_NORMALLY_CLOSED */
            cont_contactorStates[contactor].feedback =
                SPS_GetChannelPexFeedback(CONT_GetSpsChannelIndexFromContactor(contactor), false);
        }
    }
}

extern STD_RETURN_TYPE_e CONT_SetContactorState(
    const CONT_NAMES_e name,
    CONT_ELECTRICAL_STATE_TYPE_e requestedContactorState) {
    CONT_CONTACTOR_INDEX contactor = CONT_ResolveContactorName(name);
    FAS_ASSERT(contactor < BS_NR_OF_CONTACTORS);
    FAS_ASSERT(
        (CONT_SWITCH_OFF == requestedContactorState) || (CONT_SWITCH_ON == requestedContactorState) ||
        (CONT_SWITCH_UNDEF == requestedContactorState));

    STD_RETURN_TYPE_e retVal = STD_OK;

    if (requestedContactorState == CONT_SWITCH_ON) {
        cont_contactorStates[contactor].currentSet = CONT_SWITCH_ON;
    } else if (requestedContactorState == CONT_SWITCH_OFF) {
        cont_contactorStates[contactor].currentSet = CONT_SWITCH_OFF;
    } else {
        retVal = STD_NOT_OK;
    }

    CONT_SetSpsChannelFromContactor(contactor);

    return retVal;
}

/**
 * @brief   checks the feedback of the contactors
 *
 * @details makes a DIAG entry for each contactor when the feedback does not match the set value
 */
extern void CONT_CheckFeedback(void) {
    CONT_GetContactorFeedback();
    /* TODO implement when contactor feedback is available */
}

extern STD_RETURN_TYPE_e CONT_CloseString(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* String contactors in the list start at index 0
     * so string number corresponds to contactor index
     */
    return CONT_SetContactorState(CONT_GetContactorName(stringNumber), CONT_SWITCH_ON);
}

extern STD_RETURN_TYPE_e CONT_OpenString(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* String contactors in the list start at index 0
     * so string number corresponds to contactor index
     */
    /* TODO: implement current check and similar when opening */
    return CONT_SetContactorState(CONT_GetContactorName(stringNumber), CONT_SWITCH_OFF);
}

extern STD_RETURN_TYPE_e CONT_ClosePrecharge(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retVal         = STD_NOT_OK;
    uint8_t prechargeContactorNumber = 0u;
    static_assert((BS_NR_OF_STRINGS <= (uint8_t)UINT8_MAX), "This code assumes BS_NR_OF_STRINGS fits into uint8_t");

    /* Precharge contactors in the list stay after string contactors
     * so it has index (number of contactors)-1
     */
    if (bs_stringsWithPrecharge[stringNumber] == BS_STRING_WITH_PRECHARGE) {
        /* Find contactor number corresponding to precharge of the requested string */
        for (uint8_t stringPrechargeNumber = 0u; stringPrechargeNumber < BS_NR_OF_STRINGS; stringPrechargeNumber++) {
            if (bs_stringsWithPrecharge[stringPrechargeNumber] == BS_STRING_WITH_PRECHARGE) {
                if (stringPrechargeNumber == stringNumber) {
                    break;
                }
                prechargeContactorNumber++;
            }
        }
        if ((BS_NR_OF_STRINGS + prechargeContactorNumber) < BS_NR_OF_CONTACTORS) {
            retVal = CONT_SetContactorState(
                CONT_GetContactorName(BS_NR_OF_STRINGS + prechargeContactorNumber), CONT_SWITCH_ON);
        }
    }
    return retVal;
}

extern STD_RETURN_TYPE_e CONT_OpenPrecharge(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    STD_RETURN_TYPE_e retVal         = STD_NOT_OK;
    uint8_t prechargeContactorNumber = 0u;

    /* Precharge contactors in the list stay after string contactors
     * so it has index (number of contactors)-1
     */
    if (bs_stringsWithPrecharge[stringNumber] == BS_STRING_WITH_PRECHARGE) {
        /* Find contactor number corresponding to precharge of the requested string */
        for (uint8_t stringPrechargeNumber = 0u; stringPrechargeNumber < BS_NR_OF_STRINGS; stringPrechargeNumber++) {
            if (bs_stringsWithPrecharge[stringPrechargeNumber] == BS_STRING_WITH_PRECHARGE) {
                if (stringPrechargeNumber == stringNumber) {
                    break;
                }
                prechargeContactorNumber++;
            }
        }
        if ((BS_NR_OF_STRINGS + prechargeContactorNumber) < BS_NR_OF_CONTACTORS) {
            retVal = CONT_SetContactorState(
                CONT_GetContactorName(BS_NR_OF_STRINGS + prechargeContactorNumber), CONT_SWITCH_OFF);
        }
    }
    return retVal;
}

extern CONT_ELECTRICAL_STATE_TYPE_e CONT_GetState(uint8_t contactorNumber) {
    FAS_ASSERT(contactorNumber < BS_NR_OF_CONTACTORS);
    return (cont_contactorStates[contactorNumber].feedback);
}

extern void CONT_Initialize(void) {
    CONT_InitializationCheckOfContactorRegistry();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CONT_InitializationCheckOfContactorRegistry() {
    CONT_InitializationCheckOfContactorRegistry();
}
extern CONT_CONTACTOR_INDEX TEST_CONT_ResolveContactorName(const CONT_NAMES_e name) {
    return CONT_ResolveContactorName(name);
}
#endif /* UNITY_UNIT_TEST */
