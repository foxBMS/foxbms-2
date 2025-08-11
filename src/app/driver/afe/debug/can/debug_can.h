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
 * @file    debug_can.h
 * @author  foxBMS Team
 * @date    2024-04-08 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  DECAN
 *
 * @brief   Header for the driver implementation of the fake CAN to AFE driver.
 * @details TODO
 */

#ifndef FOXBMS__DEBUG_CAN_H_
#define FOXBMS__DEBUG_CAN_H_

/*========== Includes =======================================================*/
#include "database_cfg.h"

#include "fstd_types.h"
#include "os.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Maximum queue timeout time in milliseconds */
#define DECAN_CAN2AFE_QUEUE_TIMEOUT_MS ((TickType_t)0u)

/** Define the invalid and valid of data */
#define DECAN_DATA_IS_INVALID (0u)

/** Define the invalid and valid of data */
#define DECAN_DATA_IS_VALID (1u)

/** Handle of the queue to transmit the received can message (cell temperature / cell voltage) to the debug/can afe */
extern OS_QUEUE ftsk_canToAfeCellTemperaturesQueue;

/** Handle of the queue to transmit the received can message (cell temperature / cell voltage) to the debug/can afe */
extern OS_QUEUE ftsk_canToAfeCellVoltagesQueue;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/** @brief  initialize driver */
extern STD_RETURN_TYPE_e DECAN_Initialize(void);

/** @brief  afe trigger function */
extern STD_RETURN_TYPE_e DECAN_TriggerAfe(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern uint16_t TEST_DECAN_ModifiedModuloFunction(uint16_t a, uint16_t b);
extern void TEST_DECAN_ConvertIndexForVoltage(uint16_t *s, uint16_t *m, uint16_t *cb, uint16_t oneNumIdxOfVoltage);
extern void TEST_DECAN_ConvertIndexForTemperature(
    uint16_t *s,
    uint16_t *m,
    uint16_t *ts,
    uint16_t oneNumIdxOfTemperature);
extern STD_RETURN_TYPE_e TEST_DECAN_ReceiveCanCellVoltages(void);
extern STD_RETURN_TYPE_e TEST_DECAN_ReceiveCanCellTemperatures(void);
#endif

#endif /* FOXBMS__DEBUG_CAN_H_ */
