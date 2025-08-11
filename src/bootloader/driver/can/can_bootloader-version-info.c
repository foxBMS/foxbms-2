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
 * @file    can_bootloader-version-info.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Implementation for handling the transmit of debug response messages
 * @details Implementation of the interface for transmitting bootloader version
 *          info messages (#CANTX_VersionInfo).
 *          The interface only serves as a wrapper that calls internally the
 *          respective functions to transmit the debug data.
 */

/*========== Includes =======================================================*/
#include "can_bootloader-version-info.h"

#include "can_cfg.h"

#include "can.h"
#include "can_helper.h"
#include "fassert.h"
#include "foxmath.h"
#include "fstd_types.h"
#include "utils.h"
#include "version.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** CAN message properties for debug response message. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - data length
 *  - Endianness @{*/
#define CANTX_VERSION_INFO_ID         (0x410u)
#define CANTX_VERSION_INFO_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_VERSION_INFO_ENDIANNESS (CAN_BIG_ENDIAN)
/**@}*/

/** @{
 * multiplexer setup for the debug response message
 */
#define CANTX_VERSION_INFO_MESSAGE_MUX_START_BIT (0x7u)
#define CANTX_VERSION_INFO_MESSAGE_MUX_LENGTH    (8u)
/** @} */

#define CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_BOOTLOADER_VERSION_INFO (0x00u)
#define CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_COMMIT_HASH_HIGH_7      (0x05u)
#define CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_COMMIT_HASH_LOW_7       (0x06u)
#define CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_BOOT_INFORMATION        (0x0Fu)

/** @{
 * configuration of the Bootloader software version information signals for
 * multiplexer 'BootloaderVersionInfo' in the 'DebugResponse' message
 */
#define CANTX_MUX_VERSION_INFO_SIGNAL_MAJOR_VERSION_START_BIT             (15u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_MAJOR_VERSION_LENGTH                (8u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_MINOR_VERSION_START_BIT             (23u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_MINOR_VERSION_LENGTH                (8u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_PATCH_VERSION_START_BIT             (31u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_PATCH_VERSION_LENGTH                (8u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_START_BIT          (39u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_LENGTH             (5u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_START_BIT (34u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_LENGTH    (1u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_DIRTY_FLAG_START_BIT                (33u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_DIRTY_FLAG_LENGTH                   (1u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_START_BIT     (32u)
#define CANTX_MUX_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_LENGTH        (1u)
/** @} */

/** maximum distance from release that can be encoded in the boot message */
#define CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE (31u)
#if CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE > UINT8_MAX
#error "This code assumes that the define is smaller or equal to UINT8_MAX")
#endif

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
 * configuration of the BOOTLOADER software version information signals for
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
static uint64_t CANTX_TransmitBootloaderVersionInfo(void);

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
 * @brief   Transmit the boot message and its magic end data
 * @return  message data for the can message
 */
static uint64_t CANTX_TransmitBootMagicEnd(void);

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
static STD_RETURN_TYPE_e CANTX_VersionInfoSendMessage(uint64_t messageData);

/*========== Static Function Implementations ================================*/
static uint64_t CANTX_TransmitBootloaderVersionInfo(void) {
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
        CANTX_VERSION_INFO_MESSAGE_MUX_START_BIT,
        CANTX_VERSION_INFO_MESSAGE_MUX_LENGTH,
        CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_BOOTLOADER_VERSION_INFO,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_VERSION_INFO_SIGNAL_MAJOR_VERSION_START_BIT,
        CANTX_MUX_VERSION_INFO_SIGNAL_MAJOR_VERSION_LENGTH,
        ver_versionInformation.major,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_VERSION_INFO_SIGNAL_MINOR_VERSION_START_BIT,
        CANTX_MUX_VERSION_INFO_SIGNAL_MINOR_VERSION_LENGTH,
        ver_versionInformation.minor,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_VERSION_INFO_SIGNAL_PATCH_VERSION_START_BIT,
        CANTX_MUX_VERSION_INFO_SIGNAL_PATCH_VERSION_LENGTH,
        ver_versionInformation.patch,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_START_BIT,
        CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_LENGTH,
        distanceCapped,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_START_BIT,
        CANTX_MUX_VERSION_INFO_SIGNAL_RELEASE_DISTANCE_OVERFLOW_LENGTH,
        releaseDistanceOverflow,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_VERSION_INFO_SIGNAL_DIRTY_FLAG_START_BIT,
        CANTX_MUX_VERSION_INFO_SIGNAL_DIRTY_FLAG_LENGTH,
        isDirty,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_MUX_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_START_BIT,
        CANTX_MUX_VERSION_INFO_SIGNAL_UNDER_VERSION_CONTROL_LENGTH,
        underVersionControl,
        CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitBootMagic(uint64_t messageData) {
    FAS_ASSERT((messageData == CANTX_BOOT_MAGIC_DATA_START) || (messageData == CANTX_BOOT_MAGIC_DATA_END));

    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_VERSION_INFO_MESSAGE_MUX_START_BIT,
        CANTX_VERSION_INFO_MESSAGE_MUX_LENGTH,
        CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_BOOT_INFORMATION,
        CAN_BIG_ENDIAN);
    CAN_TxSetMessageDataWithSignalData(
        &message, CANTX_MUX_BOOT_SIGNAL_START_BIT, CANTX_MUX_BOOT_SIGNAL_LENGTH, messageData, CAN_BIG_ENDIAN);

    return message;
}

static uint64_t CANTX_TransmitBootMagicStart(void) {
    return CANTX_TransmitBootMagic(CANTX_BOOT_MAGIC_DATA_START);
}

static uint64_t CANTX_TransmitBootMagicEnd(void) {
    return CANTX_TransmitBootMagic(CANTX_BOOT_MAGIC_DATA_END);
}

static STD_RETURN_TYPE_e CANTX_TransmitCommitHash(void) {
    STD_RETURN_TYPE_e successfullyQueued = STD_NOT_OK;
    /* prepare and send the first 7 high characters*/
    uint64_t messageData = CANTX_TransmitCommitHashHigh();
    successfullyQueued   = CANTX_VersionInfoSendMessage(messageData);

    /* check if the message was queued successfully */
    if (successfullyQueued == STD_OK) {
        /* prepare and send the last 7 low characters */
        messageData        = CANTX_TransmitCommitHashLow();
        successfullyQueued = CANTX_VersionInfoSendMessage(messageData);
    }

    return successfullyQueued;
}

static uint64_t CANTX_TransmitCommitHashHigh(void) {
    uint64_t message    = 0u;
    uint64_t signalData = 0u;

    /* set message data with mux value and first seven chars of commit hash */
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CANTX_VERSION_INFO_MESSAGE_MUX_START_BIT,
        CANTX_VERSION_INFO_MESSAGE_MUX_LENGTH,
        CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_COMMIT_HASH_HIGH_7,
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
        CANTX_VERSION_INFO_MESSAGE_MUX_START_BIT,
        CANTX_VERSION_INFO_MESSAGE_MUX_LENGTH,
        CANTX_VERSION_INFO_MESSAGE_MUX_VALUE_COMMIT_HASH_LOW_7,
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

static STD_RETURN_TYPE_e CANTX_VersionInfoSendMessage(uint64_t messageData) {
    /* AXIVION Routine Generic-MissingParameterAssert: messageData: parameter accept whole range */
    uint8_t data[CAN_MAX_DLC] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    CAN_TxSetCanDataWithMessageData(messageData, &data[0], CANTX_VERSION_INFO_ENDIANNESS);
    CAN_BUFFER_ELEMENT_s canBufferOfMessage = {
        .pCanNode = CAN_NODE_1,
        .id       = CANTX_VERSION_INFO_ID,
        .idType   = CANTX_VERSION_INFO_ID_TYPE,
        .data     = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};
    for (uint8_t i_data = 0u; i_data < CAN_MAX_DLC; i_data++) {
        canBufferOfMessage.data[i_data] = data[i_data];
    }
    STD_RETURN_TYPE_e successfullyQueued = CAN_SendMessage(canBufferOfMessage);
    return successfullyQueued;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e CANTX_VersionInfo(CANTX_VERSION_INFO_ACTIONS_e action) {
    STD_RETURN_TYPE_e successfullyQueued = STD_NOT_OK;
    uint64_t messageData                 = 0u;
    switch (action) {
        case CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO:
            messageData        = CANTX_TransmitBootloaderVersionInfo();
            successfullyQueued = CANTX_VersionInfoSendMessage(messageData);
            break;
        case CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_START:
            messageData        = CANTX_TransmitBootMagicStart();
            successfullyQueued = CANTX_VersionInfoSendMessage(messageData);
            break;
        case CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_END:
            messageData        = CANTX_TransmitBootMagicEnd();
            successfullyQueued = CANTX_VersionInfoSendMessage(messageData);
            break;
        case CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH:
            successfullyQueued = CANTX_TransmitCommitHash();
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break; /* LCOV_EXCL_LINE */
    }
    return successfullyQueued;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CANTX_TransmitBootloaderVersionInfo(void) {
    return CANTX_TransmitBootloaderVersionInfo();
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
extern STD_RETURN_TYPE_e TEST_CANTX_TransmitCommitHash(void) {
    return CANTX_TransmitCommitHash();
}
extern uint64_t TEST_CANTX_TransmitCommitHashLow(void) {
    return CANTX_TransmitCommitHashLow();
}
extern uint64_t TEST_CANTX_TransmitCommitHashHigh(void) {
    return CANTX_TransmitCommitHashHigh();
}
extern STD_RETURN_TYPE_e TEST_CANTX_VersionInfoSendMessage(uint64_t messageData) {
    return CANTX_VersionInfoSendMessage(messageData);
}
#endif
