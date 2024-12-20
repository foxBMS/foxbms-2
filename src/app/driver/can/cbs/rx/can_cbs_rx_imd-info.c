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
 * @file    can_cbs_rx_imd-info.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for IMD messages
 */

/*========== Includes =======================================================*/
#include "bender_iso165c_cfg.h"

/* AXIVION Next Codeline Generic-LocalInclude: 'can_cbs_rx.h' declares the
 * prototype for the callback 'CANRX_ImdInfo' */
#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* Can message start bit and length defines */
#define CANRX_IMD_INFO_INSULATION_MEASUREMENT_START_BIT             (0u)
#define CANRX_IMD_INFO_INSULATION_MEASUREMENT_LENGTH                (16u)
#define CANRX_IMD_INFO_IMC_INSULATION_FAULT_START_BIT               (16u)
#define CANRX_IMD_INFO_IMC_INSULATION_FAULT_LENGTH                  (CAN_BIT)
#define CANRX_IMD_INFO_IMC_CHASSIS_FAULT_START_BIT                  (17u)
#define CANRX_IMD_INFO_IMC_CHASSIS_FAULT_LENGTH                     (CAN_BIT)
#define CANRX_IMD_INFO_IMC_SYSTEM_FAILURE_START_BIT                 (18u)
#define CANRX_IMD_INFO_IMC_SYSTEM_FAILURE_LENGTH                    (CAN_BIT)
#define CANRX_IMD_INFO_IMC_CALIBRATION_RUNNING_START_BIT            (19u)
#define CANRX_IMD_INFO_IMC_CALIBRATION_RUNNING_LENGTH               (CAN_BIT)
#define CANRX_IMD_INFO_IMC_SELF_TEST_RUNNING_START_BIT              (20u)
#define CANRX_IMD_INFO_IMC_SELF_TEST_RUNNING_LENGTH                 (CAN_BIT)
#define CANRX_IMD_INFO_IMC_INSULATION_WARNING_START_BIT             (21u)
#define CANRX_IMD_INFO_IMC_INSULATION_WARNING_LENGTH                (CAN_BIT)
#define CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_START_BIT (32u)
#define CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_LENGTH    (CAN_BIT)
#define CANRX_IMD_INFO_VIFC_IMC_CONNECTIVITY_START_BIT              (33u)
#define CANRX_IMD_INFO_VIFC_IMC_CONNECTIVITY_LENGTH                 (CAN_BIT)
#define CANRX_IMD_INFO_VIFC_IMC_ALIVE_STATUS_START_BIT              (34u)
#define CANRX_IMD_INFO_VIFC_IMC_ALIVE_STATUS_LENGTH                 (CAN_BIT)
#define CANRX_IMD_INFO_VIFC_COMMAND_START_BIT                       (35u)
#define CANRX_IMD_INFO_VIFC_COMMAND_LENGTH                          (CAN_BIT)
#define CANRX_IMD_INFO_VIFC_RESISTANCE_VALUE_OUTDATED_START_BIT     (40u)
#define CANRX_IMD_INFO_VIFC_RESISTANCE_VALUE_OUTDATED_LENGTH        (CAN_BIT)
#define CANRX_IMD_INFO_VIFC_OVERALL_SELF_TEST_START_BIT             (44u)
#define CANRX_IMD_INFO_VIFC_OVERALL_SELF_TEST_LENGTH                (CAN_BIT)
#define CANRX_IMD_INFO_VIFC_PARAMETER_CONFIG_SELF_TEST_START_BIT    (45u)
#define CANRX_IMD_INFO_VIFC_PARAMETER_CONFIG_SELF_TEST_LENGTH       (CAN_BIT)

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief Reads the information from the can data and transfers it to a can buffer element
 * @param messageDlc Dlc size of the message
 * @param kpkCanData Pointer to the data of the received message
 * @param canBuffer Pointer to the can buffer element where information is stored
 */
static void CANRX_TransferImdInfoMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer);

/**
 * @brief Reads measured resistance from imd info and writes it to the data table
 * @param messageData Data from the IMD info message
 * @param pTableInsulationMonitoring Pointer to the data table where information will be written
 */
static void CANRX_GetMeasuredResistance(
    uint64_t messageData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief Reads imc status from imd info and sets flags of the data table
 * @param messageData Data from the IMD info message
 * @param pTableInsulationMonitoring Pointer to the data table where information will be written
 */
static void CANRX_GetImcStatus(uint64_t messageData, DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief Reads vifc status from imd info and sets flags of the data table
 * @param messageData Data from the IMD info message
 * @param pTableInsulationMonitoring Pointer to the data table where information will be written
 */
static void CANRX_GetVifcStatus(uint64_t messageData, DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief Checks if self test is running
 * @param messageData Data from the IMD info message
 * @return True if self test is running
 */
static bool CANRX_SelfTestRunning(uint64_t messageData);

/**
 * @brief Checks if insulation measurement is active
 * @param messageData Data from the IMD info message
 * @return True if insulation measurement is active
 */
static bool CANRX_InsulationMeasurementActive(uint64_t messageData);

/**
 * @brief Checks if a self test has been executed
 * @param messageData Data from the IMD info message
 * @return True if a self test has been executed
 */
static bool CANRX_SelfTestExecuted(uint64_t messageData);

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/
static void CANRX_TransferImdInfoMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer) {
    FAS_ASSERT(0u < messageDlc);
    FAS_ASSERT(messageDlc <= CAN_MAX_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(canBuffer != NULL_PTR);

    /* determine dlc size of the message */
    const uint8_t boundedDlc = MATH_MinimumOfTwoUint8_t(messageDlc, CAN_MAX_DLC);

    /* transfer can data to buffer element*/
    for (uint8_t i = 0; i < boundedDlc; i++) {
        canBuffer->data[i] = kpkCanData[i];
    }
}

static void CANRX_GetMeasuredResistance(
    uint64_t messageData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    uint64_t signalData = 0u;
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_INSULATION_MEASUREMENT_START_BIT,
        CANRX_IMD_INFO_INSULATION_MEASUREMENT_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    pTableInsulationMonitoring->insulationResistance_kOhm = (uint32_t)signalData;
}

static void CANRX_GetImcStatus(uint64_t messageData, DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    uint64_t signalData = 0u;

    /* Insulation fault */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_IMC_INSULATION_FAULT_START_BIT,
        CANRX_IMD_INFO_IMC_INSULATION_FAULT_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        pTableInsulationMonitoring->dfIsCriticalResistanceDetected = false;
    } else {
        pTableInsulationMonitoring->dfIsCriticalResistanceDetected = true;
    }

    /* Chassis fault */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_IMC_CHASSIS_FAULT_START_BIT,
        CANRX_IMD_INFO_IMC_CHASSIS_FAULT_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        pTableInsulationMonitoring->dfIsChassisFaultDetected = false;
    } else {
        pTableInsulationMonitoring->dfIsChassisFaultDetected = true;
    }

    /* System failure */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_IMC_SYSTEM_FAILURE_START_BIT,
        CANRX_IMD_INFO_IMC_SYSTEM_FAILURE_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        pTableInsulationMonitoring->dfIsDeviceErrorDetected = false;
        pTableInsulationMonitoring->areDeviceFlagsValid     = true;
    } else {
        pTableInsulationMonitoring->dfIsDeviceErrorDetected = true;
        pTableInsulationMonitoring->areDeviceFlagsValid     = false;
    }

    /* Insulation warning */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_IMC_INSULATION_WARNING_START_BIT,
        CANRX_IMD_INFO_IMC_INSULATION_WARNING_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        pTableInsulationMonitoring->dfIsWarnableResistanceDetected = false;
    } else {
        pTableInsulationMonitoring->dfIsWarnableResistanceDetected = true;
    }
}

static void CANRX_GetVifcStatus(
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    uint64_t messageData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);
    uint64_t signalData = 0u;

    /* Insulation measurement status */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_START_BIT,
        CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        pTableInsulationMonitoring->isImdRunning = true;
    } else {
        pTableInsulationMonitoring->isImdRunning = false;
    }

    /* Insulation resistance value outdated */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_VIFC_RESISTANCE_VALUE_OUTDATED_START_BIT,
        CANRX_IMD_INFO_VIFC_RESISTANCE_VALUE_OUTDATED_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        pTableInsulationMonitoring->dfIsMeasurementUpToDate = true;
    } else {
        pTableInsulationMonitoring->dfIsMeasurementUpToDate = false;
    }
}

static bool CANRX_SelfTestRunning(uint64_t messageData) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    uint64_t signalData  = 0u;
    bool selfTestRunning = false;

    /* Check if self test is running */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_IMC_SELF_TEST_RUNNING_START_BIT,
        CANRX_IMD_INFO_IMC_SELF_TEST_RUNNING_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 1u) {
        selfTestRunning = true;
    }

    return selfTestRunning;
}

static bool CANRX_InsulationMeasurementActive(uint64_t messageData) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    uint64_t signalData              = 0u;
    bool insulationMeasurementActive = false;

    /* Check if insulation measurement is active*/
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_START_BIT,
        CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        insulationMeasurementActive = true;
    }
    return insulationMeasurementActive;
}

static bool CANRX_SelfTestExecuted(uint64_t messageData) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    uint64_t signalData   = 0u;
    bool selfTestExecuted = false;

#ifdef I165C_SELF_TEST_LONG
    /* Check if overall self test has been executed */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_VIFC_OVERALL_SELF_TEST_START_BIT,
        CANRX_IMD_INFO_VIFC_OVERALL_SELF_TEST_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        selfTestExecuted = true;
    }
#else
    /* Check if parameter self test has been executed */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_VIFC_PARAMETER_CONFIG_SELF_TEST_START_BIT,
        CANRX_IMD_INFO_VIFC_PARAMETER_CONFIG_SELF_TEST_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    if (signalData == 0u) {
        selfTestExecuted = true;
    }
#endif

    return selfTestExecuted;
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_ImdInfo(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    /* This handler is only implemented for little endian */
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(message.id == CANRX_IMD_INFO_ID);
    FAS_ASSERT(message.idType == CANRX_IMD_INFO_ID_TYPE);
    FAS_ASSERT(message.dlc <= CAN_MAX_DLC); /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    /* set up can buffer element */
    CAN_BUFFER_ELEMENT_s canBuffer = {
        .canNode = I165C_CAN_NODE, .id = message.id, .idType = message.idType, .data = {0u}};
    uint32_t retVal = 1u;

    /* copy message data to buffer element */
    CANRX_TransferImdInfoMessageToCanBuffer(message.dlc, kpkCanData, &canBuffer);

    /*send buffer element to the back of queue*/
    if (OS_SendToBackOfQueue(*(kpkCanShim->pQueueImd), (void *)&canBuffer, 0u) == OS_SUCCESS) {
        retVal = 0u;
    }
    return retVal;
}

extern void CANRX_ImdInfoGetDataFromMessage(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring) {
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(pTableInsulationMonitoring != NULL_PTR);

    uint64_t messageData = 0u;

    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, CAN_LITTLE_ENDIAN);

    CANRX_GetMeasuredResistance(messageData, pTableInsulationMonitoring);

    CANRX_GetImcStatus(messageData, pTableInsulationMonitoring);

    CANRX_GetVifcStatus(messageData, pTableInsulationMonitoring);
}

extern bool CANRX_ImdInfoCheckMeasurementMode(const uint8_t *const kpkCanData, uint8_t mode) {
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT((mode == I165C_ENABLE_MEASUREMENT) || (mode == I165C_DISABLE_MEASUREMENT));

    uint64_t messageData = 0u;
    uint64_t signalData  = 0u;
    bool returnValue     = false;

    /* get message data from can data */
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, CAN_LITTLE_ENDIAN);
    /* get current measurement mode from message data */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_START_BIT,
        CANRX_IMD_INFO_VIFC_INSULATION_MEASUREMENT_STATUS_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);

    /* Compare input mode with actual mode from IMD */
    uint8_t actualMeasurementMode = (uint8_t)signalData;
    if (mode == actualMeasurementMode) {
        returnValue = true;
    }
    return returnValue;
}

extern bool CANRX_ImdInfoHasSelfTestBeenExecuted(const uint8_t *const kpkCanData) {
    FAS_ASSERT(kpkCanData != NULL_PTR);

    uint64_t messageData     = 0u;
    uint64_t signalData      = 0u;
    bool anySelfTestExecuted = false;

    /* get message data from can data */
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, CAN_LITTLE_ENDIAN);
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_IMD_INFO_VIFC_OVERALL_SELF_TEST_START_BIT,
        CANRX_IMD_INFO_VIFC_OVERALL_SELF_TEST_LENGTH,
        &signalData,
        CAN_LITTLE_ENDIAN);
    /* test if overall self test has been executed */
    if (signalData == 0u) {
        anySelfTestExecuted = true;
    } else {
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            CANRX_IMD_INFO_VIFC_PARAMETER_CONFIG_SELF_TEST_START_BIT,
            CANRX_IMD_INFO_VIFC_PARAMETER_CONFIG_SELF_TEST_LENGTH,
            &signalData,
            CAN_LITTLE_ENDIAN);
        /* test if parameter config self test has been executed, only in case overall self test has not been executed */
        if (signalData == 0u) {
            anySelfTestExecuted = true;
        }
    }
    return anySelfTestExecuted;
}

extern bool CANRX_ImdInfoIsSelfTestFinished(const uint8_t *const kpkCanData) {
    FAS_ASSERT(kpkCanData != NULL_PTR);

    uint64_t messageData  = 0u;
    bool selfTestFinished = true;

    /* Get message data from can data */
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, CAN_LITTLE_ENDIAN);

    if (CANRX_SelfTestRunning(messageData)) {
        selfTestFinished = false;
    } else {
        if (CANRX_InsulationMeasurementActive(messageData)) {
            selfTestFinished = false;
        } else {
            selfTestFinished = CANRX_SelfTestExecuted(messageData);
        }
    }

    return selfTestFinished;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANRX_TransferImdInfoMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer) {
    CANRX_TransferImdInfoMessageToCanBuffer(messageDlc, kpkCanData, canBuffer);
}
#endif
