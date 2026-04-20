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
 * @file    can_cbs_rx_cs_lem-cab500.c
 * @author  foxBMS Team
 * @date    2025-04-28 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for lem current sensor measurements
 */

/*========== Includes =======================================================*/
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_rx.h' declares the
 * prototype for the callback 'CANRX_CsIsabellenhuetteIvtS' */
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_START_BIT (32u)
#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_LENGTH    (CAN_BIT)

#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_START_BIT (39u)
#define CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_LENGTH    (7u)

#define CANRX_CS_LEM_CAB500_STRING0_CURRENT_START_BIT (7u)
#define CANRX_CS_LEM_CAB500_STRING0_CURRENT_LENGTH    (32u)

#define CANRX_LEM_ZERO_AMPERE_VALUE (0x80000000)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Sets the error states in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   messageId    message ID to be handled
 * @param   stringNumber addressed string
 */
static void CANRX_LemHandleChannelError(const CAN_SHIM_s *const kpkCanShim, uint32_t messageId, uint8_t stringNumber);

/**
 * @brief   Resets all error states in the CAN data shim to valid
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 */
static void CANRX_LemResetError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber);

/**
 * @brief   Sets current value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   messageId    message ID to be handled
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_LemHandleSensorData(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber,
    int32_t signalData);

/**
 * @brief   Sets current value in the CAN data shim
 * @param   kpkCanShim   shim to the database entries
 * @param   stringNumber addressed string
 * @param   signalData   actual signal data
 */
static void CANRX_LemSetCurrent(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);

/**
 * @brief   Sets the string number based on the associated CAN message ID
 * @details See the documentation of the CAN module FAQ when implementing a
 *          multi-string application
 *          <a
 * href="../../../../software/modules/driver/can/can_how-to.html#multi-string-support-when-using-isabellenhuette-ivt-current-sensors"
 * target="_blank"> Multi-string Support when using Isabellenhuette IVT Current Sensors
 *          </a>.
 * @param   messageId message ID to be handled
 * @return  string number associated with the CAN message ID
 */
static uint8_t CANRX_LemSetStringNumberBasedOnCanMessageId(uint32_t messageId);

/*========== Static Function Implementations ================================*/
static void CANRX_LemHandleChannelError(const CAN_SHIM_s *const kpkCanShim, uint32_t messageId, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(messageId == CANRX_CS_LEM_CAB500_STRING0_ID);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    switch (messageId) {
        case CANRX_CS_LEM_CAB500_STRING0_ID: /* Current status */
            kpkCanShim->pTableCurrent->invalidMeasurement[stringNumber] = 1;
            /* TODO identify error*/
            break;
        default:                    /* LCOV_EXCL_LINE */
            /* No error detected */ /* LCOV_EXCL_LINE */
            break;                  /* LCOV_EXCL_LINE */
    }
}

static void CANRX_LemResetError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    kpkCanShim->pTableCurrent->invalidMeasurement[stringNumber] = 0;
}

static void CANRX_LemHandleSensorData(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber,
    int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(messageId == CANRX_CS_LEM_CAB500_STRING0_ID);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */

    switch (messageId) {
        /* Current measurement */
        case CANRX_CS_LEM_CAB500_STRING0_ID:
            CANRX_LemSetCurrent(kpkCanShim, stringNumber, signalData);
            break;
        default:                  /* LCOV_EXCL_LINE */
            FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
            break;                /* LCOV_EXCL_LINE */
    }
}

static void CANRX_LemSetCurrent(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    /* AXIVION Routine Generic-MissingParameterAssert: signalData: parameter accepts whole range */

    int32_t current_mA = signalData - CANRX_LEM_ZERO_AMPERE_VALUE;

    kpkCanShim->pTableCurrent->current_mA[stringNumber] = current_mA;
    kpkCanShim->pTableCurrent->newCurrent++;
    kpkCanShim->pTableCurrent->previousTimestamp[stringNumber] = kpkCanShim->pTableCurrent->timestamp[stringNumber];
    kpkCanShim->pTableCurrent->timestamp[stringNumber]         = OS_GetTickCount();
}

static uint8_t CANRX_LemSetStringNumberBasedOnCanMessageId(uint32_t messageId) {
    FAS_ASSERT(messageId == CANRX_CS_LEM_CAB500_STRING0_ID);
    uint8_t stringNumber = 0u;
    return stringNumber;
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_CsLemCab500(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANRX_CS_LEM_CAB500_STRING0_ID);
    FAS_ASSERT(message.idType == CANRX_CS_LEM_CAB500_STRING0_MESSAGES_ID_TYPE);
    FAS_ASSERT(message.dlc == CANRX_CS_LEM_CAB500_STRING0_MESSAGES_DLC);
    FAS_ASSERT(message.endianness == CANRX_CS_LEM_CAB500_STRING0_ENDIANNESS);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    uint64_t messageData = 0uLL;
    uint64_t canSignal   = 0uLL;

    int32_t signalData   = 0;
    uint8_t stringNumber = CANRX_LemSetStringNumberBasedOnCanMessageId(message.id);

    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

    /* Handle error flags */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_START_BIT,
        CANRX_CS_LEM_CAB500_STRING0_ERROR_INDICATION_LENGTH,
        &canSignal,
        message.endianness);
    if (canSignal != 0uLL) {
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_START_BIT,
            CANRX_CS_LEM_CAB500_STRING0_ERROR_INFORMATION_LENGTH,
            &canSignal,
            message.endianness);
        CANRX_LemHandleChannelError(kpkCanShim, message.id, stringNumber);
    } else {
        CANRX_LemResetError(kpkCanShim, stringNumber);
    }

    /* Get data from sensor reading */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_CS_LEM_CAB500_STRING0_CURRENT_START_BIT,
        CANRX_CS_LEM_CAB500_STRING0_CURRENT_LENGTH,
        &canSignal,
        message.endianness);
    signalData = (int32_t)canSignal;
    CANRX_LemHandleSensorData(kpkCanShim, message.id, stringNumber, signalData);

    DATA_WRITE_DATA(kpkCanShim->pTableCurrent);
    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_LemHandleSensorData(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber,
    int32_t signalData) {
    CANRX_LemHandleSensorData(kpkCanShim, messageId, stringNumber, signalData);
}
extern void TEST_CANRX_LemSetCurrent(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData) {
    CANRX_LemSetCurrent(kpkCanShim, stringNumber, signalData);
}
extern uint8_t TEST_CANRX_LemSetStringNumberBasedOnCanMessageId(uint32_t messageId) {
    return CANRX_LemSetStringNumberBasedOnCanMessageId(messageId);
}
extern void TEST_CANRX_LemResetError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber) {
    CANRX_LemResetError(kpkCanShim, stringNumber);
}
extern void TEST_CANRX_LemHandleChannelError(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber) {
    CANRX_LemHandleChannelError(kpkCanShim, messageId, stringNumber);
}
#endif
