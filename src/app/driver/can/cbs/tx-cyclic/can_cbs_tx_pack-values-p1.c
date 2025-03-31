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
 * @file    can_cbs_tx_pack-values-p1.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for pack value and string value messages
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** @{
 * defines of the insulation resistance signal
*/
#define CANTX_PACK_P1_INSULATION_RESISTANCE_START_BIT    (7u)
#define CANTX_PACK_P1_INSULATION_RESISTANCE_LENGTH       (13u)
#define CANTX_MINIMUM_VALUE_INSULATION_RESISTANCE_SIGNAL (0.0f)
#define CANTX_MAXIMUM_VALUE_INSULATION_RESISTANCE_SIGNAL (57337.0f)
#define CANTX_FACTOR_INSULATION_RESISTANCE               (7.0f)
/** @} */

/** @{
 * configuration of the insulation resistance signal
*/
static const CAN_SIGNAL_TYPE_s cantx_insulationResistance = {
    CANTX_PACK_P1_INSULATION_RESISTANCE_START_BIT,
    CANTX_PACK_P1_INSULATION_RESISTANCE_LENGTH,
    CANTX_FACTOR_INSULATION_RESISTANCE,
    CAN_SIGNAL_OFFSET_0,
    CANTX_MINIMUM_VALUE_INSULATION_RESISTANCE_SIGNAL,
    CANTX_MAXIMUM_VALUE_INSULATION_RESISTANCE_SIGNAL};
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief  reads the insulation resistance from the database and calculates the return values
 * @param  kpkCanShim can shim with database information
 * @return insulation resistance value
 */
static uint64_t CANTX_GetInsulationResistance(const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief sets the message data with signal data of insulation resistance
 * @param pMessageData message data of the CAN message
 * @param kpkCanShim can shim with database information
 */
static void CANTX_SetInsulationResistance(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_GetInsulationResistance(const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* insulation resistance */
    float_t signalData = (float_t)kpkCanShim->pTableInsulation->insulationResistance_kOhm;
    CAN_TxPrepareSignalData(&signalData, cantx_insulationResistance);
    uint64_t data = (uint64_t)signalData;
    return data;
}

static void CANTX_SetInsulationResistance(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    uint64_t signalData = CANTX_GetInsulationResistance(kpkCanShim);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        cantx_insulationResistance.bitStart,
        cantx_insulationResistance.bitLength,
        signalData,
        CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_PackValuesP1(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);
    FAS_ASSERT(message.id == CANTX_PACK_VALUES_P1_ID);
    FAS_ASSERT(message.idType == CANTX_PACK_VALUES_P1_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CANTX_PACK_VALUES_P1_ENDIANNESS);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* read database entry */
    DATA_READ_DATA(kpkCanShim->pTableInsulation);

    /* build message from data */
    CANTX_SetInsulationResistance(&messageData, kpkCanShim);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, CAN_BIG_ENDIAN);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANTX_SetInsulationResistance(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    CANTX_SetInsulationResistance(pMessageData, kpkCanShim);
}

extern uint64_t TEST_CANTX_GetInsulationResistance(const CAN_SHIM_s *const kpkCanShim) {
    return CANTX_GetInsulationResistance(kpkCanShim);
}

#endif
