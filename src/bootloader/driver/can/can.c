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
 * @file    can.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Driver for the CAN module
 * @details Implementation of the CAN interrupts, initialization, buffers,
 *          receive and transmit interfaces.
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "can.h"

#include "boot_cfg.h"
#include "can_cfg.h"

#include "HL_can.h"

#include "can_bootloader-version-info.h"
#include "can_helper.h"
#include "fassert.h"
#include "fstd_types.h"
#include "io.h"
#include "mcu.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** return value of function canGetData if no data was lost during reception */
#define CAN_HAL_RETVAL_NO_DATA_LOST (1u)

/**
 * IF2ARB register configuration
 *
 * Bits 28-0 ID: Message identifier
 *    ID[28:0] 29-bit Identifier ("Extended Frame").
 *    ID[28:18] 11-bit Identifier ("Standard Frame").
 */
/** ID shift for standard identifier */
#define CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT (18u)

/* Bit 29 Dir: Message direction
 *    0 Direction = Receive: On Tx Request, a Remote Frame with the identifier of this message object is
 *      transmitted. On receiving a Data Frame with a matching identifier, this message is stored in this
 *      message object.
 *    1 Direction = Transmit: On TxRequest, the respective message object is transmitted as a Data
 *      Frame. On receiving a Remote Frame with a matching identifier, the TxRequest bit of this message
 *      object is set (if RemoteEntry = 1).
 */
/** IF2ARB set TX direction */
#define CAN_IF2ARB_SET_TX_DIRECTION ((uint32)1u << 29u)

/* Bit 30 - Xtd: Extended identifier
 *    0 The 11-bit ("standard") identifier is used for this message object
 *    1 The 29-bit ("extended") identifier is used for this message object
 */
/** IF2ARB use standard identifier */
#define CAN_IF2ARB_USE_STANDARD_IDENTIFIER ((uint32)0u << 30u)

/** Number of bits to be shifted for each byte in an array contains n*bytes */
#define CAN_NUM_BITS_SHIFT_IN_BYTES_ARRAY (8u)

/** Number of bytes contained in the transfer info data */
#define CAN_NUM_BYTES_DATA_TRANSFER_INFO (4u)

/** Indicate whether the CAN message has been successfully written into message
 *  box:
 *    - 0: not successfully;
 *    - 1: successfully */
/**@{*/
#define CAN_WRITE_IN_TO_MESSAGE_BOX_SUCCESSFULLY     (1u)
#define CAN_WRITE_IN_TO_MESSAGE_BOX_NOT_SUCCESSFULLY (0u)
/**@}*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/** Initialize transceiver standby/enable pins */
static void CAN_InitializeTransceiver(void);

/*========== Static Function Implementations ================================*/

static void CAN_InitializeTransceiver(void) {
    /** Initialize transceiver for CAN1 */
    IO_SetPinDirectionToOutput(&CAN_CAN1_IO_REG_DIR, CAN_CAN1_ENABLE_PIN);
    IO_SetPinDirectionToOutput(&CAN_CAN1_IO_REG_DIR, CAN_CAN1_STANDBY_PIN);
    IO_PinSet(&CAN_CAN1_IO_REG_DOUT, CAN_CAN1_ENABLE_PIN);
    IO_PinSet(&CAN_CAN1_IO_REG_DOUT, CAN_CAN1_STANDBY_PIN);
}

/*========== Extern Function Implementations ================================*/
extern void CAN_Initialize(void) {
    canInit();
    /* Initialize transceiver standby/enable pins */
    CAN_InitializeTransceiver();
}

extern STD_RETURN_TYPE_e CAN_SendBootloaderInfo(void) {
    /* Prepare the can message to send */
    CAN_BUFFER_ELEMENT_s canBufferToSendFsmStates = {
        .pCanNode = can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].pCanNode,
        .id       = can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.id,
        .idType   = can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.idType,
        .data     = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};

    /* Set the variables in the message */
    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_START_BIT,
        CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_LENGTH,
        (uint64_t)can_stateOfCanCommunication,
        can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_START_BIT,
        CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_LENGTH,
        (uint64_t)boot_state,
        can_txMessages[CAN_TX_BOOTLOADER_FSM_STATES_INDEX].message.endianness);

    /* Fill the CAN buffer with signal data */
    CAN_TxSetCanDataWithMessageData(
        message,
        canBufferToSendFsmStates.data,
        can_txMessages[CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_INDEX].message.endianness);

    /* Send the can message */
    return CAN_SendMessage(canBufferToSendFsmStates);
}

extern STD_RETURN_TYPE_e CAN_SendBootloaderVersionInfo(void) {
    /* Send bootloader version info by calling function in can_bootloader-version-info.c */
    return CANTX_VersionInfo(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO);
}

extern STD_RETURN_TYPE_e CAN_SendBootloaderCommitHash(void) {
    /* Send bootloader version info by calling function in can_bootloader-version-info.c */
    return CANTX_VersionInfo(CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH);
}

extern STD_RETURN_TYPE_e CAN_SendDataTransferInfo(void) {
    /* Prepare the can message to send */
    CAN_BUFFER_ELEMENT_s canBufferToSendDataTransferInfo = {
        .pCanNode = can_txMessages[CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_INDEX].pCanNode,
        .id       = can_txMessages[CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_INDEX].message.id,
        .idType   = can_txMessages[CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_INDEX].message.idType,
        .data     = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};

    /* Set the variables in the message */
    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_START_BIT,
        CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_LENGTH,
        (uint64_t)can_infoOfDataTransfer.numOfCurrentLoop,
        can_txMessages[CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_INDEX].message.endianness);

    /* Fill the CAN buffer with signal data */
    CAN_TxSetCanDataWithMessageData(
        message,
        canBufferToSendDataTransferInfo.data,
        can_txMessages[CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_INDEX].message.endianness);

    /* Send the can message */
    return CAN_SendMessage(canBufferToSendDataTransferInfo);
}

extern STD_RETURN_TYPE_e CAN_SendAcknowledgeMessage(
    CAN_ACKNOWLEDGE_FLAG_e acknowledgeFlag,
    CAN_ACKNOWLEDGE_MESSAGE_e acknowledgeMessage,
    CAN_STATUS_CODE_e statusCode,
    CAN_YES_NO_FLAG_e response) {
    /* Prepare the can message to send*/
    CAN_BUFFER_ELEMENT_s canBufferToSend = {
        .pCanNode = can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].pCanNode,
        .id       = can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.id,
        .idType   = can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.idType,
        .data     = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};

    /* Set the variables in the message */
    uint64_t message = 0u;
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_LENGTH,
        (uint64_t)acknowledgeFlag,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_LENGTH,
        (uint64_t)acknowledgeMessage,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_LENGTH,
        (uint64_t)statusCode,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);
    CAN_TxSetMessageDataWithSignalData(
        &message,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_START_BIT,
        CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_LENGTH,
        (uint64_t)response,
        can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);

    /* Transfer the 64-bit message into the uint8_t data array */
    CAN_TxSetCanDataWithMessageData(
        message, canBufferToSend.data, can_txMessages[CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX].message.endianness);

    /* Send the can message */
    return CAN_SendMessage(canBufferToSend);
}

extern STD_RETURN_TYPE_e CAN_SendMessage(CAN_BUFFER_ELEMENT_s message) {
    FAS_ASSERT(message.pCanNode != NULL_PTR);
    FAS_ASSERT(message.pCanNode == CAN_NODE_1);
    FAS_ASSERT(message.idType == CAN_STANDARD_IDENTIFIER_11_BIT);

    STD_RETURN_TYPE_e result     = STD_NOT_OK;
    uint32_t resultOfCanTransmit = CAN_WRITE_IN_TO_MESSAGE_BOX_NOT_SUCCESSFULLY;
    /** Parse all TX message boxes until we find a free one,
     *  then use it to send the CAN message.
     *  In the HAL, message box numbers start from 1, not 0.
     */
    for (uint8_t messageBox = 1u; messageBox <= CAN_NR_OF_TX_MESSAGE_BOX; messageBox++) {
        if (canIsTxMessagePending(message.pCanNode->pCanNodeRegister, messageBox) == 0u) {
            /* id shifted by 18 to use standard frame;
             * standard frame: bits [28:18];
             * bit 29 set to 1: to set direction Tx in IF2ARB register */
            canUpdateID(
                message.pCanNode->pCanNodeRegister,
                messageBox,
                ((message.id << CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT) | CAN_IF2ARB_SET_TX_DIRECTION |
                 CAN_IF2ARB_USE_STANDARD_IDENTIFIER));
            resultOfCanTransmit = canTransmit(message.pCanNode->pCanNodeRegister, messageBox, message.data);
            if (resultOfCanTransmit == CAN_WRITE_IN_TO_MESSAGE_BOX_SUCCESSFULLY) {
                result = STD_OK;
                break;
            }
        }
    }
    return result;
}

#if !defined(UNITY_UNIT_TEST) || defined(COMPILE_FOR_UNIT_TEST)
/* in the unit test case we mock 'HL_can.h', so we have an implementation */
extern void canMessageNotification(canBASE_t *node, uint32 messageBox) {
    /* AXIVION Routine Generic-MissingParameterAssert: node: unchecked in interrupt */
    /* AXIVION Routine Generic-MissingParameterAssert: messageBox: unchecked in interrupt */
    if (messageBox > CAN_NR_OF_TX_MESSAGE_BOX) {
        uint8_t messageData[CAN_DEFAULT_DLC] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
        /* Call can api to get the can data from messageBox to ram, Possible return values:
        *   - 0: no new data; - 1: no data lost; - 3: data lost */
        uint32_t retval = canGetData(node, messageBox, (uint8 *)&messageData[0]);
        if (retval == CAN_HAL_RETVAL_NO_DATA_LOST) {
            /* Extract standard identifier from IF2ARB register*/
            uint32_t id = canGetID(node, messageBox) >> CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT;
            /* Check the call back function of the relevant function and call
             * the corresponding callback functions */
            for (uint16_t i = 0u; i < CAN_NUMBER_OF_RX_MESSAGES; i++) {
                if ((node == can_rxMessages[i].pCanNode->pCanNodeRegister) && (id == can_rxMessages[i].message.id) &&
                    (CAN_STANDARD_IDENTIFIER_11_BIT == can_rxMessages[i].message.idType)) {
                    if (can_rxMessages[i].callbackFunction != NULL_PTR) {
                        can_rxMessages[i].callbackFunction(can_rxMessages[i].message, messageData);
                    }
                }
            }
        }
    }
}
#endif

extern void CAN_SendBootMessage(void) {
    /* The magic boot sequence on CAN is:
       - <magic boot ID>:         0xFE 0xFE 0xFE 0xFE 0xFE 0xFE 0xFE 0x0F
       - <bms version info ID>:   <what ever the version is>
       - <commit hash>:           <what ever the git short hash is>
       - <magic boot ID>:         0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x0F
    */
    if (CANTX_VersionInfo(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_START) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_VersionInfo(CANTX_VERSION_INFO_TRANSMIT_BOOTLOADER_VERSION_INFO) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_VersionInfo(CANTX_VERSION_INFO_TRANSMIT_COMMIT_HASH) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_VersionInfo(CANTX_VERSION_INFO_TRANSMIT_BOOT_MAGIC_END) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
