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
 * @file    can_cbs_rx_imd.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for IMD messages
 */

/*========== Includes =======================================================*/
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_ImdInfo(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    /* This handler is only implemented for little endian */
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(message.id == CANRX_IMD_INFO_ID);
    FAS_ASSERT(message.idType == CANRX_IMD_ID_TYPE);
    FAS_ASSERT(message.dlc <= CAN_MAX_DLC); /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    CAN_BUFFER_ELEMENT_s canMessage = {0};
    uint32_t retVal                 = 1u;
    const uint8_t boundedDlc        = MATH_MinimumOfTwoUint8_t(message.dlc, CAN_MAX_DLC);

    canMessage.id = message.id;
    for (uint8_t i = 0; i < boundedDlc; i++) {
        canMessage.data[i] = kpkCanData[i];
    }
    if (OS_SendToBackOfQueue(*(kpkCanShim->pQueueImd), (void *)&canMessage, 0u) == OS_SUCCESS) {
        retVal = 0u;
    }
    return retVal;
}

extern uint32_t CANRX_ImdResponse(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    /* This handler is only implemented for little endian */
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(message.id == CANRX_IMD_RESPONSE_ID);
    FAS_ASSERT(message.idType == CANRX_IMD_ID_TYPE);
    FAS_ASSERT(message.dlc <= CAN_MAX_DLC); /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    const uint8_t boundedDlc = MATH_MinimumOfTwoUint8_t(message.dlc, CAN_MAX_DLC);

    CAN_BUFFER_ELEMENT_s canMessage = {0};
    uint32_t retVal                 = 1u;

    canMessage.id = message.id;
    for (uint8_t i = 0; i < boundedDlc; i++) {
        canMessage.data[i] = kpkCanData[i];
    }
    if (OS_SendToBackOfQueue(*(kpkCanShim->pQueueImd), (void *)&canMessage, 0u) == OS_SUCCESS) {
        retVal = 0u;
    }
    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
