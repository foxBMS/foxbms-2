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
 * @file    contactor_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-11 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  CONT
 *
 * @brief   Header for the configuration for the driver for the contactors
 *
 */

#ifndef FOXBMS__CONTACTOR_CFG_H_
#define FOXBMS__CONTACTOR_CFG_H_

/*========== Includes =======================================================*/

#include "battery_system_cfg.h"

#include "sps_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/
/** Symbolic names for the possible states of the contactors */
typedef enum {
    CONT_SWITCH_OFF,       /*!< Contactor off       --> Contactor is open */
    CONT_SWITCH_ON,        /*!< Contactor on        --> Contactor is closed */
    CONT_SWITCH_UNDEFINED, /*!< Contactor undefined --> Contactor state not known */
} CONT_ELECTRICAL_STATE_TYPE_e;

/** Symbolic names defining the electric behavior of the contactor */
typedef enum {
    CONT_FEEDBACK_NORMALLY_OPEN,   /*!< Feedback line of a contactor is normally open */
    CONT_FEEDBACK_NORMALLY_CLOSED, /*!< Feedback line of a contactor is normally closed */
    CONT_FEEDBACK_THROUGH_CURRENT, /*!< Get feedback information through the current flowing into the contactor */
    CONT_HAS_NO_FEEDBACK,          /*!< Feedback line of the contactor is not used */
} CONT_FEEDBACK_TYPE_e;

/** Contactor type */
typedef enum {
    CONT_PLUS,      /*!< Contactor is placed in HV plus path */
    CONT_MINUS,     /*!< Contactor is placed in HV minus path */
    CONT_PRECHARGE, /*!< Contactor is used as precharge contactor, installed as a plus contactor */
    CONT_UNDEFINED, /*!< Undefined contactor */
} CONT_TYPE_e;

/**
 * Some contactors are designed and optimized to open current that is flowing
 * in one direction to provide maximum current breaking capability. This
 * direction is then dependent on how the contactors are installed withing the
 * battery system. If bidirectional contactors are used, the main power
 * terminals can be connected in either direction and the current breaking
 * capability is the same for both directions.
 */
typedef enum {
    CONT_CHARGING_DIRECTION,    /*!< Contactor is preferred opened in charge current direction */
    CONT_DISCHARGING_DIRECTION, /*!< Contactor is preferred opened in discharge current direction */
    CONT_BIDIRECTIONAL,         /*!< Contactor has no preferred way of being opened depending on the current flow */
} CONT_CURRENT_BREAKING_DIRECTION_e;

/** Status struct for a registry of all contactors */
typedef struct {
    CONT_ELECTRICAL_STATE_TYPE_e currentSet;    /*!< current setpoint for the contactor */
    CONT_ELECTRICAL_STATE_TYPE_e feedback;      /*!< feedback from the contactor */
    const CONT_FEEDBACK_TYPE_e feedbackPinType; /*!< type of feedback that the contactor uses */
    const BS_STRING_ID_e stringIndex;           /*!< index in which string the contactor is placed */
    const CONT_TYPE_e type;                     /*!< type of contactor */
    const SPS_CHANNEL_INDEX spsChannel;         /*!< channel index of the SPS to which this contactor is connected */
    const CONT_CURRENT_BREAKING_DIRECTION_e breakingDirection; /*!< preferred contactor opening direction */
} CONT_CONTACTOR_STATE_s;

/** index number of the contactor array; not to be confused with #SPS_CHANNEL_INDEX */
typedef uint8_t CONT_CONTACTOR_INDEX;

/** central state registry of all contactors of the system */
extern CONT_CONTACTOR_STATE_s cont_contactorStates[BS_NR_OF_CONTACTORS];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CONTACTOR_CFG_H_ */
