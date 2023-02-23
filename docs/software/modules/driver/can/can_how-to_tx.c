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
 * @file    can_how-to_tx.c
 * @author  foxBMS Team
 * @date    2022-09-12 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup SOME_GROUP
 * @prefix  CAN
 *
 * @brief   Documentation file to show how a new CAN message is added to the
 *          project.
 *
 */

/*========== Includes =======================================================*/

#include "can_cfg.h"

/*========== Macros and Definitions =========================================*/

/* example-can-message-id-macro-start */
/** CAN message properties for BMS state message. Required properties are:
 * - Message ID
 * - Identifier type (standard or extended)
 * - Message period and phase in ms
 * - Endianness of message data @{*/
#define CANTX_BMS_STATE_ID         (0x220u)
#define CANTX_BMS_STATE_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_BMS_STATE_PERIOD_ms  (100u)
#define CANTX_BMS_STATE_PHASE_ms   (0u)
#define CANTX_BMS_STATE_ENDIANNESS (CAN_BIG_ENDIAN)
/**@}*/
/* example-can-message-id-macro-end */

/* example-can-message-id-details-start */
#define CANTX_BMS_STATE_MESSAGE                                                \
    {                                                                          \
        .dlc        = CAN_DEFAULT_DLC,                                         \
        .id         = CANTX_BMS_STATE_ID,                                      \
        .endianness = CANTX_BMS_STATE_ENDIANNESS,                              \
        .idType     = CANTX_BMS_STATE_ID_TYPE,                                 \
    },                                                                         \
    {                                                                          \
        .period = CANTX_BMS_STATE_PERIOD_ms, .phase = CANTX_BMS_STATE_PHASE_ms \
    }
/* example-can-message-id-details-end */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/* example-can-tx-messages-callback-declaration-start */
extern uint32_t CANTX_BmsState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);
/* example-can-tx-messages-callback-declaration-end */

/* example-can-tx-messages-configuration-start */
const CAN_TX_MESSAGE_TYPE_s can_txMessages[] = {
    /* other messages */
    {CAN_NODE_1, CANTX_BMS_STATE_MESSAGE, &CANTX_BmsState, NULL_PTR}, /*!< BMS state */
};
/* example-can-tx-messages-configuration-end */

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
/* example-can-tx-messages-callback-definition-start */
extern uint32_t CANTX_BmsState(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* Do message handling stuff */
    return 0;
}
/* example-can-tx-messages-callback-definition-end */

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
