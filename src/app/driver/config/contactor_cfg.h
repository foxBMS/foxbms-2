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
 * @file    contactor_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-11 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  CONT
 *
 * @brief   Header for the configuration for the driver for the contactors
 *
 */

#ifndef FOXBMS__CONTACTOR_CFG_H_
#define FOXBMS__CONTACTOR_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

#include "sps_types.h"

/*========== Macros and Definitions =========================================*/

/*================== Main precharge configuration ====================*/

/*========== Extern Constant and Variable Declarations ======================*/
/** Symbolic names for the possible states of the contactors */
typedef enum CONT_ELECTRICAL_STATE_TYPE {
    CONT_SWITCH_OFF,   /*!< Contactor off       --> Contactor is open */
    CONT_SWITCH_ON,    /*!< Contactor on        --> Contactor is closed */
    CONT_SWITCH_UNDEF, /*!< Contactor undefined --> Contactor state not known */
} CONT_ELECTRICAL_STATE_TYPE_e;

/** Symbolic names for the contactors */
typedef enum CONT_NAMES {
    CONT_STRING0_PLUS,  /*!< Contactor in string0 plus path */
    CONT_STRING0_MINUS, /*!< Contactor in string0 minus path */
    CONT_STRING1_PLUS,  /*!< Contactor in string1 plus path */
    CONT_STRING1_MINUS, /*!< Contactor in string1 minus path */
    CONT_STRING2_PLUS,  /*!< Contactor in string2 plus path */
    CONT_STRING2_MINUS, /*!< Contactor in string2 minus path */
    CONT_PRECHARGE,     /*!< Precharge contactor */
} CONT_NAMES_e;

/** Symbolic names defining the electric behavior of the contactor */
typedef enum CONT_FEEDBACK_TYPE {
    CONT_FEEDBACK_NORMALLY_OPEN,   /*!< Feedback line of a contactor is normally open */
    CONT_FEEDBACK_NORMALLY_CLOSED, /*!< Feedback line of a contactor is normally closed */
    CONT_FEEDBACK_THROUGH_CURRENT, /*!< Get feedback information through the current flowing into the contactor */
    CONT_HAS_NO_FEEDBACK,          /*!< Feedback line of the contactor is not used */
} CONT_FEEDBACK_TYPE_e;

/** Status struct for a registry of all contactors */
typedef struct CONT_CONTACTOR_STATE {
    CONT_ELECTRICAL_STATE_TYPE_e currentSet;    /*!< current setpoint for the contactor */
    CONT_ELECTRICAL_STATE_TYPE_e feedback;      /*!< feedback from the contactor */
    const CONT_FEEDBACK_TYPE_e feedbackPinType; /*!< type of feedback that the contactor uses */
    const SPS_CHANNEL_INDEX spsChannel;         /*!< channel index of the SPS to which this contactor is connected */
    const CONT_NAMES_e name;                    /*!< name of the contactor */
} CONT_CONTACTOR_STATE_s;

/** central state registry of all contactors of the system */
extern CONT_CONTACTOR_STATE_s cont_contactorStates[BS_NR_OF_CONTACTORS];

/** index number of the contactor array; not to be confused with #SPS_CHANNEL_INDEX */
typedef uint8_t CONT_CONTACTOR_INDEX;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__CONTACTOR_CFG_H_ */
