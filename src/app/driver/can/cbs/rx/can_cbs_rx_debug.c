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
 * @file    can_cbs_rx_debug.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVER
 * @prefix  CANRX
 *
 * @brief   CAN driver Rx callback implementation
 * @details CAN Rx callback for the debug message
 */

/*========== Includes =======================================================*/
#include "can_cbs_rx.h"
#include "can_cbs_tx_debug-response.h"
#include "can_cbs_tx_unsupported-message.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "fram.h"
#include "ftask.h"
#include "reset.h"
#include "rtc.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

#define CANRX_BIT (1u)

/** @{
 * multiplexer setup for the debug message
 */
#define CANRX_DEBUG_MESSAGE_MUX_START_BIT (0x7u)
#define CANRX_DEBUG_MESSAGE_MUX_LENGTH    (8u)
/** @} */

/** @{
 * supported multiplexer values
 */
#define CANRX_DEBUG_MESSAGE_MUX_VALUE_VERSION_INFORMATION (0x00u)
#define CANRX_DEBUG_MESSAGE_MUX_VALUE_RTC                 (0x01u)
#define CANRX_DEBUG_MESSAGE_MUX_VALUE_SOFTWARE_RESET      (0x02u)
#define CANRX_DEBUG_MESSAGE_MUX_VALUE_FRAM_INITIALIZATION (0x03u)
#define CANRX_DEBUG_MESSAGE_MUX_VALUE_TIME_INFO           (0x04u)
/** @} */

/** @{
 * configuration of the version information signals for multiplexer
 * 'VersionInformation' in the 'Debug message
 */
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_BMS_SOFTWARE_VERSION_START_BIT  (8u)
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_BMS_SOFTWARE_VERSION_LENGTH     (CANRX_BIT)
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_UNIQUE_DIE_ID_START_BIT     (9u)
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_UNIQUE_DIE_ID_LENGTH        (CANRX_BIT)
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_LOT_NUMBER_START_BIT        (10u)
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_LOT_NUMBER_LENGTH           (CANRX_BIT)
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_WAFER_INFORMATION_START_BIT (11u)
#define CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_WAFER_INFORMATION_LENGTH    (CANRX_BIT)
/** @} */

/** @{
 * configuration of the RTC signals for multiplexer 'RTC' in the 'Debug'
 * message
 */
#define CANRX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_START_BIT (15u)
#define CANRX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_LENGTH    (7u)
#define CANRX_MUX_RTC_SIGNAL_SECONDS_START_BIT              (8u)
#define CANRX_MUX_RTC_SIGNAL_SECONDS_LENGTH                 (6u)
#define CANRX_MUX_RTC_SIGNAL_MINUTES_START_BIT              (18u)
#define CANRX_MUX_RTC_SIGNAL_MINUTES_LENGTH                 (6u)
#define CANRX_MUX_RTC_SIGNAL_HOURS_START_BIT                (28u)
#define CANRX_MUX_RTC_SIGNAL_HOURS_LENGTH                   (5u)
#define CANRX_MUX_RTC_SIGNAL_WEEKDAY_START_BIT              (39u)
#define CANRX_MUX_RTC_SIGNAL_WEEKDAY_LENGTH                 (3u)
#define CANRX_MUX_RTC_SIGNAL_DAY_START_BIT                  (36u)
#define CANRX_MUX_RTC_SIGNAL_DAY_LENGTH                     (5u)
#define CANRX_MUX_RTC_SIGNAL_MONTH_START_BIT                (47u)
#define CANRX_MUX_RTC_SIGNAL_MONTH_LENGTH                   (4u)
#define CANRX_MUX_RTC_SIGNAL_YEAR_START_BIT                 (43u)
#define CANRX_MUX_RTC_SIGNAL_YEAR_LENGTH                    (7u)
/** @} */

/** @{
 * configuration of the software reset signals for multiplexer 'SoftwareReset'
 * in the 'Debug' message
 */
#define CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_SOFTWARE_RESET_START_BIT (39u)
#define CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_SOFTWARE_RESET_LENGTH    (CANRX_BIT)
/** @} */

/** @{
 * configuration of the FRAM initialization signals for multiplexer 'InitializeFram'
 * in the 'Debug' message
 */
#define CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_FRAM_INITIALIZATION_START_BIT (27u)
#define CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_FRAM_INITIALIZATION_LENGTH    (CANRX_BIT)
/** @} */

/** @{
 * configuration of the time info signals for multiplexer 'TimeInfo'
 * in the 'Debug' message
 */
#define CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_REQUEST_RTC_TIME_START_BIT (8u)
#define CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_REQUEST_RTC_TIME_LENGTH    (CANRX_BIT)
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Parses CAN message to handle version related messages
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 */
static void CANRX_ProcessVersionInformationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses CAN message to set the RTC clock
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 */
static void CANRX_ProcessRtcMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses CAN message to handle software related requests, e.g., a
 *          software reset of the BMS
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 */
static void CANRX_ProcessSoftwareResetMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses CAN message related to FRAM initialization
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 */
static void CANRX_ProcessFramInitializationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses CAN message to handle time information requests
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 */
static void CANRX_ProcessTimeInfoMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the hundredth of seconds
 *          information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns Decoded hundredth of seconds information
 */
static uint8_t CANRX_GetHundredthOfSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the seconds information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns Decoded seconds information
 */
static uint8_t CANRX_GetSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the minutes information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns Decoded minutes information
 */
static uint8_t CANRX_GetMinutes(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the hours information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns Decoded hours information
 */
static uint8_t CANRX_GetHours(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the weekday information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns Decoded weekday information
 */
static uint8_t CANRX_GetWeekday(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the day information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns Decoded Day information
 */
static uint8_t CANRX_GetDay(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the month information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
* @returns  Decoded month information
 */
static uint8_t CANRX_GetMonth(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Parses the CAN message to retrieve the year information
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
* @returns  Decoded year information
 */
static uint8_t CANRX_GetYear(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Check if the BMS software version information is requested
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns true if the information is requested, false otherwise
 */
static bool CANRX_CheckIfBmsSoftwareVersionIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Triggers sending of the BMS software version information message
 */
static void CANRX_TriggerBmsSoftwareVersionMessage(void);

/**
 * @brief   Check if the MCU die ID information is requested
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns true if the information is requested, false otherwise
 */
static bool CANRX_CheckIfMcuUniqueDieIdIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Triggers sending of the MCU die ID information message
 */
static void CANRX_TriggerMcuUniqueDieIdMessage(void);

/**
 * @brief   Check if the MCU lot number information is requested
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns true if the information is requested, false otherwise
 */
static bool CANRX_CheckIfMcuLotNumberIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Triggers sending of the MCU lot number information message
 */
static void CANRX_TriggerMcuLotNumberMessage(void);

/**
 * @brief   Check if the MCU wafer information is requested
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns true if the information is requested, false otherwise
 */
static bool CANRX_CheckIfMcuWaferInformationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Triggers sending of the MCU wafer information message
 */
static void CANRX_TriggerMcuWaferInformationMessage(void);

/**
 * @brief   Check if a software reset is requested
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns true if software reset is requested, false otherwise
 */
static bool CANRX_CheckIfSoftwareResetIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Check if a FRAM initialization is requested
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns true if software reset is requested, false otherwise
 */
static bool CANRX_CheckIfFramInitializationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Check if the RTC time information is requested
 * @param   messageData message data of the CAN message
 * @param   endianness  endianness of the message
 * @returns true if the information is requested, false otherwise
 */
static bool CANRX_CheckIfTimeInfoIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

/**
 * @brief   Triggers sending of the RTC time information message
 */
static void CANRX_TriggerTimeInfoMessage(void);

/*========== Static Function Implementations ================================*/

static uint8_t CANRX_GetHundredthOfSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    uint64_t signalData = 0u;
    /* Get hundredth of seconds information form the message and return that value */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_START_BIT,
        CANRX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_LENGTH,
        &signalData,
        endianness);
    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static uint8_t CANRX_GetSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    uint64_t signalData = 0u;
    /* Get seconds information form the message and return that value */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_RTC_SIGNAL_SECONDS_START_BIT,
        CANRX_MUX_RTC_SIGNAL_SECONDS_LENGTH,
        &signalData,
        endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static uint8_t CANRX_GetMinutes(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    uint64_t signalData = 0u;
    /* Get minutes information form the message and return that value */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_RTC_SIGNAL_MINUTES_START_BIT,
        CANRX_MUX_RTC_SIGNAL_MINUTES_LENGTH,
        &signalData,
        endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static uint8_t CANRX_GetHours(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    uint64_t signalData = 0u;
    /* Get hour information form the message and return that value */
    CAN_RxGetSignalDataFromMessageData(
        messageData, CANRX_MUX_RTC_SIGNAL_HOURS_START_BIT, CANRX_MUX_RTC_SIGNAL_HOURS_LENGTH, &signalData, endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static uint8_t CANRX_GetWeekday(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    uint64_t signalData = 0u;
    /* Get weekday information form the message and return that value */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_RTC_SIGNAL_WEEKDAY_START_BIT,
        CANRX_MUX_RTC_SIGNAL_WEEKDAY_LENGTH,
        &signalData,
        endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static uint8_t CANRX_GetDay(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    uint64_t signalData = 0u;
    /* Get day information form the message and return that value */
    CAN_RxGetSignalDataFromMessageData(
        messageData, CANRX_MUX_RTC_SIGNAL_DAY_START_BIT, CANRX_MUX_RTC_SIGNAL_DAY_LENGTH, &signalData, endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static uint8_t CANRX_GetMonth(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    /* Get month information form the message and return that value */
    uint64_t signalData = 0u;
    CAN_RxGetSignalDataFromMessageData(
        messageData, CANRX_MUX_RTC_SIGNAL_MONTH_START_BIT, CANRX_MUX_RTC_SIGNAL_MONTH_LENGTH, &signalData, endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static uint8_t CANRX_GetYear(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    /* Get year information form the message and return that value */
    uint64_t signalData = 0u;
    CAN_RxGetSignalDataFromMessageData(
        messageData, CANRX_MUX_RTC_SIGNAL_YEAR_START_BIT, CANRX_MUX_RTC_SIGNAL_YEAR_LENGTH, &signalData, endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1: only first byte is of interest */
    return (uint8_t)signalData;
}

static void CANRX_ProcessRtcMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    RTC_TIME_DATA_s time = {0};

    time.hundredthOfSeconds = CANRX_GetHundredthOfSeconds(messageData, endianness);
    time.seconds            = CANRX_GetSeconds(messageData, endianness);
    time.minutes            = CANRX_GetMinutes(messageData, endianness);
    time.hours              = CANRX_GetHours(messageData, endianness);
    time.weekday            = CANRX_GetWeekday(messageData, endianness);
    time.day                = CANRX_GetDay(messageData, endianness);
    time.month              = CANRX_GetMonth(messageData, endianness);
    time.year               = CANRX_GetYear(messageData, endianness);
    if (OS_SendToBackOfQueue(ftsk_rtcSetTimeQueue, (void *)&time, 0u) == OS_SUCCESS) {
        /* queue is not full */
    }
}

static bool CANRX_CheckIfBmsSoftwareVersionIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    bool isRequested    = false;
    uint64_t signalData = 0u;

    /* get BMS software version information bit from the CAN message */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_BMS_SOFTWARE_VERSION_START_BIT,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_BMS_SOFTWARE_VERSION_LENGTH,
        &signalData,
        endianness);
    if (signalData == 1u) {
        isRequested = true;
    }
    return isRequested;
}

static void CANRX_TriggerBmsSoftwareVersionMessage(void) {
    /* send the debug message containing the BMS software version information and trap if this does not work */
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
}

static bool CANRX_CheckIfMcuUniqueDieIdIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    bool isRequested    = false;
    uint64_t signalData = 0u;

    /* get MCU unique die id information bit from the CAN message */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_UNIQUE_DIE_ID_START_BIT,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_UNIQUE_DIE_ID_LENGTH,
        &signalData,
        endianness);
    if (signalData == 1u) {
        isRequested = true;
    }
    return isRequested;
}

static void CANRX_TriggerMcuUniqueDieIdMessage(void) {
    /* send the debug message containing the MCU unique die id information and trap if this does not work */
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
}

static bool CANRX_CheckIfMcuLotNumberIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    bool isRequested    = false;
    uint64_t signalData = 0u;

    /* get MCU lot number information bit from the CAN message */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_LOT_NUMBER_START_BIT,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_LOT_NUMBER_LENGTH,
        &signalData,
        endianness);
    if (signalData == 1u) {
        isRequested = true;
    }
    return isRequested;
}

static void CANRX_TriggerMcuLotNumberMessage(void) {
    /* send the debug message containing the MCU lot number information and trap if this does not work */
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
}

static bool CANRX_CheckIfMcuWaferInformationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    bool isRequested    = false;
    uint64_t signalData = 0u;

    /* get MCU wafer information bit from the CAN message */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_WAFER_INFORMATION_START_BIT,
        CANRX_MUX_VERSION_INFO_SIGNAL_GET_MCU_WAFER_INFORMATION_LENGTH,
        &signalData,
        endianness);
    if (signalData == 1u) {
        isRequested = true;
    }
    return isRequested;
}

static void CANRX_TriggerMcuWaferInformationMessage(void) {
    /* send the debug message containing the MCU wafer information and trap if this does not work */
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
}

static void CANRX_ProcessVersionInformationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    /* check if any of the version information is requested and if so transmit the information */
    if (CANRX_CheckIfBmsSoftwareVersionIsRequested(messageData, endianness) == true) {
        CANRX_TriggerBmsSoftwareVersionMessage();
    }
    if (CANRX_CheckIfMcuUniqueDieIdIsRequested(messageData, endianness) == true) {
        CANRX_TriggerMcuUniqueDieIdMessage();
    }
    if (CANRX_CheckIfMcuLotNumberIsRequested(messageData, endianness) == true) {
        CANRX_TriggerMcuLotNumberMessage();
    }
    if (CANRX_CheckIfMcuWaferInformationIsRequested(messageData, endianness) == true) {
        CANRX_TriggerMcuWaferInformationMessage();
    }
}

static bool CANRX_CheckIfSoftwareResetIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    bool isRequested    = false;
    uint64_t signalData = 0u;

    /* get software reset bit from the CAN message */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_SOFTWARE_RESET_START_BIT,
        CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_SOFTWARE_RESET_LENGTH,
        &signalData,
        endianness);
    if (signalData == 1u) {
        isRequested = true;
    }
    return isRequested;
}

static bool CANRX_CheckIfFramInitializationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    bool isRequested    = false;
    uint64_t signalData = 0u;

    /* get software reset bit from the CAN message */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_FRAM_INITIALIZATION_START_BIT,
        CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_FRAM_INITIALIZATION_LENGTH,
        &signalData,
        endianness);
    if (signalData == 1u) {
        isRequested = true;
    }
    return isRequested;
}

static void CANRX_ProcessSoftwareResetMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    /* trigger software reset, if requested*/
    if (CANRX_CheckIfSoftwareResetIsRequested(messageData, endianness) == true) {
        SYS_TriggerSoftwareReset();
    }
}

static void CANRX_ProcessFramInitializationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    /* trigger FRAM initialization, if requested*/
    if (CANRX_CheckIfFramInitializationIsRequested(messageData, endianness) == true) {
        FRAM_ReinitializeAllEntries();
    }
}

static void CANRX_ProcessTimeInfoMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    /* trigger RTC time information message, if requested*/
    if (CANRX_CheckIfTimeInfoIsRequested(messageData, endianness) == true) {
        CANRX_TriggerTimeInfoMessage();
    }
}

static bool CANRX_CheckIfTimeInfoIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    FAS_ASSERT(endianness == CAN_BIG_ENDIAN);

    bool isRequested    = false;
    uint64_t signalData = 0u;

    /* get RTC time information bit from the CAN message */
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_REQUEST_RTC_TIME_START_BIT,
        CANRX_MUX_SOFTWARE_SIGNAL_TRIGGER_REQUEST_RTC_TIME_LENGTH,
        &signalData,
        endianness);
    if (signalData == 1u) {
        isRequested = true;
    }
    return isRequested;
}

static void CANRX_TriggerTimeInfoMessage(void) {
    /* send the debug message containing the RTC time information and trap if this does not work */
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CANRX_Debug(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(message.id == CANRX_DEBUG_ID);
    FAS_ASSERT(message.idType == CANRX_DEBUG_ID_TYPE);
    FAS_ASSERT(message.dlc == CAN_FOXBMS_MESSAGES_DEFAULT_DLC);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);

    uint64_t messageData = 0u;
    uint64_t muxValue    = 0u;

    /* Get the message data and retrieve the multiplexer value from it*/
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);
    CAN_RxGetSignalDataFromMessageData(
        messageData, CANRX_DEBUG_MESSAGE_MUX_START_BIT, CANRX_DEBUG_MESSAGE_MUX_LENGTH, &muxValue, message.endianness);

    /* process the respective handler function per supported multiplexer value;
       in case of an unsupported multiplexer value, transmit that information
       on the CAN bus. */
    switch (muxValue) {
        case CANRX_DEBUG_MESSAGE_MUX_VALUE_VERSION_INFORMATION:
            CANRX_ProcessVersionInformationMux(messageData, message.endianness);
            break;
        case CANRX_DEBUG_MESSAGE_MUX_VALUE_RTC:
            CANRX_ProcessRtcMux(messageData, message.endianness);
            break;
        case CANRX_DEBUG_MESSAGE_MUX_VALUE_SOFTWARE_RESET:
            CANRX_ProcessSoftwareResetMux(messageData, message.endianness);
            break;
        case CANRX_DEBUG_MESSAGE_MUX_VALUE_FRAM_INITIALIZATION:
            CANRX_ProcessFramInitializationMux(messageData, message.endianness);
            break;
        case CANRX_DEBUG_MESSAGE_MUX_VALUE_TIME_INFO:
            CANRX_ProcessTimeInfoMux(messageData, message.endianness);
            break;
        default:
            CANTX_UnsupportedMultiplexerValue(message.id, (uint32_t)muxValue);
            break;
    }
    return 0u;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

/* export RTC helper functions */
extern uint8_t TEST_CANRX_GetHundredthOfSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetHundredthOfSeconds(messageData, endianness);
}
extern uint8_t TEST_CANRX_GetSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetSeconds(messageData, endianness);
}
extern uint8_t TEST_CANRX_GetMinutes(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetMinutes(messageData, endianness);
}
extern uint8_t TEST_CANRX_GetHours(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetHours(messageData, endianness);
}
extern uint8_t TEST_CANRX_GetWeekday(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetWeekday(messageData, endianness);
}
extern uint8_t TEST_CANRX_GetDay(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetDay(messageData, endianness);
}
extern uint8_t TEST_CANRX_GetMonth(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetMonth(messageData, endianness);
}
extern uint8_t TEST_CANRX_GetYear(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_GetYear(messageData, endianness);
}

/* export actions */
extern void TEST_CANRX_TriggerBmsSoftwareVersionMessage(void) {
    return CANRX_TriggerBmsSoftwareVersionMessage();
}
extern void TEST_CANRX_TriggerMcuUniqueDieIdMessage(void) {
    return CANRX_TriggerMcuUniqueDieIdMessage();
}
extern void TEST_CANRX_TriggerMcuLotNumberMessage(void) {
    return CANRX_TriggerMcuLotNumberMessage();
}
extern void TEST_CANRX_TriggerMcuWaferInformationMessage(void) {
    return CANRX_TriggerMcuWaferInformationMessage();
}
extern void TEST_CANRX_TriggerTimeInfoMessage(void) {
    return CANRX_TriggerTimeInfoMessage();
}

/* export check if functions */
extern bool TEST_CANRX_CheckIfBmsSoftwareVersionIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_CheckIfBmsSoftwareVersionIsRequested(messageData, endianness);
}
extern bool TEST_CANRX_CheckIfMcuUniqueDieIdIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_CheckIfMcuUniqueDieIdIsRequested(messageData, endianness);
}
extern bool TEST_CANRX_CheckIfMcuLotNumberIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_CheckIfMcuLotNumberIsRequested(messageData, endianness);
}
extern bool TEST_CANRX_CheckIfMcuWaferInformationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_CheckIfMcuWaferInformationIsRequested(messageData, endianness);
}
extern bool TEST_CANRX_CheckIfSoftwareResetIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_CheckIfSoftwareResetIsRequested(messageData, endianness);
}
extern bool TEST_CANRX_CheckIfFramInitializationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_CheckIfFramInitializationIsRequested(messageData, endianness);
}
extern bool TEST_CANRX_CheckIfTimeInfoIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    return CANRX_CheckIfTimeInfoIsRequested(messageData, endianness);
}

/* export mux processing functions */
extern void TEST_CANRX_ProcessVersionInformationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    CANRX_ProcessVersionInformationMux(messageData, endianness);
}
extern void TEST_CANRX_ProcessRtcMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    CANRX_ProcessRtcMux(messageData, endianness);
}
extern void TEST_CANRX_ProcessSoftwareResetMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    CANRX_ProcessSoftwareResetMux(messageData, endianness);
}
extern void TEST_CANRX_ProcessFramInitializationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    CANRX_ProcessFramInitializationMux(messageData, endianness);
}
extern void TEST_CANRX_ProcessTimeInfoMux(uint64_t messageData, CAN_ENDIANNESS_e endianness) {
    CANRX_ProcessTimeInfoMux(messageData, endianness);
}

#endif
