/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_string-values-p1.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
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

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/* configuration of the StringP1 message bits */
#define CANTX_STRING_P1_STRING_MUX_START_BIT     (7u)
#define CANTX_STRING_P1_STRING_MUX_LENGTH        (4u)
#define CANTX_STRING_P1_ENERGY_COUNTER_START_BIT (15u)
#define CANTX_STRING_P1_ENERGY_COUNTER_LENGTH    (32u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief sets multiplexer of StringP1 message to string number
 */
static void CANTX_SetStringP1Mux(uint64_t *pMessageData, uint64_t signalData);

/**
 * @brief sets energy counter value in StringP1 message
 */
static void CANTX_SetStringEnergyCounter(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/
static void CANTX_SetStringP1Mux(uint64_t *pMessageData, uint64_t signalData) {
    FAS_ASSERT(signalData < BS_NR_OF_STRINGS);
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_STRING_P1_STRING_MUX_START_BIT,
        CANTX_STRING_P1_STRING_MUX_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetStringEnergyCounter(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* String voltage */
    OS_EnterTaskCritical(); /* this access has to be protected as it conflicts with the 1ms task */
    uint64_t signalData = (uint64_t)kpkCanShim->pTableCurrentSensor->energyCounter_Wh[stringNumber];
    OS_ExitTaskCritical();

    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_STRING_P1_ENERGY_COUNTER_START_BIT,
        CANTX_STRING_P1_ENERGY_COUNTER_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_StringValuesP1(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_VALUES_P1_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_VALUES_P1_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;
    uint64_t signalData  = 0u;

    const uint8_t stringNumber = *pMuxId;

    /* First signal to transmit cell voltages: get database values */
    if (stringNumber == 0u) {
        /* Do not read pTableMsl and pTableErrorState as they already are read
         * with a higher frequency from CANTX_BmsState callback */
        DATA_READ_DATA(kpkCanShim->pTableCurrentSensor);
    }

    signalData = (uint64_t)stringNumber;
    CANTX_SetStringP1Mux(&messageData, signalData);
    CANTX_SetStringEnergyCounter(&messageData, stringNumber, kpkCanShim);

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* Check mux value */
    if (*pMuxId >= BS_NR_OF_STRINGS) {
        *pMuxId = 0u;
    }

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANTX_SetStringP1Mux(uint64_t *pMessageData, uint64_t signalData) {
    CANTX_SetStringP1Mux(pMessageData, signalData);
}
extern void TEST_CANTX_SetStringEnergyCounter(
    uint64_t *pMessageData,
    uint8_t stringNumber,
    const CAN_SHIM_s *const kpkCanShim) {
    CANTX_SetStringEnergyCounter(pMessageData, stringNumber, kpkCanShim);
}
#endif
