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
 * @file    can_cbs_tx_debug-response.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Implementation for handling the transmit of debug response messages
 * @details Implementation of the interface for transmitting debug response
 *          messages (#CANTX_DebugResponse).
 *          The interface only serves as a wrapper that calls internally the
 *          respective functions to transmit the debug data.
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "can_cbs_tx_debug-response.h"

#include "can.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"
#include "database.h"
#include "foxmath.h"
#include "fstd_types.h"
#include "mcu.h"
#include "rtc.h"
#include "utils.h"
#include "version.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** @{
 * multiplexer setup for the debug response message
 */
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT (0x7u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH    (8u)
/** @} */

#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BMS_SOFTWARE_VERSION_INFO (0x00u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_UNIQUE_DIE_ID         (0x01u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_LOT_NUMBER            (0x02u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_WAFER_INFORMATION     (0x03u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_RTC_TIME                  (0x04u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_COMMIT_HASH_HIGH_7        (0x05u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_COMMIT_HASH_LOW_7         (0x06u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_UPTIME                    (0x07u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BOOT_TIMESTAMP            (0x0Eu)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BOOT_INFORMATION          (0x0Fu)

/** @{
 * configuration of the BMS software version information signals for
 * multiplexer 'BmsSoftwareVersionInfo' in the 'DebugResponse' message
 */
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MAJOR_VERSION_START_BIT             (15u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MAJOR_VERSION_LENGTH                (8u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MINOR_VERSION_START_BIT             (23u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MINOR_VERSION_LENGTH                (8u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_PATCH_VERSION_START_BIT             (31u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_PATCH_VERSION_LENGTH                (8u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_START_BIT          (39u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_LENGTH             (5u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_START_BIT (34u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_LENGTH    (1u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_DIRTY_FLAG_START_BIT                (33u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_DIRTY_FLAG_LENGTH                   (1u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_START_BIT     (32u)
#define CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_LENGTH        (1u)
/** @} */

/** @{
 * configuration of the RTC signals for multiplexer 'RtcTime' in the
 * 'DebugResponse' message
 */
#define CANTX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_START_BIT (15u)
#define CANTX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_LENGTH    (7u)
#define CANTX_MUX_RTC_SIGNAL_SECONDS_START_BIT              (8u)
#define CANTX_MUX_RTC_SIGNAL_SECONDS_LENGTH                 (6u)
#define CANTX_MUX_RTC_SIGNAL_MINUTES_START_BIT              (18u)
#define CANTX_MUX_RTC_SIGNAL_MINUTES_LENGTH                 (6u)
#define CANTX_MUX_RTC_SIGNAL_HOURS_START_BIT                (28u)
#define CANTX_MUX_RTC_SIGNAL_HOURS_LENGTH                   (5u)
#define CANTX_MUX_RTC_SIGNAL_WEEKDAY_START_BIT              (39u)
#define CANTX_MUX_RTC_SIGNAL_WEEKDAY_LENGTH                 (3u)
#define CANTX_MUX_RTC_SIGNAL_DAY_START_BIT                  (36u)
#define CANTX_MUX_RTC_SIGNAL_DAY_LENGTH                     (5u)
#define CANTX_MUX_RTC_SIGNAL_MONTH_START_BIT                (47u)
#define CANTX_MUX_RTC_SIGNAL_MONTH_LENGTH                   (4u)
#define CANTX_MUX_RTC_SIGNAL_YEAR_START_BIT                 (43u)
#define CANTX_MUX_RTC_SIGNAL_YEAR_LENGTH                    (7u)
#define CANTX_MUX_RTC_SIGNAL_REQUEST_FLAG_START_BIT         (52u)
#define CANTX_MUX_RTC_SIGNAL_REQUEST_FLAG_LENGTH            (2u)
/** @} */

/** @{
 * configuration of the OS signals for multiplexer 'Uptime' in the
 * 'DebugResponse' message
 */
#define CANTX_MUX_OS_SIGNAL_THOUSANDTH_OF_SECONDS_START_BIT (15u)
#define CANTX_MUX_OS_SIGNAL_THOUSANDTH_OF_SECONDS_LENGTH    (10u)
#define CANTX_MUX_OS_SIGNAL_HUNDREDTH_OF_SECONDS_START_BIT  (21u)
#define CANTX_MUX_OS_SIGNAL_HUNDREDTH_OF_SECONDS_LENGTH     (7u)
#define CANTX_MUX_OS_SIGNAL_TENTH_OF_SECONDS_START_BIT      (30u)
#define CANTX_MUX_OS_SIGNAL_TENTH_OF_SECONDS_LENGTH         (4u)
#define CANTX_MUX_OS_SIGNAL_SECONDS_START_BIT               (26u)
#define CANTX_MUX_OS_SIGNAL_SECONDS_LENGTH                  (6u)
#define CANTX_MUX_OS_SIGNAL_MINUTES_START_BIT               (36u)
#define CANTX_MUX_OS_SIGNAL_MINUTES_LENGTH                  (6u)
#define CANTX_MUX_OS_SIGNAL_HOURS_START_BIT                 (46u)
#define CANTX_MUX_OS_SIGNAL_HOURS_LENGTH                    (5u)
#define CANTX_MUX_OS_SIGNAL_DAY_START_BIT                   (41u)
#define CANTX_MUX_OS_SIGNAL_DAY_LENGTH                      (5u)
/** @} */

/** maximum distance from release that can be encoded in the boot message */
#define CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE (31u)
#if CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE > UINT8_MAX
#error "This code assumes that the define is smaller or equal to UINT8_MAX")
#endif

/** @{
 * configuration of the lot message signals for multiplexer 'McuLotNumber'
 * in the 'DebugResponse' message
 */
#define CANTX_MUX_MCU_UNIQUE_DIE_ID_SIGNAL_UNIQUE_DIE_ID_START_BIT (15u)
#define CANTX_MUX_MCU_UNIQUE_DIE_ID_SIGNAL_UNIQUE_DIE_ID_LENGTH    (32u)
/** @} */

/** @{
 * configuration of the lot message signals for multiplexer 'McuLotNumber'
 * in the 'DebugResponse' message
 */
#define CANTX_MUX_MCU_LOT_NUMBER_SIGNAL_LOT_NUMBER_START_BIT (15u)
#define CANTX_MUX_MCU_LOT_NUMBER_SIGNAL_LOT_NUMBER_LENGTH    (32u)
/** @} */

/** @{
 * configuration of the wafer information message signals for multiplexer
 * 'McuWaferInformation' in the 'DebugResponse' message
 */
#define CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_WAFER_NUMBER_START_BIT       (15u)
#define CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_WAFER_NUMBER_LENGTH          (8u)
#define CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_X_WAFER_COORDINATE_START_BIT (27u)
#define CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_X_WAFER_COORDINATE_LENGTH    (12u)
#define CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_Y_WAFER_COORDINATE_START_BIT (23u)
#define CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_Y_WAFER_COORDINATE_LENGTH    (12u)
/** @} */

/** @{
 * configuration of the boot message signals for multiplexer 'BootInformation'
 * in the 'DebugResponse' message
 */
#define CANTX_MUX_BOOT_SIGNAL_START_BIT (15u)
#define CANTX_MUX_BOOT_SIGNAL_LENGTH    (56u)
/** @} */

/** @{
 * Magic data, i.e., bit muster, in the boot message
 */
#define CANTX_BOOT_MAGIC_DATA_START (0xFEFEFEFEFEFEFEuLL)
#define CANTX_BOOT_MAGIC_DATA_END   (0x01010101010101uLL)
/** @} */

/** @{
 * Register mapping of 'DIEIDL' (see #CANTX_TransmitMcuWaferInformation)
 * x-coordinate: bits 0-11  -> 0b_0000_00000_0000_0000_0000_1111_1111_1111 = 0x00000FFF
 * y-coordinate: bits 12-23 -> 0b_0000_00000_1111_1111_1111_0000_0000_0000 = 0x00FFF000
 * wafer number: bits 24-32 -> 0b_1111_11111_0000_0000_0000_0000_0000_0000 = 0xFF000000
 *
 * docref: SPNU563A-March 2018: Table 2-47. Die Identification Register, Lower Word (DIEIDL) Field Descriptions
 * */
#define CANTX_WAFER_X_COORDINATE_BITMASK       (0x00000FFFuLL)
#define CANTX_WAFER_Y_COORDINATE_BITMASK       (0x00FFF000uLL)
#define CANTX_WAFER_Y_COORDINATE_SHIFT_12_BITS (12uLL)
#define CANTX_WAFER_NUMBER_BITMASK             (0xFF000000uLL)
#define CANTX_WAFER_NUMBER_SHIFT_24_BITS       (24uLL)
/** @} */

/** @{
 * configuration of the BMS software version information signals for
 * multiplexer 'CommitHash' in the 'DebugResponse' message
 */
#define CANTX_MUX_COMMIT_HASH_SIGNAL_STRING_START_BIT (15u)
#define CANTX_MUX_COMMIT_HASH_SIGNAL_STRING_LENGTH    (56u)
#define CANTX_MUX_COMMIT_HASH_HIGH_FIRST_CHAR         (0u)
#define CANTX_MUX_COMMIT_HASH_LOW_FIRST_CHAR          (7u)
#define CANTX_MUX_COMMIT_HASH_CHARS_PER_MUX           (7u)
#define CANTX_MUX_COMMIT_HASH_SHA_LENGTH              (14u)
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Transmit the embedded software version information
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitBmsVersionInfo(void);

/**
 * @brief   Transmit the MCU's unique id
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitMcuUniqueDieId(void);

/**
 * @brief   Transmit the MCU's lot number ID information
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitMcuLotNumber(void);

/**
 * @brief   Transmit the MCU's wafer information
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitMcuWaferInformation(void);

/**
 * @brief   Transmit a boot message
 * @param   messageData message data to be put in the boot message
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitBootMagic(uint64_t messageData);

/**
 * @brief   Transmit the boot message and its magic start data
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitBootMagicStart(void);

/**
 * @brief   Transmit the boot timestamp message
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitBootTimeStamp(void);

/**
 * @brief   Transmit the boot message and its magic end data
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitBootMagicEnd(void);

/**
 * @brief   Transmit the RTC time message
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitRtcTime(void);

/**
 * @brief   Transmit the uptime message
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitUptime(void);

/**
 * @brief Transmit the complete short commit hash in two messages
 * @return  message data for the can message
 */
static STD_RETURN_TYPE_e CANTX_TransmitCommitHash(void);

/**
 * @brief Transmit the first seven characters of the short commit hash
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitCommitHashHigh(void);

/**
 * @brief Transmit the last character of the short commit hash
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitCommitHashLow(void);

/**
 * @brief Sets the can data and sends the message
 * @param messageData Data that will be transmitted with the can message
 * @return #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_DebugResponseSendMessage(uint64_t messageData);

/*========== Static Function Implementations ================================*/

static uint64_t CANTX_TransmitBmsVersionInfo(void) {
    /* Set release distance (capped to maximum value) */
    const uint8_t distanceCapped = (uint8_t)MATH_MinimumOfTwoUint16_t(
        ver_versionInformation.distanceFromLastRelease, (uint16_t)CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE);
    uint8_t releaseDistanceOverflow = 0u;
    if (ver_versionInformation.distanceFromLastRelease > CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE) {
        releaseDistanceOverflow = 1u;
    }
    uint64_t isDirty = 0u;
    if (ver_versionInformation.isDirty == true) {
        isDirty = 1u;
    }
    uint64_t underVersionControl = 0u;
    if (ver_versionInformation.underVersionControl == true) {
        underVersionControl = 1u;
    }

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BMS_SOFTWARE_VERSION_INFO,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MAJOR_VERSION_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MAJOR_VERSION_LENGTH,
        ver_versionInformation.major,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MINOR_VERSION_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MINOR_VERSION_LENGTH,
        ver_versionInformation.minor,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_PATCH_VERSION_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_PATCH_VERSION_LENGTH,
        ver_versionInformation.patch,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_LENGTH,
        distanceCapped,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_LENGTH,
        releaseDistanceOverflow,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_DIRTY_FLAG_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_DIRTY_FLAG_LENGTH,
        isDirty,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_LENGTH,
        underVersionControl,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitMcuUniqueDieId(void) {
    const uint32_t deviceRegister = MCU_GetDeviceRegister();

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_UNIQUE_DIE_ID,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_MCU_UNIQUE_DIE_ID_SIGNAL_UNIQUE_DIE_ID_START_BIT,
        CANTX_MUX_MCU_UNIQUE_DIE_ID_SIGNAL_UNIQUE_DIE_ID_LENGTH,
        deviceRegister,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitMcuLotNumber(void) {
    const uint32_t dieIdHigh = MCU_GetDieIdHigh();

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_LOT_NUMBER,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_MCU_LOT_NUMBER_SIGNAL_LOT_NUMBER_START_BIT,
        CANTX_MUX_MCU_LOT_NUMBER_SIGNAL_LOT_NUMBER_LENGTH,
        dieIdHigh,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitMcuWaferInformation(void) {
    const uint32_t dieIdLow = MCU_GetDieIdLow();

    uint64_t xWaferCoordinate = (uint64_t)(dieIdLow & CANTX_WAFER_X_COORDINATE_BITMASK);
    uint64_t yWaferCoordinate =
        (uint64_t)((dieIdLow & CANTX_WAFER_Y_COORDINATE_BITMASK) >> CANTX_WAFER_Y_COORDINATE_SHIFT_12_BITS);
    uint64_t waferNumber = (uint64_t)((dieIdLow & CANTX_WAFER_NUMBER_BITMASK) >> CANTX_WAFER_NUMBER_SHIFT_24_BITS);

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_WAFER_INFORMATION,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_WAFER_NUMBER_START_BIT,
        CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_WAFER_NUMBER_LENGTH,
        waferNumber,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_X_WAFER_COORDINATE_START_BIT,
        CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_X_WAFER_COORDINATE_LENGTH,
        xWaferCoordinate,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_Y_WAFER_COORDINATE_START_BIT,
        CANTX_MUX_MCU_WAFER_INFORMATION_SIGNAL_Y_WAFER_COORDINATE_LENGTH,
        yWaferCoordinate,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitBootMagic(uint64_t messageData) {
    FAS_ASSERT((messageData == CANTX_BOOT_MAGIC_DATA_START) || (messageData == CANTX_BOOT_MAGIC_DATA_END));

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BOOT_INFORMATION,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message, CANTX_MUX_BOOT_SIGNAL_START_BIT, CANTX_MUX_BOOT_SIGNAL_LENGTH, messageData, CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitBootTimeStamp(void) {
    RTC_TIME_DATA_s timestamp = RTC_GetSystemStartUpTime();

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BOOT_TIMESTAMP,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_START_BIT,
        CANTX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_LENGTH,
        timestamp.hundredthOfSeconds,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_SECONDS_START_BIT,
        CANTX_MUX_RTC_SIGNAL_SECONDS_LENGTH,
        timestamp.seconds,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_MINUTES_START_BIT,
        CANTX_MUX_RTC_SIGNAL_MINUTES_LENGTH,
        timestamp.minutes,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_HOURS_START_BIT,
        CANTX_MUX_RTC_SIGNAL_HOURS_LENGTH,
        timestamp.hours,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_WEEKDAY_START_BIT,
        CANTX_MUX_RTC_SIGNAL_WEEKDAY_LENGTH,
        timestamp.weekday,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message, CANTX_MUX_RTC_SIGNAL_DAY_START_BIT, CANTX_MUX_RTC_SIGNAL_DAY_LENGTH, timestamp.day, CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_MONTH_START_BIT,
        CANTX_MUX_RTC_SIGNAL_MONTH_LENGTH,
        timestamp.month,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_YEAR_START_BIT,
        CANTX_MUX_RTC_SIGNAL_YEAR_LENGTH,
        timestamp.year,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitBootMagicStart(void) {
    return CANTX_TransmitBootMagic(CANTX_BOOT_MAGIC_DATA_START);
}

static uint64_t CANTX_TransmitBootMagicEnd(void) {
    return CANTX_TransmitBootMagic(CANTX_BOOT_MAGIC_DATA_END);
}

static uint64_t CANTX_TransmitRtcTime(void) {
    RTC_TIME_DATA_s currentRtcTime = RTC_GetSystemTimeRtcFormat();

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_RTC_TIME,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_START_BIT,
        CANTX_MUX_RTC_SIGNAL_HUNDREDTH_OF_SECONDS_LENGTH,
        currentRtcTime.hundredthOfSeconds,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_SECONDS_START_BIT,
        CANTX_MUX_RTC_SIGNAL_SECONDS_LENGTH,
        currentRtcTime.seconds,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_MINUTES_START_BIT,
        CANTX_MUX_RTC_SIGNAL_MINUTES_LENGTH,
        currentRtcTime.minutes,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_HOURS_START_BIT,
        CANTX_MUX_RTC_SIGNAL_HOURS_LENGTH,
        currentRtcTime.hours,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_WEEKDAY_START_BIT,
        CANTX_MUX_RTC_SIGNAL_WEEKDAY_LENGTH,
        currentRtcTime.weekday,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_DAY_START_BIT,
        CANTX_MUX_RTC_SIGNAL_DAY_LENGTH,
        currentRtcTime.day,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_MONTH_START_BIT,
        CANTX_MUX_RTC_SIGNAL_MONTH_LENGTH,
        currentRtcTime.month,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_YEAR_START_BIT,
        CANTX_MUX_RTC_SIGNAL_YEAR_LENGTH,
        currentRtcTime.year,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_RTC_SIGNAL_REQUEST_FLAG_START_BIT,
        CANTX_MUX_RTC_SIGNAL_REQUEST_FLAG_LENGTH,
        currentRtcTime.requestFlag,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitUptime(void) {
    OS_TIMER_s currentOsTimer = OS_GetOsTimer();

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_UPTIME,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_OS_SIGNAL_THOUSANDTH_OF_SECONDS_START_BIT,
        CANTX_MUX_OS_SIGNAL_THOUSANDTH_OF_SECONDS_LENGTH,
        currentOsTimer.timer_1ms,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_OS_SIGNAL_HUNDREDTH_OF_SECONDS_START_BIT,
        CANTX_MUX_OS_SIGNAL_HUNDREDTH_OF_SECONDS_LENGTH,
        currentOsTimer.timer_10ms,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_OS_SIGNAL_TENTH_OF_SECONDS_START_BIT,
        CANTX_MUX_OS_SIGNAL_TENTH_OF_SECONDS_LENGTH,
        currentOsTimer.timer_100ms,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_OS_SIGNAL_SECONDS_START_BIT,
        CANTX_MUX_OS_SIGNAL_SECONDS_LENGTH,
        currentOsTimer.timer_sec,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_OS_SIGNAL_MINUTES_START_BIT,
        CANTX_MUX_OS_SIGNAL_MINUTES_LENGTH,
        currentOsTimer.timer_min,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_OS_SIGNAL_HOURS_START_BIT,
        CANTX_MUX_OS_SIGNAL_HOURS_LENGTH,
        currentOsTimer.timer_h,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_OS_SIGNAL_DAY_START_BIT,
        CANTX_MUX_OS_SIGNAL_DAY_LENGTH,
        currentOsTimer.timer_d,
        CAN_BIG_ENDIAN);

    return message;
}

static STD_RETURN_TYPE_e CANTX_TransmitCommitHash(void) {
    STD_RETURN_TYPE_e successfullyQueued = STD_NOT_OK;
    /* prepare and send the first 7 high characters*/
    uint64_t messageData = CANTX_TransmitCommitHashHigh();
    successfullyQueued   = CANTX_DebugResponseSendMessage(messageData);

    /* check if the message was queued successfully */
    if (successfullyQueued == STD_OK) {
        /* prepare and send the last 2 low characters */
        messageData        = CANTX_TransmitCommitHashLow();
        successfullyQueued = CANTX_DebugResponseSendMessage(messageData);
    }

    return successfullyQueued;
}

static uint64_t CANTX_TransmitCommitHashHigh(void) {
    uint64_t message    = 0u;
    uint64_t signalData = 0u;

    /* set message data with mux value and first seven chars of commit hash*/
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_COMMIT_HASH_HIGH_7,
        CAN_BIG_ENDIAN);
    UTIL_ExtractCharactersFromString(
        &signalData,
        ver_versionInformation.commitHash,
        CANTX_MUX_COMMIT_HASH_SHA_LENGTH,
        CANTX_MUX_COMMIT_HASH_HIGH_FIRST_CHAR,
        CANTX_MUX_COMMIT_HASH_CHARS_PER_MUX);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_COMMIT_HASH_SIGNAL_STRING_START_BIT,
        CANTX_MUX_COMMIT_HASH_SIGNAL_STRING_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitCommitHashLow(void) {
    uint64_t message    = 0u;
    uint64_t signalData = 0u;

    /* set message data with mux value and last char of commit hash*/
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_COMMIT_HASH_LOW_7,
        CAN_BIG_ENDIAN);
    UTIL_ExtractCharactersFromString(
        &signalData,
        ver_versionInformation.commitHash,
        CANTX_MUX_COMMIT_HASH_SHA_LENGTH,
        CANTX_MUX_COMMIT_HASH_LOW_FIRST_CHAR,
        CANTX_MUX_COMMIT_HASH_CHARS_PER_MUX);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_COMMIT_HASH_SIGNAL_STRING_START_BIT,
        CANTX_MUX_COMMIT_HASH_SIGNAL_STRING_LENGTH,
        signalData,
        CAN_BIG_ENDIAN);

    return message;
}

static STD_RETURN_TYPE_e CANTX_DebugResponseSendMessage(uint64_t messageData) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    CAN_TxSetCanDataWithMessageData(messageData, &data[0], CAN_BIG_ENDIAN);
    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_RESPONSE_ID, CANTX_DEBUG_RESPONSE_ID_TYPE, &data[0]);

    return successfullyQueued;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_ACTIONS_e action) {
    STD_RETURN_TYPE_e successfullyQueued = STD_NOT_OK;
    uint64_t messageData                 = 0u;
    switch (action) {
        case CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO:
            messageData = CANTX_TransmitBmsVersionInfo();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID:
            messageData = CANTX_TransmitMcuUniqueDieId();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER:
            messageData = CANTX_TransmitMcuLotNumber();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION:
            messageData = CANTX_TransmitMcuWaferInformation();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START:
            messageData = CANTX_TransmitBootMagicStart();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END:
            messageData = CANTX_TransmitBootMagicEnd();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_TIMESTAMP:
            messageData = CANTX_TransmitBootTimeStamp();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME:
            messageData = CANTX_TransmitRtcTime();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_UPTIME:
            messageData = CANTX_TransmitUptime();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH:
            successfullyQueued = CANTX_TransmitCommitHash();
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
    if (action != CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH) {
        successfullyQueued = CANTX_DebugResponseSendMessage(messageData);
    }
    return successfullyQueued;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CANTX_TransmitBmsVersionInfo(void) {
    return CANTX_TransmitBmsVersionInfo();
}
extern uint64_t TEST_CANTX_TransmitMcuUniqueDieId(void) {
    return CANTX_TransmitMcuUniqueDieId();
}
extern uint64_t TEST_CANTX_TransmitMcuLotNumber(void) {
    return CANTX_TransmitMcuLotNumber();
}
extern uint64_t TEST_CANTX_TransmitMcuWaferInformation(void) {
    return CANTX_TransmitMcuWaferInformation();
}
extern uint64_t TEST_CANTX_TransmitBootMagic(uint64_t messageData) {
    return CANTX_TransmitBootMagic(messageData);
}
extern uint64_t TEST_CANTX_TransmitBootMagicStart(void) {
    return CANTX_TransmitBootMagicStart();
}
extern uint64_t TEST_CANTX_TransmitBootMagicEnd(void) {
    return CANTX_TransmitBootMagicEnd();
}
extern uint64_t TEST_CANTX_TransmitBootTimeStamp(void) {
    return CANTX_TransmitBootTimeStamp();
}
extern uint64_t TEST_CANTX_TransmitRtcTime(void) {
    return CANTX_TransmitRtcTime();
}
extern uint64_t TEST_CANTX_TransmitUptime(void) {
    return CANTX_TransmitUptime();
}
extern STD_RETURN_TYPE_e TEST_CANTX_TransmitCommitHash(void) {
    return CANTX_TransmitCommitHash();
}
extern uint64_t TEST_CANTX_TransmitCommitHashLow(void) {
    return CANTX_TransmitCommitHashLow();
}
extern uint64_t TEST_CANTX_TransmitCommitHashHigh(void) {
    return CANTX_TransmitCommitHashHigh();
}
extern STD_RETURN_TYPE_e TEST_CANTX_DebugResponseSendMessage(uint64_t messageData) {
    return CANTX_DebugResponseSendMessage(messageData);
}
#endif
