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
 * @file    can_cbs_tx_bms-state-details.c
 * @author  foxBMS Team
 * @date    2021-07-21 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for state messages
 */

/*========== Includes =======================================================*/
#include "bms.h"
/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_tx_cyclic.h' declares
 * the prototype for the callback 'CANTX_BmsStateDetails' */
#include "can_cbs_tx_cyclic.h"
#include "can_cfg_tx-cyclic-message-definitions.h"
#include "can_helper.h"
#include "sys_mon.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * Configuration of the signals
 */
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_START_BIT          (4u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_LENGTH             (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_START_BIT               (3u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_LENGTH                  (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_START_BIT                (2u)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_LENGTH                   (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_START_BIT                 (1u)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_LENGTH                    (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_START_BIT              (0u)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_LENGTH                 (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_RECORDED_START_BIT (12u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_RECORDED_LENGTH    (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_RECORDED_START_BIT      (11u)
#define CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_RECORDED_LENGTH         (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_RECORDED_START_BIT       (10u)
#define CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_RECORDED_LENGTH          (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_RECORDED_START_BIT        (9u)
#define CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_RECORDED_LENGTH           (CAN_BIT)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_RECORDED_START_BIT     (8u)
#define CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_RECORDED_LENGTH        (CAN_BIT)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief sets the message data for current violation engine
 * @param   pMessageData message data of the CAN message
 * @param   kpkCanShim  const pointer to CAN shim
 */
static void CANTX_SetTimingViolationEngine(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief sets the message data for current violation 1ms
 * @param   pMessageData message data of the CAN message
 * @param   kpkCanShim  const pointer to CAN shim
 */
static void CANTX_SetTimingViolation1ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief sets the message data for current violation 10ms
 * @param   pMessageData message data of the CAN message
 * @param   kpkCanShim  const pointer to CAN shim
 */
static void CANTX_SetTimingViolation10Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief sets the message data for current violation 100ms
 * @param   pMessageData message data of the CAN message
 * @param   kpkCanShim  const pointer to CAN shim
 */
static void CANTX_SetTimingViolation100Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief sets the message data for current violation 100ms algorithm
 * @param   pMessageData message data of the CAN message
 * @param   kpkCanShim  const pointer to CAN shim
 */
static void CANTX_SetTimingViolation100MsAlgo(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief sets the message data for recorded violation engine
 * @param   pMessageData              message data of the CAN message
 * @param   pRecordedTimingViolations set  of all task timing violations
 */
static void CANTX_SetTimingViolationEngineRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);

/**
 * @brief sets the message data for recorded violation 1ms
 * @param   pMessageData              message data of the CAN message
 * @param   pRecordedTimingViolations set  of all task timing violations
 */
static void CANTX_SetTimingViolation1MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);

/**
 * @brief sets the message data for recorded violation 10ms
 * @param   pMessageData              message data of the CAN message
 * @param   pRecordedTimingViolations set  of all task timing violations
 */
static void CANTX_SetTimingViolation10MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);

/**
 * @brief sets the message data for recorded violation 100ms
 * @param   pMessageData              message data of the CAN message
 * @param   pRecordedTimingViolations set  of all task timing violations
 */
static void CANTX_SetTimingViolation100MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);

/**
 * @brief sets the message data for recorded violation 100ms algorithm
 * @param   pMessageData              message data of the CAN message
 * @param   pRecordedTimingViolations set  of all task timing violations
 */
static void CANTX_SetTimingViolation100MsAlgoRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations);

/*========== Static Function Implementations ================================*/
static void CANTX_SetTimingViolationEngine(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* current violation engine */
    uint64_t data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->taskEngineTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation1ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* current violation 1ms */
    uint64_t data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task1msTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation10Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* current violation 10ms */
    uint64_t data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task10msTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation100Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* current violation 100ms */
    uint64_t data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task100msTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation100MsAlgo(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    /* current violation 100ms algorithm */
    uint64_t data = CAN_ConvertBooleanToInteger(kpkCanShim->pTableErrorState->task100msAlgoTimingViolationError);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolationEngineRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(pRecordedTimingViolations != NULL_PTR);

    /* recorded violation engine */
    uint64_t data = CAN_ConvertBooleanToInteger(pRecordedTimingViolations->recordedViolationEngine);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_RECORDED_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_ENGINE_TASK_RECORDED_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation1MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(pRecordedTimingViolations != NULL_PTR);

    /* recorded violation 1ms */
    uint64_t data = CAN_ConvertBooleanToInteger(pRecordedTimingViolations->recordedViolation1ms);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_RECORDED_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_1MS_TASK_RECORDED_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation10MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(pRecordedTimingViolations != NULL_PTR);

    /* recorded violation 10ms */
    uint64_t data = CAN_ConvertBooleanToInteger(pRecordedTimingViolations->recordedViolation10ms);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_RECORDED_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_10MS_TASK_RECORDED_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation100MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(pRecordedTimingViolations != NULL_PTR);

    /* recorded violation 100ms */
    uint64_t data = CAN_ConvertBooleanToInteger(pRecordedTimingViolations->recordedViolation100ms);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_RECORDED_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_TASK_RECORDED_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

static void CANTX_SetTimingViolation100MsAlgoRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(pRecordedTimingViolations != NULL_PTR);

    /* recorded violation 100ms algorithm */
    uint64_t data = CAN_ConvertBooleanToInteger(pRecordedTimingViolations->recordedViolation100msAlgo);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_RECORDED_START_BIT,
        CANTX_SIGNAL_TIMING_VIOLATION_100MS_ALGO_TASK_RECORDED_LENGTH,
        data,
        CAN_BIG_ENDIAN);
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANTX_BmsStateDetails(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANTX_BMS_STATE_DETAILS_ID);
    FAS_ASSERT(message.idType == CANTX_BMS_STATE_DETAILS_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId == NULL_PTR); /* pMuxId is not used here, therefore has to be NULL_PTR */
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t messageData = 0u;

    DATA_READ_DATA(kpkCanShim->pTableErrorState);
    SYSM_TIMING_VIOLATION_RESPONSE_s recordedTimingViolations = {false, false, false, false, false, false};
    SYSM_GetRecordedTimingViolations(&recordedTimingViolations);

    /* set message with current violation data */
    CANTX_SetTimingViolationEngine(&messageData, kpkCanShim);
    CANTX_SetTimingViolation1ms(&messageData, kpkCanShim);
    CANTX_SetTimingViolation10Ms(&messageData, kpkCanShim);
    CANTX_SetTimingViolation100Ms(&messageData, kpkCanShim);
    CANTX_SetTimingViolation100MsAlgo(&messageData, kpkCanShim);

    /* set message with recorded violation data */
    CANTX_SetTimingViolationEngineRec(&messageData, &recordedTimingViolations);
    CANTX_SetTimingViolation1MsRec(&messageData, &recordedTimingViolations);
    CANTX_SetTimingViolation10MsRec(&messageData, &recordedTimingViolations);
    CANTX_SetTimingViolation100MsRec(&messageData, &recordedTimingViolations);
    CANTX_SetTimingViolation100MsAlgoRec(&messageData, &recordedTimingViolations);

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(messageData, pCanData, message.endianness);

    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANTX_SetTimingViolationEngine(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    CANTX_SetTimingViolationEngine(pMessageData, kpkCanShim);
}
extern void TEST_CANTX_SetTimingViolation1ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    CANTX_SetTimingViolation1ms(pMessageData, kpkCanShim);
}
extern void TEST_CANTX_SetTimingViolation10Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    CANTX_SetTimingViolation10Ms(pMessageData, kpkCanShim);
}
extern void TEST_CANTX_SetTimingViolation100Ms(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    CANTX_SetTimingViolation100Ms(pMessageData, kpkCanShim);
}
extern void TEST_CANTX_SetTimingViolation100MsAlgo(uint64_t *pMessageData, const CAN_SHIM_s *const kpkCanShim) {
    CANTX_SetTimingViolation100MsAlgo(pMessageData, kpkCanShim);
}
extern void TEST_CANTX_SetTimingViolationEngineRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    CANTX_SetTimingViolationEngineRec(pMessageData, pRecordedTimingViolations);
}
extern void TEST_CANTX_SetTimingViolation1MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    CANTX_SetTimingViolation1MsRec(pMessageData, pRecordedTimingViolations);
}
extern void TEST_CANTX_SetTimingViolation10MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    CANTX_SetTimingViolation10MsRec(pMessageData, pRecordedTimingViolations);
}
extern void TEST_CANTX_SetTimingViolation100MsRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    CANTX_SetTimingViolation100MsRec(pMessageData, pRecordedTimingViolations);
}
extern void TEST_CANTX_SetTimingViolation100MsAlgoRec(
    uint64_t *pMessageData,
    SYSM_TIMING_VIOLATION_RESPONSE_s *pRecordedTimingViolations) {
    CANTX_SetTimingViolation100MsAlgoRec(pMessageData, pRecordedTimingViolations);
}
#endif
