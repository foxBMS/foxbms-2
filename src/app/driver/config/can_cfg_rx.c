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
 * @file    can_cfg_rx.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  CANRX
 *
 * @brief   Configuration for the CAN module
 * @details The CAN bus settings and the received messages and their reception
 *          handling are to be specified here.
 */

/*========== Includes =======================================================*/
#include "foxbms_config.h"

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
    {CAN_NODE_1, CANRX_BMS_STATE_REQUEST_MESSAGE, &CANRX_BmsStateRequest},
    {CAN_NODE_DEBUG_MESSAGE, CANRX_DEBUG_MESSAGE, &CANRX_Debug},

#if (defined(FOXBMS_IMD_BENDER_ISO165C) && (FOXBMS_IMD_BENDER_ISO165C == 1))
    {CAN_NODE_IMD, CANRX_IMD_BENDER_ISO165C_INFO_MESSAGE, &CANRX_ImdBenderIso165cInfo},
    {CAN_NODE_IMD, CANRX_IMD_BENDER_ISO165C_RESPONSE_MESSAGE, &CANRX_ImdBenderIso165cResponse},
#endif
#if (defined(FOXBMS_CS_ISABELLENHUETTE_IVT_S) && (FOXBMS_CS_ISABELLENHUETTE_IVT_S == 1))
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_MESSAGE, &CANRX_CsIsabellenhuetteIvtS},
#endif
#if (defined(FOXBMS_AS_HONEYWELL_BAS6C_X00) && (FOXBMS_AS_HONEYWELL_BAS6C_X00 == 1))
    {CAN_NODE_1, CANRX_AS_HONEYWELL_BAS6C_X00_MESSAGE, &CANRX_AsHoneywellBas6cX00},
#endif
#if (defined(FOXBMS_AFE_DRIVER_DEBUG_CAN) && (FOXBMS_AFE_DRIVER_DEBUG_CAN == 1))
    {CAN_NODE_RX_CELL_VOLTAGES, CANRX_AFE_CELL_VOLTAGES_MESSAGE, &CANRX_CellVoltages},
    {CAN_NODE_RX_CELL_TEMPERATURES, CANRX_AFE_CELL_TEMPERATURES_MESSAGE, &CANRX_CellTemperatures},
#endif
#if (defined(FOXBMS_CS_LEM_CAB500) && (FOXBMS_CS_LEM_CAB500 == 1))
    {CAN_NODE_CURRENT_SENSOR, CANRX_CS_LEM_CAB500_STRING0_MESSAGE, &CANRX_CsLemCab500},
#endif
};

/** length of CAN message arrays */
const uint8_t can_rxMessagesLength = sizeof(can_rxMessages) / sizeof(can_rxMessages[0]);

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
