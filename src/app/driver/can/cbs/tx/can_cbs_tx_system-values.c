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
 * @file    can_cbs_tx_system-values.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for pack value and string value messages
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_PackValues(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(pMuxId == NULL_PTR);

    FAS_ASSERT(message.id == CANTX_PACK_VALUES_ID);
    FAS_ASSERT(message.idType == CANTX_PACK_VALUES_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    /* Read database entry */
    DATA_READ_DATA(kpkCanShim->pTablePackValues);

    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    /* Battery voltage */
    float_t signalData = kpkCanShim->pTablePackValues->batteryVoltage_mV;
    float_t offset     = 0.0f;
    float_t factor     = 0.01f; /* convert mV to 100mV */
    signalData         = (signalData + offset) * factor;
    uint64_t data      = (uint64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 14u, data, message.endianness);

    /* System voltage */
    signalData = kpkCanShim->pTablePackValues->highVoltageBusVoltage_mV;
    offset     = 0.0f;
    factor     = 0.01f; /* convert mV to 100mV */
    signalData = (signalData + offset) * factor;
    data       = (uint64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 9u, 14u, data, message.endianness);

    /* System current */
    signalData = kpkCanShim->pTablePackValues->packCurrent_mA;
    offset     = 0.0f;
    factor     = 0.1f; /* convert mA to 10mA */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 41u, 18u, data, message.endianness);

    /* System power */
    signalData = kpkCanShim->pTablePackValues->packPower_W;
    offset     = 0.0f;
    factor     = 0.1f; /* convert W to 10W */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 27u, 18u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0;
}

extern uint32_t CANTX_StringValuesP0(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_STRING_VALUES_P0_ID);
    FAS_ASSERT(message.idType == CANTX_STRING_VALUES_P0_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(*pMuxId < BS_NR_OF_STRINGS);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;
    uint64_t data        = 0;
    float_t signalData   = 0.0f;
    float_t offset       = 0.0f;
    float_t factor       = 0.0f;

    const uint8_t stringNumber = *pMuxId;

    /* First signal to transmit cell voltages: get database values */
    if (stringNumber == 0u) {
        /* Do not read pTableMsl and pTableErrorState as they already are read
         * with a higher frequency from CANTX_BmsState callback */
        DATA_READ_DATA(kpkCanShim->pTablePackValues);
    }

    /* mux value */
    data = (uint64_t)stringNumber;
    /* set data in CAN frame */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 3u, data, message.endianness);

    /* String voltage */
    signalData = (float_t)kpkCanShim->pTablePackValues->stringVoltage_mV[stringNumber];
    offset     = 0.0f;
    factor     = 0.1f; /* convert mV to 10mV */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 4u, 17u, data, message.endianness);

    /* String current */
    signalData = (float_t)kpkCanShim->pTablePackValues->stringCurrent_mA[stringNumber];
    offset     = 0.0f;
    factor     = 0.1f; /* convert mA to 10mA */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 19u, 18u, data, message.endianness);

    /* String power */
    signalData = (float_t)kpkCanShim->pTablePackValues->stringPower_W[stringNumber];
    offset     = 0.0f;
    factor     = 0.1f; /* convert W to 10W */
    signalData = (signalData + offset) * factor;
    data       = (int64_t)signalData;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 33u, 18u, data, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* Check mux value */
    if (*pMuxId >= BS_NR_OF_STRINGS) {
        *pMuxId = 0u;
    }

    return 0;
}

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

    /* mux value */
    signalData = (uint64_t)stringNumber;
    /* set data in CAN frame */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Signal data defined in .dbc file. */
    CAN_TxSetMessageDataWithSignalData(&messageData, 7u, 4u, signalData, message.endianness);

    /* String voltage */
    OS_EnterTaskCritical(); /* this access has to be protected as it conflicts with the 1ms task */
    signalData = (int64_t)kpkCanShim->pTableCurrentSensor->energyCounter_Wh[stringNumber];
    OS_ExitTaskCritical();
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&messageData, 15u, 32u, signalData, message.endianness);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    /* now copy data in the buffer that will be used to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* Check mux value */
    if (*pMuxId >= BS_NR_OF_STRINGS) {
        *pMuxId = 0u;
    }

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
