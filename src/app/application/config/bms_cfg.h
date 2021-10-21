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
 * @file    bms_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-07-29 (date of last update)
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  BMS
 *
 * @brief   bms driver configuration header
 */

#ifndef FOXBMS__BMS_CFG_H_
#define FOXBMS__BMS_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "ftask_cfg.h"

/*========== Macros and Definitions =========================================*/

/** ID to send a message without request */
#define BMS_REQ_ID_NOREQ (0u)

/** ID to request the STANDBY state via CAN */
#define BMS_REQ_ID_STANDBY (3u)

/** ID to request the NORMAL state via CAN */
#define BMS_REQ_ID_NORMAL (1u)

/** ID to request the CHARGE state via CAN */
#define BMS_REQ_ID_CHARGE (2u)

/**
 * @brief   Functions searching for next string return this value if no further
 *          string is available */
#define BMS_NO_STRING_AVAILABLE (255u)

/**
 * @brief   TODO
 * @details This define MUST represent the cycle time of the task in which
 *          context the functions run, e.g., if the #BMS_Trigger() is running
 *          in the 10 ms task then the define must be set to 10.
 *          This sets the minimum time between two subsequent executed
 *          states/substates.
 *          Define is only used for compile-time assertion, it has no
 *          programatic influence on the actual code.
 */
#define BMS_STATEMACHINE_TASK_CYCLE_CONTEXT_MS (10u)

#if BMS_STATEMACHINE_TASK_CYCLE_CONTEXT_MS != FTSK_TASK_CYCLIC_10MS_CYCLE_TIME
#error "Invalid BMS configuration. Make sure that BMS timing is configured correctly!"
#endif

/**
 * @brief   BMS statemachine short time definition in #BMS_Trigger() calls
 *          until next state/substate is processed
 */
#define BMS_STATEMACH_SHORTTIME (1u)

/**
 * @brief   BMS statemachine medium time definition in #BMS_Trigger() calls
 *          until next state/substate is processed
 */
#define BMS_STATEMACH_MEDIUMTIME (5u)

/**
 * @brief   BMS statemachine long time definition in #BMS_Trigger() calls until
 *          next state/substate is processed
 */
#define BMS_STATEMACH_LONGTIME (10u)

/** Time in #BMS_Trigger() calls to wait after closing a string */
#define BMS_TIME_WAIT_AFTER_STRING_CLOSED (20u)

/** Time in #BMS_Trigger() calls to wait after opening a string */
#define BMS_TIME_WAIT_AFTER_OPENING_STRING (10u)

/**
 * @brief   Timeout in #BMS_Trigger() calls when closing a string after which
 *          the state machines goes to error if the string still has not closed
 */
#define BMS_STRING_CLOSE_TIMEOUT (500u)

/** Timeout in #BMS_Trigger() calls to wait after opening a string */
#define BMS_STRING_OPEN_TIMEOUT (1000u)

/**
 * @brief   Max voltage difference in mV between two strings to allow
 * closing the next string
 */
#define BMS_NEXT_STRING_VOLTAGE_LIMIT_MV (3000)

/** Max average string current to allow closing next string */
#define BMS_AVERAGE_STRING_CURRENT_LIMIT_MA (20000)

/** Delay after closing precharge in #BMS_Trigger() calls */
#define BMS_TIME_WAIT_AFTER_CLOSING_PRECHARGE (100u)

/** Delay after opening precharge in #BMS_Trigger() calls */
#define BMS_TIME_WAIT_AFTER_OPENING_PRECHARGE (50u)

/**
 * @brief   Time to wait in #BMS_Trigger() calls after precharge opened because
 *          precharge failed
 */
#define BMS_TIME_WAIT_AFTERPRECHARGEFAIL (300U)

/**
 * @brief   Timeout in 1*10ms to wait before re-entering to precharge
 * @details Prevents mechanical close/open timer cycle in case the control
 *          units sends incorrect state requests.
 */
#define BMS_OSCILLATION_TIMEOUT (1000u)

/** Number of allowed tries to close contactors */
#define BMS_PRECHARGE_TRIES (3u)

/** Precharge threshold limit on voltage (in mV) */
#define BMS_PRECHARGE_VOLTAGE_THRESHOLD_mV (1000LL) /* mV */

/** Precharge threshold limit on current (in mA) */
#define BMS_PRECHARGE_CURRENT_THRESHOLD_mA (50) /* mA */

/**
 * @details Time to wait after contactors opened because precharge failed in
 *          #BMS_Trigger() calls
 */
#define BMS_STATEMACH_TIMEAFTERPRECHARGEFAIL (100u)

/**
 * @details Timeout in #BMS_Trigger() calls when closing precharge after which
 *          the state machines goes to error if precharge still has not closed
 */
#define BMS_PRECHARGE_CLOSE_TIMEOUT (500u)

/**
 * @details Timeout in #BMS_Trigger() calls when opening precharge after which
 *          the state machines goes to error if precharge still has not opened
 */
#define BMS_PRECHARGE_OPEN_TIMEOUT (500u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__BMS_CFG_H_ */
