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
 * @file    can_cbs.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   CAN driver callback implementation
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "can_cbs.h"

#include "boot_cfg.h"

#include "can.h"
#include "can_helper.h"
#include "crc.h"
#include "fassert.h"
#include "flash.h"
#include "fstring.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CAN_CBS_RETURN_SUCCESSFUL     (0u)
#define CAN_CBS_RETURN_NOT_SUCCESSFUL (1u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern uint32_t CAN_RxActionRequest(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData) {
    FAS_ASSERT(message.id == CAN_RX_REQUEST_MESSAGE_ID);
    FAS_ASSERT(message.idType == CAN_STANDARD_IDENTIFIER_11_BIT);
    FAS_ASSERT(message.dlc == CAN_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(kpkCanData != NULL);

    /* Get received CAN message from the mailbox */
    uint64_t messageData = 0u;
    CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

    /* Extract the variable 'BootloaderAction' from the received can message */
    uint64_t canSignal = 0u;
    CAN_RxGetSignalDataFromMessageData(
        messageData,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT,
        CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH,
        &canSignal,
        message.endianness);

    /* AXIVION Next Codeline Style MisraC2012Directive-4.1, MisraC2012-10.5:
    it is indented to convert 64-bits can signal into 8-bits and then convert
    it into the request code in 8-bits */
    CAN_REQUEST_CODE_e bootloaderAction = (CAN_REQUEST_CODE_e)((uint8_t)canSignal);

    switch (bootloaderAction) {
        case CMD_TO_TRANSFER_PROGRAM:
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * Return value of function discarded because it will not be evaluated.
             * The reason not to evaluate the value is to keep the system simple.
             * If the host has dropped one can message, it will request again.
             * And if the host has dropped all messages, that means there are some
             * problems with the can connection, it will show error status at the
             * host side */
            CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, NO);

            if (can_stateOfCanCommunication == CAN_FSM_STATE_NO_COMMUNICATION) {
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * Return value of function is discarded because it will not be evaluated,
                 * more details see the first comment */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_AND_IN_PROCESSING, NO);

                /* Change the can communication state */
                can_stateOfCanCommunication = CAN_FSM_STATE_WAIT_FOR_INFO;

                /* Send ACK message */
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * Return value of function is discarded because it will not be evaluated,
                 * more details see the first comment */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_TRANSFER_PROGRAM, RECEIVED_AND_PROCESSED, NO);
            }
            break;

        case CMD_TO_RUN_PROGRAM:
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * Return value of function is discarded because it will not be evaluated,
             * more details see the first comment */
            CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_BUT_NOT_PROCESSED, NO);

            if (can_stateOfCanCommunication == CAN_FSM_STATE_NO_COMMUNICATION) {
                /* Change the can communication state */
                can_stateOfCanCommunication = CAN_FSM_STATE_RUN_PROGRAM;

                /* Send acknowledge messages */
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * Return value of function is discarded because it will not be evaluated,
                 * more details see the first comment */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_IN_PROCESSING, NO);
            }
            break;

        case CMD_TO_RESET_BOOT_PROCESS:
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * Return value of function is discarded because it will not be evaluated,
             * more details see the first comment */
            CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_RESET_BOOT_PROCESS, RECEIVED_BUT_NOT_PROCESSED, NO);

            /* Change the can communication state */
            can_stateOfCanCommunication = CAN_FSM_STATE_RESET_BOOT;

            /* Send acknowledge messages */
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * Return value of function is discarded because it will not be evaluated,
             * more details see the first comment */
            CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_RESET_BOOT_PROCESS, RECEIVED_AND_IN_PROCESSING, NO);

            break;

        case CMD_TO_GET_BOOTLOADER_INFO:
            /* Send bootloader info (fsm states) */
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * Return value of function is discarded because it will not be evaluated,
             * more details see the first comment */
            CAN_SendBootloaderInfo();

            break;

        case CMD_TO_GET_DATA_TRANSFER_INFO:
            /* Send data transfer info (the current data transfer loop number) */
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * Return value of function is discarded because it will not be evaluated,
             * more details see the first comment */
            CAN_SendDataTransferInfo();

            break;

        case CMD_TO_GET_VERSION_INFO:
            /* Send bootloader version info and commit hash */
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * Return value of function is discarded because it will not be evaluated,
             * more details see the first comment */
            CAN_SendBootloaderVersionInfo();
            CAN_SendBootloaderCommitHash();

            break;

        default:
            /* If the request code has not been registered */
            break;
    }
    return 0u;
}

extern uint32_t CAN_RxCrc8Bytes(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData) {
    FAS_ASSERT(message.id == CAN_RX_CRC_8_BYTES_ID);
    FAS_ASSERT(message.idType == CAN_STANDARD_IDENTIFIER_11_BIT);
    FAS_ASSERT(message.dlc == CAN_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(kpkCanData != NULL);

    uint32_t retVal = CAN_CBS_RETURN_SUCCESSFUL;
    bool gotoNext   = true;

    /* Entry protection: the received data will only be processed if the current
    state is one of the required */
    if ((can_stateOfCanCommunication != CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE) &&
        (can_stateOfCanCommunication != CAN_FSM_STATE_RECEIVED_8_BYTES_DATA)) {
        retVal   = CAN_CBS_RETURN_NOT_SUCCESSFUL;
        gotoNext = false;
    }

    if (gotoNext) {
        /* Get received can message from mailbox */
        uint64_t messageData = 0u;
        CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

        /* Extract the variable - crc8Bytes */
        uint64_t canSignal = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT,
            CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH,
            &canSignal,
            message.endianness);
        uint64_t crc8Bytes = canSignal;

        /* Validate vector table */
        if (can_stateOfCanCommunication == CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE) {
            can_infoOfDataTransfer.crc64OfVectorTable = crc8Bytes;
            uint64_t crc64OfVectorTableOnBoard        = 0u;
            crc64OfVectorTableOnBoard                 = CRC_CalculateCrc64(
                (const uint64_t *)can_infoOfDataTransfer.vectorTable, BOOT_NUM_OF_VECTOR_TABLE_8_BYTES, 0);

            if (can_infoOfDataTransfer.crc64OfVectorTable == crc64OfVectorTableOnBoard) {
                can_stateOfCanCommunication = CAN_FSM_STATE_VALIDATED_VECTOR_TABLE;
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * Return value of function is discarded because it will not be evaluated,
                 * more details see the first comment */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CRC_OF_VECTOR_TABLE, RECEIVED_AND_PROCESSED, YES);
            } else {
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * Return value of function is discarded because it will not be evaluated,
                 * more details see the first comment */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CRC_OF_VECTOR_TABLE, RECEIVED_AND_PROCESSED, NO);

                /* Reset vector table relevant variables in can_infoOfDataTransfer */
                CAN_ResetVectorTableRelevantVariables();

                /* Since this vector table can not be validated, go back to continue to wait for vector table */
                can_stateOfCanCommunication = CAN_FSM_STATE_FINISHED_FINAL_VALIDATION;
            }
        }

        if (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_8_BYTES_DATA) {
            /* Get boolean conditions for different cases */
            /* Disable IRQ interrupt before every function that will be run from RAM */
            _disable_IRQ_interrupt_();
            FLASH_FLASH_SECTOR_s currentSector =
                FLASH_GetFlashSector((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8);
            _enable_IRQ_interrupt_();
            uint32_t lenOfCurrentSector = (uint32_t)currentSector.pU8SectorAddressEnd -
                                          (uint32_t)currentSector.pU32SectorAddressStart + 1u;
            bool isFinishedTransferCurrentSector = (can_infoOfDataTransfer.sectorBufferCurrentAddressU8 -
                                                    BOOT_SECTOR_BUFFER_START_ADDRESS) >= lenOfCurrentSector;
            bool isThisTheLastDataLoop =
                (can_infoOfDataTransfer.numOfCurrentLoop == can_infoOfDataTransfer.totalNumOfDataTransferLoops);

            /* Check the can state */
            if (isFinishedTransferCurrentSector || isThisTheLastDataLoop) {
                /* Update the can_infoOfDataTransfer */
                can_infoOfDataTransfer.programCrc8Bytes = crc8Bytes;
                can_stateOfCanCommunication             = CAN_FSM_STATE_RECEIVED_8_BYTES_CRC;
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * Return value of function is discarded because it will not be evaluated,
                 * more details see the first comment */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_IN_PROCESSING, NO);
            }
        }
    }

    return retVal;
}

extern uint32_t CAN_RxData8Bytes(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData) {
    FAS_ASSERT(message.id == CAN_RX_DATA_8_BYTES_ID);
    FAS_ASSERT(message.idType == CAN_STANDARD_IDENTIFIER_11_BIT);
    FAS_ASSERT(message.dlc == CAN_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(kpkCanData != NULL);

    uint32_t retVal = CAN_CBS_RETURN_SUCCESSFUL;
    bool gotoNext   = true;

    /* Entry protection: the received data will only be processed if the current
    state is one of the required */
    if ((can_stateOfCanCommunication != CAN_FSM_STATE_FINISHED_FINAL_VALIDATION) &&
        (can_stateOfCanCommunication != CAN_FSM_STATE_RECEIVED_LOOP_NUMBER)) {
        retVal   = CAN_CBS_RETURN_NOT_SUCCESSFUL;
        gotoNext = false;
    }

    if (gotoNext) {
        /* Get received can message from mailbox */
        uint64_t messageData = 0u;
        CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

        /* Extract the variable - data8Bytes */
        uint64_t canSignal = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT,
            CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH,
            &canSignal,
            message.endianness);
        uint64_t data8Bytes = canSignal;

        /* Transfer the the vector table */
        if (can_stateOfCanCommunication == CAN_FSM_STATE_FINISHED_FINAL_VALIDATION) {
            can_infoOfDataTransfer.vectorTable[can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes] =
                data8Bytes;
            can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes++;
            if (can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes == BOOT_NUM_OF_VECTOR_TABLE_8_BYTES) {
                can_stateOfCanCommunication = CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE;
                /* Send the acknowledge message */
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * Return value of function is discarded because it will not be evaluated,
                 * more details see the first comment */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_VECTOR_TABLE, RECEIVED_AND_PROCESSED, NO);
            }
        }

        /* Transfer the program (except for vector table) */
        if (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_LOOP_NUMBER) {
            /* Write the 8 byte data to sector buffer */
            /* It triggers error if the sector buffer current address is not in its supposed range */
            if ((can_infoOfDataTransfer.sectorBufferCurrentAddressU8 >
                 (BOOT_SECTOR_BUFFER_END_ADDRESS - BOOT_NUM_OF_BYTES_IN_64_BITS + 1u)) ||
                (can_infoOfDataTransfer.sectorBufferCurrentAddressU8 < BOOT_SECTOR_BUFFER_START_ADDRESS)) {
                can_stateOfCanCommunication = CAN_FSM_STATE_ERROR;
                retVal                      = CAN_CBS_RETURN_NOT_SUCCESSFUL;
                gotoNext                    = false;
            }

            if (gotoNext) {
                memcpy(
                    (uint8_t *)can_infoOfDataTransfer.sectorBufferCurrentAddressU8,
                    &data8Bytes,
                    BOOT_NUM_OF_BYTES_IN_64_BITS);

                /* Update can status */
                can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_8_BYTES_DATA;

                /* TODO: Do hardware test for the following fix */
                if (can_infoOfDataTransfer.numOfCurrentLoop == can_infoOfDataTransfer.totalNumOfDataTransferLoops) {
                    /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                     * Return value of function is discarded because it will not be evaluated,
                     * more details see the first comment */
                    CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_SUB_SECTOR_DATA, RECEIVED_AND_PROCESSED, YES);
                } else {
                    /* Update the sector buffer address (RAM area where the received
                    data will be temporarily stored) for the next data loop */
                    can_infoOfDataTransfer.sectorBufferCurrentAddressU8 += BOOT_NUM_OF_BYTES_IN_64_BITS;
                    /* Update the flash address where the data received in the next
                    data loop will be written to (via buffer) */
                    can_infoOfDataTransfer.programCurrentAddressU8 += CAN_DATA_SEGMENT_LEN_IN_A_LOOP_BYTES;

                    /* Reply at the last data loop of every sub sector */
                    if ((can_infoOfDataTransfer.numOfCurrentLoop % BOOT_NUM_OF_LOOPS_IN_ONE_SUB_SECTOR) == 0u) {
                        /* Update the loop number */
                        can_infoOfDataTransfer.numOfCurrentLoop++;
                        /* If possible, always put the CAN_SendAcknowledgeMessage at the end
                         * to prevent unmatched reality between the host and the microcontroller
                         */
                        /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                         * Return value of function is discarded because it will not be evaluated,
                         * more details see the first comment */
                        CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_SUB_SECTOR_DATA, RECEIVED_AND_PROCESSED, NO);
                    } else {
                        /* Update the loop number */
                        can_infoOfDataTransfer.numOfCurrentLoop++;
                    }
                }
            }
        }
    }

    return retVal;
}

extern uint32_t CAN_RxTransferProcessInfo(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData) {
    FAS_ASSERT(message.id == CAN_RX_TRANSFER_PROCESS_INFO_ID);
    FAS_ASSERT(message.idType == CAN_STANDARD_IDENTIFIER_11_BIT);
    FAS_ASSERT(message.dlc == CAN_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(kpkCanData != NULL);

    uint32_t retVal = CAN_CBS_RETURN_SUCCESSFUL;
    bool gotoNext   = true;

    /* Send can message to indicate that this callback function has been entered */
    /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
     * Return value of function is discarded because it will not be evaluated,
     * more details see the first comment */
    CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_BUT_NOT_PROCESSED, NO);

    /* Entry protection: the received data will only be processed if the current
    state is one of the required */
    if ((can_stateOfCanCommunication != CAN_FSM_STATE_WAIT_FOR_INFO)) {
        retVal   = CAN_CBS_RETURN_NOT_SUCCESSFUL;
        gotoNext = false;
    }

    if (gotoNext) {
        /* Signal the sender that this message is received and will be processed */
        /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
        * Return value of function is discarded because it will not be evaluated,
        * more details see the first comment */
        CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_IN_PROCESSING, NO);

        /* Get received can message from mailbox */
        uint64_t messageData = 0u;
        CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);

        /* Extract the variable - lenOfProgram */
        uint64_t canSignal = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_START_BIT,
            CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_LENGTH,
            &canSignal,
            message.endianness);

        /* AXIVION Next Codeline Style MisraC2012Directive-4.1: this cast is supposed to
            only extract one part of canSignal */
        uint32_t lenOfProgramInBytes = (uint32_t)canSignal;

        /* Extract the variable - numOfTransferLoops */
        canSignal = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_START_BIT,
            CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_LENGTH,
            &canSignal,
            message.endianness);

        /* AXIVION Next Codeline Style MisraC2012Directive-4.1: this cast is supposed to
            only extract one part of canSignal */
        uint32_t numOfTransferLoops = (uint32_t)canSignal;

        /* Check if the programLengthInBytes is in range */
        if ((lenOfProgramInBytes > BOOT_PROGRAM_SIZE_MAX) ||
            ((uint32_t)(lenOfProgramInBytes / BOOT_NUM_OF_BYTES_IN_ONE_DATA_TRANSFER_LOOP) != numOfTransferLoops)) {
            /* Send the acknowledge message, the can fsm state will not be updated */
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
            * Return value of function is discarded because it will not be evaluated,
            * more details see the first comment */
            CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_PROCESSED, NO);
        } else {
            /* Write the extracted values to the global variable */
            can_infoOfDataTransfer.totalNumOfDataTransferLoops = numOfTransferLoops;
            can_infoOfDataTransfer.programLengthInBytes        = lenOfProgramInBytes;
            /* Update the can communication state */
            can_stateOfCanCommunication = CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS;

            /* Send the acknowledge message */
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
            * Return value of function is discarded because it will not be evaluated,
            * more details see the first comment */
            CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_PROGRAM_INFO, RECEIVED_AND_PROCESSED, YES);
        }
    }

    return retVal;
}

extern uint32_t CAN_RxLoopInfo(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData) {
    FAS_ASSERT(message.id == CAN_RX_LOOP_INFO_ID);
    FAS_ASSERT(message.idType == CAN_STANDARD_IDENTIFIER_11_BIT);
    FAS_ASSERT(message.dlc == CAN_DEFAULT_DLC);
    FAS_ASSERT(message.endianness == CAN_LITTLE_ENDIAN);
    FAS_ASSERT(kpkCanData != NULL);

    uint32_t retVal = CAN_CBS_RETURN_SUCCESSFUL;
    bool gotoNext   = true;

    /* Entry protection: the received data will only be processed if the current
    state is one of the required */
    if ((can_stateOfCanCommunication != CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS) &&
        (can_stateOfCanCommunication != CAN_FSM_STATE_RECEIVED_LOOP_NUMBER) &&
        (can_stateOfCanCommunication != CAN_FSM_STATE_RECEIVED_8_BYTES_DATA) &&
        (can_stateOfCanCommunication != CAN_FSM_STATE_RECEIVED_8_BYTES_CRC)) {
        retVal   = CAN_CBS_RETURN_NOT_SUCCESSFUL;
        gotoNext = false;
    }

    if (gotoNext) {
        /* Extract the variable - numOfCurrentLoop */
        uint64_t messageData = 0u;
        CAN_RxGetMessageDataFromCanData(&messageData, kpkCanData, message.endianness);
        uint64_t canSignal = 0u;
        CAN_RxGetSignalDataFromMessageData(
            messageData,
            CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT,
            CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH,
            &canSignal,
            message.endianness);

        /* AXIVION Next Codeline Style MisraC2012Directive-4.1: this cast is supposed to
        only extract one part of canSignal */
        uint32_t numOfLoops = (uint32_t)canSignal;

        /* Initialize the number of current loop at the start of data transfer if it has not been initialized */
        /* Todo: fix this hidden bug: if the current can fsm state is not one of the states that it could be and
        meanwhile can_infoOfDataTransfer.numOfCurrentLoop has not been initialized, than it will be initialzed in
        a wrong case.  */
        if (can_infoOfDataTransfer.numOfCurrentLoop == 0u) {
            can_infoOfDataTransfer.numOfCurrentLoop = 1u;
        }

        /* Boolean conditions for different cases */
        bool isToRepeatThisLoop            = (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_LOOP_NUMBER);
        bool isThisLoopTheFirstInDataLoops = (can_infoOfDataTransfer.numOfCurrentLoop == 1u) &&
                                             (can_stateOfCanCommunication == CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS);
        bool isJustFinishedValidationStep    = (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_8_BYTES_CRC);
        bool isThisLoopTheFirstInSectorLoops = (isThisLoopTheFirstInDataLoops || isJustFinishedValidationStep);
        bool isEveryThingNormal              = (can_stateOfCanCommunication == CAN_FSM_STATE_RECEIVED_8_BYTES_DATA);
        /* Clean up the sector buffer */
        bool isCanStateWished = (isToRepeatThisLoop || isThisLoopTheFirstInSectorLoops || isEveryThingNormal);

        /* The can fsm state will only be updated if the number of the current loop is the one
        that it should be, meanwhile the current case should be one of the planned. */
        if ((can_infoOfDataTransfer.numOfCurrentLoop == numOfLoops) && isCanStateWished) {
            if (isThisLoopTheFirstInSectorLoops) {
                /* Save the state and variables before the transfer of the new sector */
                CAN_CopyCanDataTransferInfo(&can_infoOfDataTransfer, &can_copyOfInfoOfDataTransfer);

                /* Clean up the sector buffer */
                memset((uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS, 0, BOOT_SECTOR_BUFFER_SIZE);
            }
            can_stateOfCanCommunication = CAN_FSM_STATE_RECEIVED_LOOP_NUMBER;
        }
    }

    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
