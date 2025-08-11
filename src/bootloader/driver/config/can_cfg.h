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
 * @file    can_cfg.h
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Headers for the configuration for the CAN module
 * @details The activation and the length of the message buffers as well as the
 *          number of the messages that are received are to be configured here.
 *
 */

#ifndef FOXBMS__CAN_CFG_H_
#define FOXBMS__CAN_CFG_H_

/*========== Includes =======================================================*/

#include "boot_cfg.h"

#include "HL_can.h"
#include "HL_reg_het.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** CAN node configuration */
typedef struct {
    canBASE_t *pCanNodeRegister; /*!< register the connected CAN interface */
} CAN_NODE_s;

/** CAN node 1 defines */
#define CAN_NODE_1 ((CAN_NODE_s *)&can_node1)

/**
 * Configuration of CAN transceiver pins to the respective port expander pins.
 * @{
 */
/** IO register to which the CAN1 is connected */
#define CAN_CAN1_IO_REG_DIR  (hetREG2->DIR)
#define CAN_CAN1_IO_REG_DOUT (hetREG2->DOUT)
#define CAN_CAN1_ENABLE_PIN  (18u)
#define CAN_CAN1_STANDBY_PIN (23u)
/**@}*/

/** Start bit variables in f_BootloaderAcknowledgeMessage */
/**@{*/
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_START_BIT    (0u)
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_FLAG_LENGTH       (1u)
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_START_BIT (1u)
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_ACKNOWLEDGE_MESSAGE_LENGTH    (4u)
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_START_BIT         (5u)
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_STATUS_CODE_LENGTH            (2u)
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_START_BIT       (7u)
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_YES_NO_ANSWER_LENGTH          (1u)
/**@}*/

/** Start bit and length of variables in f_BootloaderActionRequest (400h) */
/**@{*/
#define CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_START_BIT (0u)
#define CAN_RX_BOOTLOADER_ACTION_REQUEST_BOOTLOADER_ACTION_LENGTH    (8u)
/**@}*/

/** Start bit and length of variables in f_BootloaderCrc8Bytes (460h) */
/**@{*/
#define CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_START_BIT (0u)
#define CAN_RX_BOOTLOADER_CRC_8_BYTES_CRC_LENGTH    (64u)
/**@}*/

/** Start bit and length of variables in f_BootloaderData8Bytes (440h) */
/**@{*/
#define CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_START_BIT (0u)
#define CAN_RX_BOOTLOADER_DATA_8_BYTES_DATA_LENGTH    (64u)
/**@}*/

/** The start bit and length of variables in f_BootloaderDataTransferInfo (490h) */
/**@{*/
#define CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_START_BIT (0u)
#define CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_CURRENT_LOOP_NUMBER_LENGTH    (32u)
/**@}*/

/** The start bit and length of variables in f_BootloaderFsmStates (495h) */
/**@{*/
#define CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_START_BIT  (0u)
#define CAN_TX_BOOTLOADER_FSM_STATES_CAN_FSM_STATE_LENGTH     (8u)
#define CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_START_BIT (8u)
#define CAN_TX_BOOTLOADER_FSM_STATES_BOOT_FSM_STATE_LENGTH    (8u)
/**@}*/

/** Start bit and length of variables in f_BootloaderLoopInfo (430h) */
/**@{*/
#define CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_START_BIT (0u)
#define CAN_RX_BOOTLOADER_LOOP_INFO_LOOP_NUMBER_LENGTH    (32u)
/**@}*/

/** Start bit and length of variables in f_BootloaderTransferProcessInfo (420h) */
/**@{*/
#define CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_START_BIT          (0u)
#define CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_PROGRAM_LENGTH_LENGTH             (32u)
#define CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_START_BIT (32u)
#define CAN_RX_BOOTLOADER_TRANSFER_PROCESS_INFO_REQUIRED_TRANSFER_LOOPS_LENGTH    (32u)
/**@}*/

/** The index of TX messages */
/**@{*/
#define CAN_TX_BOOTLOADER_ACKNOWLEDGE_MESSAGE_INDEX (0u)
#define CAN_TX_BOOTLOADER_FSM_STATES_INDEX          (1u)
#define CAN_TX_BOOTLOADER_DATA_TRANSFER_INFO_INDEX  (2u)
/**@}*/

/** The CAN ID of RX messages */
/**@{*/
#define CAN_RX_REQUEST_MESSAGE_ID       (0x400u)
#define CAN_RX_TRANSFER_PROCESS_INFO_ID (0x420u)
#define CAN_RX_LOOP_INFO_ID             (0x430u)
#define CAN_RX_DATA_8_BYTES_ID          (0x440u)
#define CAN_RX_CRC_8_BYTES_ID           (0x460u)
/**@}*/

/** The CAN ID of TX messages */
/**@{*/
#define CAN_TX_ACKNOWLEDGE_MESSAGE_ID   (0x480u)
#define CAN_TX_BOOTLOADER_FSM_STATES_ID (0x495u)
#define CAN_TX_DATA_TRANSFER_INFO_ID    (0x490u)
/**@}*/

/** Macros for all CAN files */
/**@{*/
#define CAN_DATA_SEGMENT_LEN_IN_A_LOOP_BYTES (BOOT_NUM_OF_BYTES_IN_64_BITS)
#define CAN_VECTOR_TABLE_LENGTH              (4u)
/**@}*/

/** Number of transmit messages */
#define CAN_NUMBER_OF_TX_MESSAGES (3u)

/** Number of receive messages */
#define CAN_NUMBER_OF_RX_MESSAGES (5u)

/* ****************************************************************************
 *  CAN NODE OPTIONS
 *****************************************************************************/
/** Maximum length of Data Length Code */
#define CAN_MAX_DLC (8u)

/** Default DLC */
#define CAN_DEFAULT_DLC (CAN_MAX_DLC)

/* **************************************************************************************
 *  CAN BUFFER OPTIONS
 *****************************************************************************************/

/** Enum for byte order (endianness)
 * \verbatim
 * CAN data example:
 *
 * LittleEndian
 * bitStart = 27; bitLength = 19
 * DataLE: 45-44-43-42-41-40-39-38-37-36-35-34-33-32-31-30-29-28-27
 *         MSB                                                   LSB
 *
 * BigEndian
 * bitStart = 21; bitLength = 19
 * DataBE: 21-20-19-18-17-16-31-30-29-28-27-26-25-24-39-38-37-36-35
 *         MSB                                                   LSB
 *                                             |||
 *                                   Receive data on CAN bus
 *                                             |||
 *                                             |||
 *                                             \_/
 *                  LE                          |                  BE
 * CAN Data Byte 0  07 06 05 04 03 02 01 00     | CAN Data Byte 0  07 06 05 04 03 02 01 00
 * CAN Data Byte 1  15 14 13 12 11 10 09 08     | CAN Data Byte 1  15 14 13 12 11 10 09 08
 * CAN Data Byte 2  23 22 21 20 19 18 17 16     | CAN Data Byte 2  23 22 21-20-19-18-17-16 MSB
 * CAN Data Byte 3  31-30-29-28-27 26 25 24 LSB | CAN Data Byte 3  31-30-29-28-27-26-25-24
 * CAN Data Byte 4  39-38-37-36-35-34-33-32     | CAN Data Byte 4  39-38-37-36-35 34 33 32 LSB
 * CAN Data Byte 5  47 46 45-44-43-42-41-40 MSB | CAN Data Byte 5  47 46 45 44 43 42 41 40
 * CAN Data Byte 6  55 54 53 52 51 50 49 48     | CAN Data Byte 6  55 54 53 52 51 50 49 48
 * CAN Data Byte 7  63 62 61 60 59 58 57 56     | CAN Data Byte 7  63 62 61 60 59 58 57 56
 *                                             |||
 *                                  Store received data in RAM
 *                                             |||
 *                                             |||
 *                                             \_/
 *                  LE                          |                  BE
 * CAN Data Byte 7  63 62 61 60 59 58 57 56     | CAN Data Byte 0  07 06 05 04 03 02 01 00     | RAM data[7]
 * CAN Data Byte 6  55 54 53 52 51 50 49 48     | CAN Data Byte 1  15 14 13 12 11 10 09 08     | RAM data[6]
 * CAN Data Byte 5  47 46 45-44-43-42-41-40 MSB | CAN Data Byte 2  23 22 21-20-19-18-17-16 MSB | RAM data[5]
 * CAN Data Byte 4  39-38-37-36-35-34-33-32     | CAN Data Byte 3  31-30-29-28-27-26-25-24     | RAM data[4]
 * CAN Data Byte 3  31-30-29-28-27 26 25 24 LSB | CAN Data Byte 4  39-38-37-36-35 34 33 32 LSB | RAM data[3]
 * CAN Data Byte 2  23 22 21 20 19 18 17 16     | CAN Data Byte 5  47 46 45 44 43 42 41 40     | RAM data[2]
 * CAN Data Byte 1  15 14 13 12 11 10 09 08     | CAN Data Byte 6  55 54 53 52 51 50 49 48     | RAM data[1]
 * CAN Data Byte 0  07 06 05 04 03 02 01 00     | CAN Data Byte 7  63 62 61 60 59 58 57 56     | RAM data[0]
 *                                      DataLE  =  DataBE
 * \endverbatim
 */
typedef enum {
    CAN_LITTLE_ENDIAN,
    CAN_BIG_ENDIAN,
} CAN_ENDIANNESS_e;

/** Enum indicating if a former request has been received or not. */
typedef enum {
    NOT_RECEIVED = 0u,
    RECEIVED     = 1u,
} CAN_ACKNOWLEDGE_FLAG_e;

/** Enum representing a yes or no answer for a specified question. */
typedef enum {
    NO  = 0u,
    YES = 1u,
} CAN_YES_NO_FLAG_e;

/** Enum representing the status of the processing of the relevant request. */
typedef enum {
    RECEIVED_BUT_NOT_PROCESSED =
        0u, /*!< inform that this message has been received and the call back functions have been called */
    RECEIVED_AND_IN_PROCESSING = 1u, /*!< inform that this message can be processed */
    RECEIVED_AND_PROCESSED     = 2u, /*!< inform that this message has been processed */
    ERROR                      = 3u, /*!< inform that there is error happening inside the board */
} CAN_STATUS_CODE_e;

/**
 * Command message type that the bootloader has received.
 * For example, PC send a command message to bootloader to let it be prepared
 * to receive the program binaries, upon receiving this command message, the
 * bootloader will reply an ACK message with one field contains information
 * about the command type it has received.
 * - RECEIVED_CMD_TO_TRANSFER_PROGRAM: Bootloader has received the command to
 *   let it be prepared to received the following program binaries (transfer
 *   the program).
 * - RECEIVED_PROGRAM_INFO: Bootloader has received the information of the
 *   program (foxBMS app).
 * - RECEIVED_LOOP_NUMBER: Bootloader has received the current loop number.
 * - RECEIVED_SUB_SECTOR_DATA: Bootloader has received the current sector data.
 */
typedef enum {
    RECEIVED_CMD_TO_TRANSFER_PROGRAM     = 1u,
    RECEIVED_PROGRAM_INFO                = 2u,
    RECEIVED_LOOP_NUMBER                 = 3u,
    RECEIVED_SUB_SECTOR_DATA             = 4u,
    RECEIVED_8_BYTES_CRC                 = 5u,
    RECEIVED_FINAL_8_BYTES_CRC_SIGNATURE = 6u,
    RECEIVED_VECTOR_TABLE                = 7u,
    RECEIVED_CRC_OF_VECTOR_TABLE         = 8u,
    RECEIVED_CMD_TO_RUN_PROGRAM          = 9u,
    RECEIVED_CMD_TO_RESET_BOOT_PROCESS   = 10u,
} CAN_ACKNOWLEDGE_MESSAGE_e;

/** Command request codes */
typedef enum {
    CMD_TO_TRANSFER_PROGRAM       = 1u,
    CMD_TO_RESET_BOOT_PROCESS     = 2u,
    CMD_TO_RUN_PROGRAM            = 3u,
    CMD_TO_GET_BOOTLOADER_INFO    = 4u,
    CMD_TO_GET_DATA_TRANSFER_INFO = 5u,
    CMD_TO_GET_VERSION_INFO       = 6u,
} CAN_REQUEST_CODE_e;

/** CAN identifier type */
typedef enum {
    CAN_STANDARD_IDENTIFIER_11_BIT,
    CAN_EXTENDED_IDENTIFIER_29_BIT,
    CAN_INVALID_TYPE,
} CAN_IDENTIFIER_TYPE_e;

/** Buffer element to store the ID and data of a CAN RX message */
typedef struct {
    CAN_NODE_s *pCanNode;         /*!< CAN node on which the message has been received */
    uint32_t id;                  /*!< ID of the CAN message */
    CAN_IDENTIFIER_TYPE_e idType; /*!< Standard or Extended identifier */
    uint8_t data[CAN_MAX_DLC];    /*!< payload of the CAN message */
} CAN_BUFFER_ELEMENT_s;

/** Definition of a CAN message (without data) */
typedef struct {
    uint32_t id;                  /*!< message ID */
    CAN_IDENTIFIER_TYPE_e idType; /*!< Standard or Extended identifier */
    uint8_t dlc;                  /*!< data length */
    CAN_ENDIANNESS_e endianness;  /*!< Byte order (big or little endian) */
} CAN_MESSAGE_PROPERTIES_s;

/** Type definition for tx callback functions used in CAN messages */
typedef uint32_t (*CAN_TxCallbackFunction_f)(CAN_MESSAGE_PROPERTIES_s message, uint8_t *pCanData);

/** Type definition for rx callback functions used in CAN messages */
typedef uint32_t (*CAN_RxCallbackFunction_f)(CAN_MESSAGE_PROPERTIES_s message, const uint8_t *const kpkCanData);

/** Type definition of the structure of a TX CAN message */
typedef struct {
    CAN_NODE_s *pCanNode;                      /*!< register on which the CAN interface is connected */
    CAN_MESSAGE_PROPERTIES_s message;          /*!< CAN message */
    CAN_TxCallbackFunction_f callbackFunction; /*!< CAN message callback after message is sent */
    uint8_t *pMuxId; /*!< for multiplexed signals: callback can use this as pointer to a mux variable, NULL_PTR if
                            unused*/
} CAN_TX_MESSAGE_TYPE_s;

/** type definition of the structure of an RX CAN message */
typedef struct {
    CAN_NODE_s *pCanNode;                      /*!< register on which the CAN interface is connected */
    CAN_MESSAGE_PROPERTIES_s message;          /*!< CAN message */
    CAN_RxCallbackFunction_f callbackFunction; /*!< CAN message callback after message is received */
} CAN_RX_MESSAGE_TYPE_s;

/** State of the can communication */
typedef enum {
    CAN_FSM_STATE_NO_COMMUNICATION      = 1u, /*!< no can communication has ever happened */
    CAN_FSM_STATE_WAIT_FOR_INFO         = 2u, /*!< start waiting for information of program */
    CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS   = 3u, /*!< start waiting for data in loops of program */
    CAN_FSM_STATE_RECEIVED_LOOP_NUMBER  = 4u, /*!< received the loop number in one data transfer loop */
    CAN_FSM_STATE_RECEIVED_8_BYTES_DATA = 5u, /*!< received the 8 bytes of data in one data transfer loop */
    CAN_FSM_STATE_RECEIVED_8_BYTES_CRC  = 6u, /*!< received the 8 bytes of CRC signature in one data transfer loop */
    CAN_FSM_STATE_FINISHED_FINAL_VALIDATION      = 7u,  /*!< finished the final validation */
    CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE = 8u,  /*!< finished transfer vector table */
    CAN_FSM_STATE_VALIDATED_VECTOR_TABLE         = 9u,  /*!< finished transfer vector table */
    CAN_FSM_STATE_ERROR                          = 10u, /*!< error happens during can communication */
    CAN_FSM_STATE_RESET_BOOT  = 11u, /*!< if there is a can message received to reset the boot process */
    CAN_FSM_STATE_RUN_PROGRAM = 12u, /*!< if there is a can message received to run the program */
} CAN_FSM_STATES_e;

/** Information on the program transfer process. */
typedef struct {
    uint32_t totalNumOfDataTransferLoops; /*!< the total number of data transfer loops */
    uint32_t numOfCurrentLoop;            /*!< The number of current loop */
    uint32_t programLengthInBytes;        /*!< The length of the program */
    uint32_t programStartAddressU8;       /*!< The start address of the program */
    uint32_t programCurrentAddressU8;     /*!< The current address where the next segment of program will be written */
    uint32_t programCurrentSectorAddressU8; /*!< The current sector address that stores the next segment of program */
    uint64_t programCrc8Bytes;              /*!< The received CRC */
    uint64_t programCrc8BytesOnBoard;       /*!< The calculated CRC */
    uint32_t sectorBufferCurrentAddressU8;  /*!< The start address of the sector buffer */
    uint8_t numOfReceivedVectorTableDataIn64Bytes;
    uint64_t vectorTable[CAN_VECTOR_TABLE_LENGTH];
    uint64_t crc64OfVectorTable;
} CAN_DATA_TRANSFER_INFO_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** State of CAN FSM */
extern CAN_FSM_STATES_e can_stateOfCanCommunication;

/** Information received during data transfer via CAN */
extern CAN_DATA_TRANSFER_INFO_s can_infoOfDataTransfer;

/** Copy of CAN data transfer to back up the CAN data transfer variable, which
 * will be used to recover the value contained in can_infoOfDataTransfer in
 * case an error occurs.
 */
extern CAN_DATA_TRANSFER_INFO_s can_copyOfInfoOfDataTransfer;

/** CAN node configurations for CAN1 */
extern const CAN_NODE_s can_node1;

/** CAN TX message configuration structs */
extern const CAN_TX_MESSAGE_TYPE_s can_txMessages[CAN_NUMBER_OF_TX_MESSAGES];

/** CAN RX message configuration structs */
extern const CAN_RX_MESSAGE_TYPE_s can_rxMessages[CAN_NUMBER_OF_RX_MESSAGES];

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Reset CAN communication relevant states, variables, and the RAM.
 */
extern void CAN_ResetCanCommunication(void);

/** @brief   Reset vector table relevant variables */
extern void CAN_ResetVectorTableRelevantVariables(void);

/**
 * @brief   Copy CAN_DATA_TRANSFER_INFO_s to CAN_DATA_TRANSFER_INFO_s.
 * @param   pkOriginalTransferInfo data source
 * @param   pCopyOfTransferInfo data destination
 */
extern void CAN_CopyCanDataTransferInfo(
    const CAN_DATA_TRANSFER_INFO_s *pkOriginalTransferInfo,
    CAN_DATA_TRANSFER_INFO_s *pCopyOfTransferInfo);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_CFG_H_ */
