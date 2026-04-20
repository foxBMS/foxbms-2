/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    contactor_how-to.h
 * @author  foxBMS Team
 * @date    2026-03-04 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup SOME_GROUP
 * @prefix  DATA
 *
 * @brief   Contactor Examples
 * @details Contactor Examples
 *
 */

#ifndef FOXBMS__CONTACTOR_HOW_TO_H_
#define FOXBMS__CONTACTOR_HOW_TO_H_

/*========== Includes =======================================================*/
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/
/** mock-up of the struct fields */
typedef struct {
} CONT_ELECTRICAL_STATE_TYPE_e;
typedef struct {
} CONT_ELECTRICAL_STATE_TYPE_e;
typedef struct {
} CONT_FEEDBACK_TYPE_e;
typedef struct {
} BS_STRING_ID_e;
typedef struct {
} CONT_TYPE_e;
typedef struct {
} SPS_CHANNEL_INDEX;
typedef struct {
} CONT_CURRENT_BREAKING_DIRECTION_e;

/* example-contactor-state-start*/
typedef struct {
    CONT_ELECTRICAL_STATE_TYPE_e currentSet;    /*!< current set point for the contactor */
    CONT_ELECTRICAL_STATE_TYPE_e feedback;      /*!< feedback from the contactor */
    const CONT_FEEDBACK_TYPE_e feedbackPinType; /*!< type of feedback that the contactor uses */
    const BS_STRING_ID_e stringIndex;           /*!< index in which string the contactor is placed */
    const CONT_TYPE_e type;                     /*!< type of contactor */
    const SPS_CHANNEL_INDEX spsChannel;         /*!< channel index of the SPS to which this contactor is connected */
    const CONT_CURRENT_BREAKING_DIRECTION_e breakingDirection; /*!< preferred contactor opening direction */
} CONT_CONTACTOR_STATE_s;
/*example-contactor-state-end*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CONTACTOR_HOW_TO_H_ */
