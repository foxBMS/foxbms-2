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
 * @file    can_cbs_tx_bms-state-details.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"
#include "diag.h"
#include "sys_mon.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANTX_BIT (1u)

/**
 * Configuration of the signals
*/
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_START_BIT          (4u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_LENGTH             (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_START_BIT               (3u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_LENGTH                  (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_START_BIT                (2u)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_LENGTH                   (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_START_BIT                 (1u)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_LENGTH                    (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_START_BIT              (0u)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_LENGTH                 (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_RECORDED_START_BIT (12u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_RECORDED_LENGTH    (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_RECORDED_START_BIT      (11u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_RECORDED_LENGTH         (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_RECORDED_START_BIT       (10u)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_RECORDED_LENGTH          (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_RECORDED_START_BIT        (9u)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_RECORDED_LENGTH           (CANTX_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_RECORDED_START_BIT     (8u)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_RECORDED_LENGTH        (CANTX_BIT)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/

#ifdef UNITY_UNIT_TEST
#endif
