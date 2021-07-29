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
 * @file    can_cbs_rx_command.c
 * @author  foxBMS Team
 * @date    2021-07-28 (date of creation)
 * @updated 2021-07-28 (date of last update)
 * @ingroup DRIVER
 * @prefix  CAN
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for command message
 */

/*========== Includes =======================================================*/
#include "bms_cfg.h"

#include "bal.h"
#include "can_cbs.h"
#include "can_helper.h"
#include "diag.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CAN_RxRequest(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);

    FAS_ASSERT(id < CAN_MAX_11BIT_ID); /* Currently standard ID, 11 bit */
    FAS_ASSERT(dlc <= CAN_MAX_DLC);    /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t message    = 0;
    uint64_t signalData = 0;

    DATA_READ_DATA(kpkCanShim->pTableStateRequest);

    CAN_RxGetMessageDataFromCanData(&message, pCanData, endianness);

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_RxGetSignalDataFromMessageData(message, 1u, 2u, &signalData, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

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
            stateRequest = BMS_REQ_ID_NOREQ;
            break;
    }
    kpkCanShim->pTableStateRequest->previousStateRequestViaCan = kpkCanShim->pTableStateRequest->stateRequestViaCan;
    kpkCanShim->pTableStateRequest->stateRequestViaCan         = stateRequest;
    if ((kpkCanShim->pTableStateRequest->stateRequestViaCan !=
         kpkCanShim->pTableStateRequest->previousStateRequestViaCan) ||
        ((OS_GetTickCount() - kpkCanShim->pTableStateRequest->header.timestamp) > 3000U)) { /* TODO: check pointer */
        kpkCanShim->pTableStateRequest->stateRequestViaCanPending = stateRequest;
    }
    kpkCanShim->pTableStateRequest->state++;

    /* Get balancing request */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_RxGetSignalDataFromMessageData(message, 8u, 1u, &signalData, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    if (STD_OK == BAL_GetInitializationState()) {
        if ((uint8_t)signalData == 0u) {
            BAL_SetStateRequest(BAL_STATE_GLOBAL_DISABLE_REQUEST);
        } else {
            BAL_SetStateRequest(BAL_STATE_GLOBAL_ENABLE_REQUEST);
        }
    }

    /* Get balancing threshold */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_RxGetSignalDataFromMessageData(message, 23u, 8u, &signalData, endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* TODO: Implement missing signals */

    DATA_WRITE_DATA(kpkCanShim->pTableStateRequest);

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
