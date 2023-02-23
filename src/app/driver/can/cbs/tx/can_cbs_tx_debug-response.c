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
 * @file    can_cbs_tx_debug-response.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
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

#include "version_cfg.h"

#include "HL_het.h"
#include "HL_reg_system.h"

#include "can.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"
#include "database.h"
#include "foxmath.h"
#include "fstd_types.h"
#include "rtc.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** @{
 * multiplexer setup for the debug message
 */
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT (0x7u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH    (8u)
/** @} */

#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BMS_SOFTWARE_VERSION_INFO (0x00u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_UNIQUE_DIE_ID         (0x01u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_LOT_NUMBER            (0x02u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_MCU_WAFER_INFORMATION     (0x03u)
#define CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_RTC_TIME                  (0x04u)
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
#define CANTX_BOOT_MAGIC_DATA_START (0b11111110111111101111111011111110111111101111111011111110uLL)
#define CANTX_BOOT_MAGIC_DATA_END   (0b00000001000000010000000100000001000000010000000100000001uLL)
/** @} */

/** @{
 * Register mapping of 'DIEIDL' (see #CANTX_TransmitMcuWaferInformation)
 * x-coordinate: bits 0-11  -> 0b_0000_00000_0000_0000_0000_1111_1111_1111 = 0x_0000_0FFF
 * y-coordinate: bits 12-23 -> 0b_0000_00000_1111_1111_1111_0000_0000_0000 = 0x_00FF_F000
 * wafer number: bits 24-32 -> 0b_1111_11111_0000_0000_0000_0000_0000_0000 = 0x_FF00_0000
 *
 * source : SPNU563A-March 2018: Table 2-47. Die Identification Register, Lower Word (DIEIDL) Field Descriptions
 * */
#define CANTX_WAFER_X_COORDINATE_BITMASK       (0x00000FFFuLL)
#define CANTX_WAFER_Y_COORDINATE_BITMASK       (0x00FFF000uLL)
#define CANTX_WAFER_Y_COORDINATE_SHIFT_12_BITS (12uLL)
#define CANTX_WAFER_NUMBER_BITMASK             (0xFF000000uLL)
#define CANTX_WAFER_NUMBER_SHIFT_24_BITS       (24uLL)
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Transmit the embedded software version information
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitBmsVersionInfo(void);

/**
 * @brief   Transmit the MCU's unique id
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitMcuUniqueDieId(void);

/**
 * @brief   Transmit the MCU's lot number ID information
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitMcuLotNumber(void);

/**
 * @brief   Transmit the MCU's wafer information
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitMcuWaferInformation(void);

/**
 * @brief   Transmit a boot message
 * @param   messageData message data to be put in the boot message
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitBootMagic(uint64_t messageData);
/**
 * @brief   Transmit the boot message and its magic start data
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitBootMagicStart(void);

/**
 * @brief   Transmit the boot message and its magic end data
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitBootMagicEnd(void);

/**
 * @brief   Transmit the RTC time message
 * @return  #STD_OK if transmission successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e CANTX_TransmitRtcTime(void);

/*========== Static Function Implementations ================================*/

static STD_RETURN_TYPE_e CANTX_TransmitBmsVersionInfo(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set release distance (capped to maximum value) */
    const uint8_t distanceCapped = (uint8_t)MATH_MinimumOfTwoUint16_t(
        ver_foxbmsVersionInformation.distanceFromLastRelease, (uint16_t)CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE);
    uint8_t releaseDistanceOverflow = 0;
    if (ver_foxbmsVersionInformation.distanceFromLastRelease > CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE) {
        releaseDistanceOverflow = 1;
    }
    uint64_t isDirty = 0;
    if (ver_foxbmsVersionInformation.isDirty == true) {
        isDirty = 1;
    }
    uint64_t underVersionControl = 0;
    if (ver_foxbmsVersionInformation.underVersionControl == true) {
        underVersionControl = 1;
    }

    uint64_t message = 0;
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
        ver_foxbmsVersionInformation.major,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MINOR_VERSION_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_MINOR_VERSION_LENGTH,
        ver_foxbmsVersionInformation.minor,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_PATCH_VERSION_START_BIT,
        CANTX_MUX_BMS_SOFTWARE_VERSION_INFO_SIGNAL_PATCH_VERSION_LENGTH,
        ver_foxbmsVersionInformation.patch,
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
    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_BIG_ENDIAN);
    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_RESPONSE_ID, CANTX_DEBUG_IDENTIFIER, &data[0]);

    return successfullyQueued;
}

static STD_RETURN_TYPE_e CANTX_TransmitMcuUniqueDieId(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Read out device register with unique ID */
    const uint32_t deviceRegister = systemREG1->DEVID;

    uint64_t message = 0;
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
    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_BIG_ENDIAN);

    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_RESPONSE_ID, CANTX_DEBUG_IDENTIFIER, &data[0]);
    return successfullyQueued;
}

static STD_RETURN_TYPE_e CANTX_TransmitMcuLotNumber(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Read out device register with die ID high */
    const uint32_t dieIdHigh = systemREG1->DIEIDH; /* equals the lot number */

    uint64_t message = 0;
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
    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_BIG_ENDIAN);

    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_RESPONSE_ID, CANTX_DEBUG_IDENTIFIER, &data[0]);
    return successfullyQueued;
}

static STD_RETURN_TYPE_e CANTX_TransmitMcuWaferInformation(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Read out device register with die ID low */
    /* SPNU563A-March 2018: 2.5.1.28 Die Identification Register Lower Word (DIEIDL) */
    const uint64_t dieIdLow = (uint64_t)systemREG1->DIEIDL;

    /* see doxygen comment of the macros */
    uint64_t xWaferCoordinate = dieIdLow & CANTX_WAFER_X_COORDINATE_BITMASK;
    uint64_t yWaferCoordinate = (dieIdLow & CANTX_WAFER_Y_COORDINATE_BITMASK) >> CANTX_WAFER_Y_COORDINATE_SHIFT_12_BITS;
    uint64_t waferNumber      = (dieIdLow & CANTX_WAFER_NUMBER_BITMASK) >> CANTX_WAFER_NUMBER_SHIFT_24_BITS;

    uint64_t message = 0;
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
    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_BIG_ENDIAN);
    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_RESPONSE_ID, CANTX_DEBUG_IDENTIFIER, &data[0]);

    return successfullyQueued;
}

static STD_RETURN_TYPE_e CANTX_TransmitBootMagic(uint64_t messageData) {
    FAS_ASSERT((messageData == CANTX_BOOT_MAGIC_DATA_START) || (messageData == CANTX_BOOT_MAGIC_DATA_END));
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    uint64_t message = 0;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_START_BIT,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_LENGTH,
        CANTX_DEBUG_RESPONSE_MESSAGE_MUX_VALUE_BOOT_INFORMATION,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message, CANTX_MUX_BOOT_SIGNAL_START_BIT, CANTX_MUX_BOOT_SIGNAL_LENGTH, messageData, CAN_BIG_ENDIAN);
    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_BIG_ENDIAN);
    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_RESPONSE_ID, CANTX_DEBUG_IDENTIFIER, &data[0]);
    return successfullyQueued;
}

static STD_RETURN_TYPE_e CANTX_TransmitBootMagicStart(void) {
    return CANTX_TransmitBootMagic(CANTX_BOOT_MAGIC_DATA_START);
}

static STD_RETURN_TYPE_e CANTX_TransmitBootMagicEnd(void) {
    return CANTX_TransmitBootMagic(CANTX_BOOT_MAGIC_DATA_END);
}

static STD_RETURN_TYPE_e CANTX_TransmitRtcTime(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    RTC_TIME_DATA_s currentRtcTime = RTC_GetSystemTimeRtcFormat();

    uint64_t message = 0;
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

    CAN_TxSetCanDataWithMessageData(message, &data[0], CAN_BIG_ENDIAN);
    STD_RETURN_TYPE_e successfullyQueued =
        CAN_DataSend(CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_RESPONSE_ID, CANTX_DEBUG_IDENTIFIER, &data[0]);
    return successfullyQueued;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_ACTIONS_e action) {
    STD_RETURN_TYPE_e successfullyQueued = STD_NOT_OK;
    switch (action) {
        case CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO:
            successfullyQueued = CANTX_TransmitBmsVersionInfo();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID:
            successfullyQueued = CANTX_TransmitMcuUniqueDieId();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER:
            successfullyQueued = CANTX_TransmitMcuLotNumber();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION:
            successfullyQueued = CANTX_TransmitMcuWaferInformation();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START:
            successfullyQueued = CANTX_TransmitBootMagicStart();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END:
            successfullyQueued = CANTX_TransmitBootMagicEnd();
            break;
        case CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME:
            successfullyQueued = CANTX_TransmitRtcTime();
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
    return successfullyQueued;
}
/*========== Getter for static Variables (Unit Test) ========================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
