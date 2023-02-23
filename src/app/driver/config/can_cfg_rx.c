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
 * @file    can_cfg_rx.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  CANRX
 *
 * @brief   Configuration for the CAN module
 *
 * @details The CAN bus settings and the received messages and their reception
 *          handling are to be specified here.
 *
 *
 */

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "database.h"
#include "ftask.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/** registry of CAN RX messages */
const CAN_RX_MESSAGE_TYPE_s can_rxMessages[] = {
    {CAN_NODE_IMD, CANRX_IMD_INFO_MESSAGE, &CANRX_ImdInfo},
    {CAN_NODE_IMD, CANRX_IMD_RESPONSE_MESSAGE, &CANRX_ImdResponse},
    {CAN_NODE_1, CANRX_BMS_STATE_REQUEST_MESSAGE, &CANRX_BmsStateRequest},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_CURRENT_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_VOLTAGE1_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_VOLTAGE2_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_VOLTAGE3_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_TEMPERATURE_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_POWER_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_CURRENT_COUNTER_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_CURRENT_SENSOR, CANRX_STRING0_ENERGY_COUNTER_MESSAGE, &CANRX_CurrentSensor},
    {CAN_NODE_DEBUG_MESSAGE, CANRX_DEBUG_MESSAGE, &CANRX_Debug},
};

/** length of CAN message arrays */
const uint8_t can_rxLength = sizeof(can_rxMessages) / sizeof(can_rxMessages[0]);

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
