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
 * @file    can_cfg.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  CAN
 *
 * @brief   Configuration for the CAN module
 * @details The CAN bus settings and the received messages and their
 *          reception handling are to be specified here.
 */

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include "can_cbs.h"
#include "fassert.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
const CAN_NODE_s can_node1 = {
    .pCanNodeRegister = canREG1,
};

/** Registry of CAN RX messages */
const CAN_RX_MESSAGE_TYPE_s can_rxMessages[CAN_NUMBER_OF_RX_MESSAGES] = {
    {CAN_NODE_1,
     {.id         = CAN_RX_REQUEST_MESSAGE_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxActionRequest},
    {CAN_NODE_1,
     {.id         = CAN_RX_TRANSFER_PROCESS_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxTransferProcessInfo},
    {CAN_NODE_1,
     {.id         = CAN_RX_LOOP_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxLoopInfo},
    {CAN_NODE_1,
     {.id         = CAN_RX_DATA_8_BYTES_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxData8Bytes},
    {CAN_NODE_1,
     {.id         = CAN_RX_CRC_8_BYTES_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = &CAN_RxCrc8Bytes},
};

/** Registry of CAN TX messages */
const CAN_TX_MESSAGE_TYPE_s can_txMessages[CAN_NUMBER_OF_TX_MESSAGES] = {
    {CAN_NODE_1,
     {.id         = CAN_TX_ACKNOWLEDGE_MESSAGE_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = NULL_PTR,
     .pMuxId           = NULL_PTR},
    {CAN_NODE_1,
     {.id         = CAN_TX_BOOTLOADER_FSM_STATES_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = NULL_PTR,
     .pMuxId           = NULL_PTR},
    {CAN_NODE_1,
     {.id         = CAN_TX_DATA_TRANSFER_INFO_ID,
      .idType     = CAN_STANDARD_IDENTIFIER_11_BIT,
      .dlc        = CAN_DEFAULT_DLC,
      .endianness = CAN_LITTLE_ENDIAN},
     .callbackFunction = NULL_PTR,
     .pMuxId           = NULL_PTR},
};

/** Initialize the state machine of CAN module */
CAN_FSM_STATES_e can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

/** CAN data transfer information */
CAN_DATA_TRANSFER_INFO_s can_infoOfDataTransfer = {
    .totalNumOfDataTransferLoops           = 0u,
    .numOfCurrentLoop                      = 0u,
    .programLengthInBytes                  = 0u,
    .programStartAddressU8                 = BOOT_PROGRAM_START_ADDRESS,
    .programCurrentAddressU8               = BOOT_PROGRAM_START_ADDRESS,
    .programCurrentSectorAddressU8         = BOOT_PROGRAM_START_ADDRESS,
    .programCrc8Bytes                      = 0u,
    .programCrc8BytesOnBoard               = 0u,
    .sectorBufferCurrentAddressU8          = BOOT_SECTOR_BUFFER_START_ADDRESS,
    .numOfReceivedVectorTableDataIn64Bytes = 0u,
    .vectorTable                           = {0u, 0u, 0u, 0u},
    .crc64OfVectorTable                    = 0u,
};

/** Copy of can data transfer */
CAN_DATA_TRANSFER_INFO_s can_copyOfInfoOfDataTransfer = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, {0u, 0u, 0u, 0u}, 0u};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void CAN_ResetVectorTableRelevantVariables(void) {
    /* Reset all vector table relevant CAN FSM state and variables */
    for (uint8_t iVector = 0u; iVector < CAN_VECTOR_TABLE_LENGTH; iVector++) {
        can_infoOfDataTransfer.vectorTable[iVector] = 0u;
    }
    can_infoOfDataTransfer.numOfReceivedVectorTableDataIn64Bytes = 0u;
    can_infoOfDataTransfer.crc64OfVectorTable                    = 0u;
}

extern void CAN_ResetCanCommunication(void) {
    /* Reset the state of CAN FSM */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    /* Reset all members of can_infoOfDataTransfer */
    can_infoOfDataTransfer.totalNumOfDataTransferLoops   = 0u;
    can_infoOfDataTransfer.numOfCurrentLoop              = 0u;
    can_infoOfDataTransfer.programLengthInBytes          = 0u;
    can_infoOfDataTransfer.programStartAddressU8         = BOOT_PROGRAM_START_ADDRESS;
    can_infoOfDataTransfer.programCurrentAddressU8       = BOOT_PROGRAM_START_ADDRESS;
    can_infoOfDataTransfer.programCurrentSectorAddressU8 = BOOT_PROGRAM_START_ADDRESS;
    can_infoOfDataTransfer.programCrc8Bytes              = 0u;
    can_infoOfDataTransfer.programCrc8BytesOnBoard       = 0u;
    can_infoOfDataTransfer.sectorBufferCurrentAddressU8  = BOOT_SECTOR_BUFFER_START_ADDRESS;
    CAN_ResetVectorTableRelevantVariables();
}

extern void CAN_CopyCanDataTransferInfo(
    const CAN_DATA_TRANSFER_INFO_s *pkOriginalTransferInfo,
    CAN_DATA_TRANSFER_INFO_s *pCopyOfTransferInfo) {
    FAS_ASSERT(pkOriginalTransferInfo != NULL_PTR);
    FAS_ASSERT(pCopyOfTransferInfo != NULL_PTR);

    /* Copy the variables that provide the general transfer information */
    pCopyOfTransferInfo->totalNumOfDataTransferLoops   = pkOriginalTransferInfo->totalNumOfDataTransferLoops;
    pCopyOfTransferInfo->numOfCurrentLoop              = pkOriginalTransferInfo->numOfCurrentLoop;
    pCopyOfTransferInfo->programLengthInBytes          = pkOriginalTransferInfo->programLengthInBytes;
    pCopyOfTransferInfo->programStartAddressU8         = pkOriginalTransferInfo->programStartAddressU8;
    pCopyOfTransferInfo->programCurrentAddressU8       = pkOriginalTransferInfo->programCurrentAddressU8;
    pCopyOfTransferInfo->programCurrentSectorAddressU8 = pkOriginalTransferInfo->programCurrentSectorAddressU8;
    pCopyOfTransferInfo->programCrc8Bytes              = pkOriginalTransferInfo->programCrc8Bytes;
    pCopyOfTransferInfo->programCrc8BytesOnBoard       = pkOriginalTransferInfo->programCrc8BytesOnBoard;
    pCopyOfTransferInfo->sectorBufferCurrentAddressU8  = pkOriginalTransferInfo->sectorBufferCurrentAddressU8;
    pCopyOfTransferInfo->numOfReceivedVectorTableDataIn64Bytes =
        pkOriginalTransferInfo->numOfReceivedVectorTableDataIn64Bytes;

    /* Copy the variables that provide the vector table */
    for (uint8_t i_vector = 0u; i_vector < CAN_VECTOR_TABLE_LENGTH; i_vector++) {
        pCopyOfTransferInfo->vectorTable[i_vector] = pkOriginalTransferInfo->vectorTable[i_vector];
    }

    /* Copy the variables that provide the CRC signature of the vector table */
    pCopyOfTransferInfo->crc64OfVectorTable = pkOriginalTransferInfo->crc64OfVectorTable;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
