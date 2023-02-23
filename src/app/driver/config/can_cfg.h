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
 * @file    can_cfg.h
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Headers for the configuration for the CAN module
 *
 * @details The activation and the length of the message buffers as well as the
 *          number of the messages that are received are to be configured here.
 *
 */

#ifndef FOXBMS__CAN_CFG_H_
#define FOXBMS__CAN_CFG_H_

/*========== Includes =======================================================*/

#include "HL_can.h"

#include "database.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* ****************************************************************************
 *  CAN NODE OPTIONS
 *****************************************************************************/
/** CAN node configuration struct */
typedef struct {
    canBASE_t *canNodeRegister; /*!< register on which the CAN interface is connected */
} CAN_NODE_s;

/** CAN node defines @{*/
#define CAN_NODE_1 ((CAN_NODE_s *)&can_node1)
#define CAN_NODE_2 ((CAN_NODE_s *)&can_node2Isolated)

#define CAN_NODE_DEBUG_MESSAGE  (CAN_NODE_1)
#define CAN_NODE_IMD            (CAN_NODE_1)
#define CAN_NODE_CURRENT_SENSOR (CAN_NODE_1)
/**@}*/

/**
 * Configuration of CAN transceiver pins to the respective port expander pins.
 * @{
 */
#define CAN1_ENABLE_PIN  (PEX_PIN00)
#define CAN1_STANDBY_PIN (PEX_PIN01)
#define CAN2_ENABLE_PIN  (PEX_PIN02)
#define CAN2_STANDBY_PIN (PEX_PIN03)
/**@}*/

/** Maximum ID if 11 bits are used */
#define CAN_MAX_11BIT_ID (2048u)
/** Maximum length of Data Length Code */
#define CAN_MAX_DLC (8u)
/** Default DLC */
#define CAN_DEFAULT_DLC (8u)
/** Default DLC for messages that are defined by the foxBMS project, i.e., not
 * defined by third party software and/or hardware. */
#define CAN_FOXBMS_MESSAGES_DEFAULT_DLC (8u)

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

/** CAN identifier type. Standard or extended identifier */
typedef enum {
    CAN_STANDARD_IDENTIFIER_11_BIT,
    CAN_EXTENDED_IDENTIFIER_29_BIT,
    CAN_INVALID_TYPE,
} CAN_IDENTIFIER_TYPE_e;

/** Buffer element used to store the ID and data of a CAN RX message */
typedef struct {
    CAN_NODE_s *canNode;          /*!< CAN node on which the message has been received */
    uint32_t id;                  /*!< ID of the CAN message */
    CAN_IDENTIFIER_TYPE_e idType; /*!< Standard or Extended identifier */
    uint8_t data[CAN_MAX_DLC];    /*!< payload of the CAN message */
} CAN_BUFFER_ELEMENT_s;

/** composite type for storing and passing on the local database table handles */
typedef struct {
    OS_QUEUE *pQueueImd;                                  /*!< handle of the message queue */
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage;         /*!< database table with cell voltages */
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperature; /*!< database table with cell temperatures */
    DATA_BLOCK_MIN_MAX_s *pTableMinMax;                   /*!< database table with min/max values */
    DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor;     /*!< database table with current sensor measurements */
    DATA_BLOCK_OPEN_WIRE_s *pTableOpenWire;               /*!< database table with open wire status */
    DATA_BLOCK_STATE_REQUEST_s *pTableStateRequest;       /*!< database table with state requests */
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues;           /*!< database table with pack values */
    DATA_BLOCK_SOF_s *pTableSof;                          /*!< database table with SOF values */
    DATA_BLOCK_SOX_s *pTableSox;                          /*!< database table with SOC and SOE values */
    DATA_BLOCK_ERROR_STATE_s *pTableErrorState;           /*!< database table with error state variables */
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulation; /*!< database table with insulation monitoring info */
    DATA_BLOCK_MSL_FLAG_s *pTableMsl;                     /*!< database table with MSL flags */
    DATA_BLOCK_RSL_FLAG_s *pTableRsl;                     /*!< database table with RSL flags */
    DATA_BLOCK_MOL_FLAG_s *pTableMol;                     /*!< database table with MOL flags */
} CAN_SHIM_s;

/** definition of a CAN message (without data) */
typedef struct {
    uint32_t id;                  /*!< message ID */
    CAN_IDENTIFIER_TYPE_e idType; /*!< Standard or Extended identifier */
    uint8_t dlc;                  /*!< data length */
    CAN_ENDIANNESS_e endianness;  /*!< Byte order (big or little endian) */
} CAN_MESSAGE_PROPERTIES_s;

/** timing information of a CAN TX message */
typedef struct {
    uint32_t period; /*!< CAN message cycle time */
    uint32_t phase;  /*!< CAN message startup (first send) offset */
} CAN_TX_MESSAGE_TIMING_s;

/** timing information of a CAN RX message */
typedef struct {
    uint32_t period; /*!< expected CAN message cycle time */
} CAN_RX_MESSAGE_TIMING_s;

/** type definition for tx callback functions used in CAN messages */
typedef uint32_t (*CAN_TxCallbackFunction_f)(
    CAN_MESSAGE_PROPERTIES_s message,
    uint8_t *canData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);

/** type definition for rx callback functions used in CAN messages */
typedef uint32_t (*CAN_RxCallbackFunction_f)(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/** type definition for structure of a TX CAN message */
typedef struct {
    CAN_NODE_s *canNode;                       /*!< CAN node on which the message is transmitted */
    CAN_MESSAGE_PROPERTIES_s message;          /*!< CAN message */
    CAN_TX_MESSAGE_TIMING_s timing;            /*!< time and phase */
    CAN_TxCallbackFunction_f callbackFunction; /*!< CAN message callback after message is sent */
    uint8_t *pMuxId; /*!< for multiplexed signals: callback can use this as pointer to a mux variable, NULL_PTR if
                            unused*/
} CAN_TX_MESSAGE_TYPE_s;

/* TODO: timing check not implemented for RX messages! */
/** type definition for structure of an RX CAN message */
typedef struct {
    CAN_NODE_s *canNode;                       /*!< CAN node on which the message is received */
    CAN_MESSAGE_PROPERTIES_s message;          /*!< CAN message */
    CAN_RX_MESSAGE_TIMING_s timing;            /*!< time and phase */
    CAN_RxCallbackFunction_f callbackFunction; /*!< CAN message callback after message is received */
} CAN_RX_MESSAGE_TYPE_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** variable for storing and passing on the local database table handles */
extern const CAN_SHIM_s can_kShim;

/** CAN node configurations for CAN1 and CAN2 (isolated) @{*/
extern const CAN_NODE_s can_node1;
extern const CAN_NODE_s can_node2Isolated;
/**@}*/

/** CAN RX and TX message configuration structs @{*/
extern const CAN_TX_MESSAGE_TYPE_s can_txMessages[];
extern const CAN_RX_MESSAGE_TYPE_s can_rxMessages[];
/**@}*/

/** array length for transmission CAN0 message definition @{*/
extern const uint8_t can_txLength;
extern const uint8_t can_rxLength;
/**@}*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_CFG_H_ */
