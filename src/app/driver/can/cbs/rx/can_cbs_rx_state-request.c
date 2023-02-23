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
 * @file    can_cbs_rx_state-request.c
 * @author  foxBMS Team
 * @date    2021-07-28 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for command message
 */

/*========== Includes =======================================================*/
#include "bms_cfg.h"

#include "bal.h"
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "diag.h"
#include "os.h"
#include "sys_mon.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * @brief   CAN state request update time
 * @details When a new CAN state request is received, it leads to an update
 *          of #DATA_BLOCK_STATE_REQUEST_s::stateRequestViaCan if one of the
 *          following conditions is met:
 *
 *             - The new request is different than the old request.
 *             - The old request is older than the timespan set in this define.
 */
#define CANRX_CAN_REQUEST_UPDATE_TIME_ms (3000u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   clears the persistent flags
 * @details This function clears all persistent flags (if signalData demands it)
 *          which are:
 *              - deep-discharge flag
 *              - sys mon violation flags
 * @param[in]   signalData  if it is 1u, flags are cleared
 */
static void CANRX_ClearAllPersistentFlags(uint64_t signalData);

/**
 * @brief   handles the mode request
 * @param[in]       signalData  extracted signal data
 * @param[in,out]   kpkCanShim  can shim with database entries
 */
static void CANRX_HandleModeRequest(uint64_t signalData, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   handles the balancing request
 * @param[in]       signalData  extracted signal data
 */
static void CANRX_HandleBalancingRequest(uint64_t signalData);

/**
 * @brief   sets the balancing threshold
 * @param[in]       signalData  extracted signal data
 */
static void CANRX_SetBalancingThreshold(uint64_t signalData);

/*========== Static Function Implementations ================================*/
static void CANRX_ClearAllPersistentFlags(uint64_t signalData) {
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    if (signalData == 1u) {
        /* clear deep discharge */
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            (void)DIAG_Handler(DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_OK, DIAG_STRING, s);
        }
        /* clear sys mon */
        SYSM_ClearAllTimingViolations();
    }
}

static void CANRX_HandleModeRequest(uint64_t signalData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */

    /** 0x00: Disconnect strings from HV bus
     *  0x01: Connect strings to HV bus to start discharge
     *  0x02: Connect strings to HV bus to start charging
     */
    uint8_t stateRequest = BMS_REQ_ID_NOREQ;

    switch (signalData) {
        case 0u:
            stateRequest = BMS_REQ_ID_STANDBY;
            break;
        case 1u:
            stateRequest = BMS_REQ_ID_NORMAL;
            break;
        case 2u:
            stateRequest = BMS_REQ_ID_CHARGE;
            break;
        default:
            /* default value already set in initialization */
            break;
    }
    kpkCanShim->pTableStateRequest->previousStateRequestViaCan = kpkCanShim->pTableStateRequest->stateRequestViaCan;
    kpkCanShim->pTableStateRequest->stateRequestViaCan         = stateRequest;
    if ((kpkCanShim->pTableStateRequest->stateRequestViaCan !=
         kpkCanShim->pTableStateRequest->previousStateRequestViaCan) ||
        (OS_CheckTimeHasPassed(kpkCanShim->pTableStateRequest->header.timestamp, CANRX_CAN_REQUEST_UPDATE_TIME_ms))) {
        kpkCanShim->pTableStateRequest->stateRequestViaCanPending = stateRequest;
    }
    if (kpkCanShim->pTableStateRequest->stateCounter == (uint8_t)UINT8_MAX) {
        /* overflow of state counter */
        kpkCanShim->pTableStateRequest->stateCounter = 0u;
    } else {
        kpkCanShim->pTableStateRequest->stateCounter++;
    }
}

static void CANRX_HandleBalancingRequest(uint64_t signalData) {
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */
    /* AXIVION Next Codeline Style MisraC2012-2.2 MisraC2012-14.3: Depending on implementation STD_NOT_OK might be returned. */
    if (BAL_GetInitializationState() == STD_OK) {
        if (signalData == 0u) {
            BAL_SetStateRequest(BAL_STATE_GLOBAL_DISABLE_REQUEST);
        } else {
            BAL_SetStateRequest(BAL_STATE_GLOBAL_ENABLE_REQUEST);
        }
    }
}

static void CANRX_SetBalancingThreshold(uint64_t signalData) {
    /* cap signal data to UINT16_MAX */
    int32_t cappedSignalData = (int32_t)signalData;
    if (signalData > (uint64_t)UINT16_MAX) {
        cappedSignalData = (uint64_t)UINT16_MAX;
    }
    BAL_SetBalancingThreshold(cappedSignalData);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_BmsStateRequest(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANRX_BMS_STATE_REQUEST_ID);
    FAS_ASSERT(message.idType == CANRX_BMS_STATE_REQUEST_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    DATA_READ_DATA(kpkCanShim->pTableStateRequest);

    uint64_t messageData = 0u;
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

    uint64_t signalData = 0;
    /* Get mode request */
    /* AXIVION Next Codeline Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_RxGetSignalDataFromMessageData(messageData, 1u, 2u, &signalData, message.endianness);
    CANRX_HandleModeRequest(signalData, kpkCanShim);

    /* check for reset flag */
    /* AXIVION Next Codeline Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_RxGetSignalDataFromMessageData(messageData, 2u, 1u, &signalData, message.endianness);
    CANRX_ClearAllPersistentFlags(signalData);

    /* Get balancing request */
    /* AXIVION Next Codeline Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_RxGetSignalDataFromMessageData(messageData, 8u, 1u, &signalData, message.endianness);
    CANRX_HandleBalancingRequest(signalData);

    /* Get balancing threshold */
    /* AXIVION Next Codeline Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_RxGetSignalDataFromMessageData(messageData, 23u, 8u, &signalData, message.endianness);
    CANRX_SetBalancingThreshold(signalData);

    /* TODO: Implement missing signals */

    DATA_WRITE_DATA(kpkCanShim->pTableStateRequest);

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
