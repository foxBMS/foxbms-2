/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @updated 2021-12-01 (date of last update)
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Headers for the configuration for the CAN module
 *
 * The activation and the length of the message buffers as well as the number of
 * the messages that are received are to be configured here
 *
 */

#ifndef FOXBMS__CAN_CFG_H_
#define FOXBMS__CAN_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "HL_can.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/

/** register on which the CAN interface is connected @{*/
#define CAN1_NODE (canREG1)
#define CAN2_NODE (canREG2)
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
#define CAN_DLC (8u)

/** TX messages - pack related */

/** CAN message ID to send state */
#define CAN_ID_TX_STATE (0x220U)
/** Periodicity of CAN state messages in ms */
#define CAN_TX_STATE_PERIOD_MS (100U)
/** Phase of CAN state messages in ms */
#define CAN_TX_STATE_PHASE_MS (0U)

/** CAN message ID to send voltages */
#define CAN_ID_TX_VOLTAGES (0x240U)
/** Periodicity of CAN voltage messages in ms */
#define CAN_TX_VOLTAGES_PERIOD_MS (100U)
/** Phase of CAN voltage messages in ms */
#define CAN_TX_VOLTAGES_PHASE_MS (10U)

/** CAN message ID to send temperatures */
#define CAN_ID_TX_TEMPERATURES (0x250U)
/** Periodicity of CAN temperature messages in ms */
#define CAN_TX_TEMPERATURES_PERIOD_MS (200U)
/** Phase of CAN temperature messages in ms */
#define CAN_TX_TEMPERATURES_PHASE_MS (20U)

/** CAN message ID to send limit values */
#define CAN_ID_TX_LIMIT_VALUES (0x224U)
/** Periodicity of CAN limit messages in ms */
#define CAN_TX_LIMIT_VALUES_PERIOD_MS (100U)
/** Phase of CAN limit messages in ms */
#define CAN_TX_LIMIT_VALUES_PHASE_MS (30U)

/** CAN message ID to send minimum and maximum values */
#define CAN_ID_TX_MINIMUM_MAXIMUM_VALUES (0x223U)
/** Periodicity of CAN minimum and maximum value messages in ms */
#define CAN_TX_MINIMUM_MAXIMUM_VALUES_PERIOD_MS (100U)
/** Phase of CAN minimum and maximum value  messages in ms */
#define CAN_TX_MINIMUM_MAXIMUM_VALUES_PHASE_MS (40U)

/** CAN message ID to send pack state estimation values */
#define CAN_ID_TX_PACK_STATE_ESTIMATION (0x225U)
/** Periodicity of CAN pack state estimation messages in ms */
#define CAN_TX_PACK_STATE_ESTIMATION_PERIOD_MS (1000U)
/** Phase of CAN pack state estimation messages in ms */
#define CAN_TX_PACK_STATE_ESTIMATION_PHASE_MS (50U)

/** CAN message ID to send pack values */
#define CAN_ID_TX_PACK_VALUES (0x222U)
/** Periodicity of CAN pack values messages in ms */
#define CAN_TX_PACK_VALUES_PERIOD_MS (100U)
/** Phase of CAN pack values messages in ms */
#define CAN_TX_PACK_VALUES_PHASE_MS (60U)

/** TX messages - string related */

/** CAN message ID to send string state */
#define CAN_ID_TX_STRING_STATE (0x221U)
/** Periodicity of CAN string state messages in ms */
#define CAN_TX_STRING_STATE_PERIOD_MS (100U)
/** Phase of CAN string state messages in ms */
#define CAN_TX_STRING_STATE_PHASE_MS (70U)

/** CAN message ID to send string values */
#define CAN_ID_TX_STRING_VALUES (0x280U)
/** Periodicity of CAN string state messages in ms */
#define CAN_TX_STRING_VALUES_PERIOD_MS (100U)
/** Phase of CAN string state messages in ms */
#define CAN_TX_STRING_VALUES_PHASE_MS (80U)

/** CAN message ID to send string minimum and maximum values */
#define CAN_ID_TX_STRING_MINIMUM_MAXIMUM (0x281U)
/** Periodicity of CAN string state messages in ms */
#define CAN_TX_STRING_MINIMUM_MAXIMUM_PERIOD_MS (100U)
/** Phase of CAN string state messages in ms */
#define CAN_TX_STRING_MINIMUM_MAXIMUM_PHASE_MS (90U)

/** CAN message ID to send string minimum and maximum values */
#define CAN_ID_TX_STRING_STATE_ESTIMATION (0x282U)
/** Periodicity of CAN string state messages in ms */
#define CAN_TX_STRING_STATE_ESTIMATION_PERIOD_MS (1000U)
/** Phase of CAN string state messages in ms */
#define CAN_TX_STRING_STATE_ESTIMATION_PHASE_MS (0U)

/** CAN message ID to send string values 2 */
#define CAN_ID_TX_STRING_VALUES_2 (0x283U)
/** Periodicity of CAN string state messages in ms */
#define CAN_TX_STRING_VALUES_2_PERIOD_MS (100U)
/** Phase of CAN string state messages in ms */
#define CAN_TX_STRING_VALUES_2_PHASE_MS (10U)

/** RX messages */

/** CAN message ID for boot message */
#define CAN_ID_BOOT_MESSAGE (0x199u)

/** CAN message ID to perform a software reset */
#define CAN_ID_SOFTWARE_RESET (0x95U)

/** CAN message ID to perform a state request */
#define CAN_ID_COMMAND (0x230U)

/** CAN message ID for debug message */
#define CAN_ID_DEBUG (0x200U)

/** CAN message ID to get software version */
#define CAN_ID_SW_VERSION (0x777U)

/**
 * -------------------------CAUTION-------------------------
 * The 3 following defines are used by the insulation monitoring device (IMD).
 * If they are changed, the IMD will not work anymore
 * -------------------------CAUTION-------------------------
 */
/** CAN message ID for info message from iso165c */
#define CAN_ID_IMD_INFO (0x37U)
/** CAN message ID for request message to iso165c */
#define CAN_ID_IMD_REQUEST (0x22U)
/** CAN message ID for response message from iso165c */
#define CAN_ID_IMD_RESPONSE (0x23U)

/** IDs for the messages from the current sensors */
/** String 0 @{*/
#define CAN_ID_STRING0_CURRENT         (0x521u)
#define CAN_ID_STRING0_VOLTAGE1        (0x522u)
#define CAN_ID_STRING0_VOLTAGE2        (0x523u)
#define CAN_ID_STRING0_VOLTAGE3        (0x524u)
#define CAN_ID_STRING0_TEMPERATURE     (0x525u)
#define CAN_ID_STRING0_POWER           (0x526u)
#define CAN_ID_STRING0_CURRENT_COUNTER (0x527u)
#define CAN_ID_STRING0_ENERGY_COUNTER  (0x528u)
/**@}
 * String 1 @{*/
#define CAN_ID_STRING1_CURRENT         (0x621u)
#define CAN_ID_STRING1_VOLTAGE1        (0x622u)
#define CAN_ID_STRING1_VOLTAGE2        (0x623u)
#define CAN_ID_STRING1_VOLTAGE3        (0x624u)
#define CAN_ID_STRING1_TEMPERATURE     (0x625u)
#define CAN_ID_STRING1_POWER           (0x626u)
#define CAN_ID_STRING1_CURRENT_COUNTER (0x627u)
#define CAN_ID_STRING1_ENERGY_COUNTER  (0x628u)
/**@}
 * String 2 @{*/
#define CAN_ID_STRING2_CURRENT         (0x721u)
#define CAN_ID_STRING2_VOLTAGE1        (0x722u)
#define CAN_ID_STRING2_VOLTAGE2        (0x723u)
#define CAN_ID_STRING2_VOLTAGE3        (0x724u)
#define CAN_ID_STRING2_TEMPERATURE     (0x725u)
#define CAN_ID_STRING2_POWER           (0x726u)
#define CAN_ID_STRING2_CURRENT_COUNTER (0x727u)
#define CAN_ID_STRING2_ENERGY_COUNTER  (0x728u)
/**@}*/

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
typedef enum CAN_ENDIANNESS {
    CAN_LITTLE_ENDIAN,
    CAN_BIG_ENDIAN,
} CAN_ENDIANNESS_e;

/** Buffer element used to store the ID and data of a CAN RX message */
typedef struct CAN_BUFFERELEMENT {
    canBASE_t *canNode;        /*!< CAN node on which the message has been received */
    uint32_t id;               /*!< ID of the CAN message */
    uint8_t data[CAN_MAX_DLC]; /*!< payload of the CAN message */
} CAN_BUFFERELEMENT_s;

/** composite type for storing and passing on the local database table handles */
typedef struct CAN_SHIM {
    OS_QUEUE *pQueueImd;                                  /*!< handle of the message queue */
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltage;         /*!< database table with cell voltages */
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperature; /*!< database table with cell temperatures */
    DATA_BLOCK_MIN_MAX_s *pTableMinMax;                   /*!< database table with min/max values */
    DATA_BLOCK_CURRENT_SENSOR_s *pTableCurrentSensor;     /*!< database table with current sensor measurements */
    DATA_BLOCK_OPEN_WIRE_s *pTableOpenWire;               /*!< database table with open wire status */
    DATA_BLOCK_STATEREQUEST_s *pTableStateRequest;        /*!< database table with state requests */
    DATA_BLOCK_PACK_VALUES_s *pTablePackValues;           /*!< database table with pack values */
    DATA_BLOCK_SOF_s *pTableSof;                          /*!< database table with SOF values */
    DATA_BLOCK_SOX_s *pTableSox;                          /*!< database table with SOC and SOE values */
    DATA_BLOCK_ERRORSTATE_s *pTableErrorState;            /*!< database table with error state variables */
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulation; /*!< database table with insulation monitoring info */
    DATA_BLOCK_MSL_FLAG_s *pTableMsl;                     /*!< database table with MSL flags */
    DATA_BLOCK_RSL_FLAG_s *pTableRsl;                     /*!< database table with RSL flags */
    DATA_BLOCK_MOL_FLAG_s *pTableMol;                     /*!< database table with MOL flags */
} CAN_SHIM_s;

/** type definition for callback functions used in CAN messages */
typedef uint32_t (*can_callback_funcPtr)(
    uint32_t ID,
    uint8_t DLC,
    CAN_ENDIANNESS_e endianness,
    uint8_t *candata,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim);

/** type definition for structure of a TX CAN message */
typedef struct CAN_MSG_TX_TYPE {
    canBASE_t *canNode;                    /*!< CAN node on which the message is transmitted */
    uint32_t id;                           /*!< CAN message id */
    uint8_t dlc;                           /*!< CAN message data length code */
    uint32_t repetitionTime;               /*!< CAN message cycle time */
    uint32_t repetitionPhase;              /*!< CAN message startup (first send) offset */
    CAN_ENDIANNESS_e endianness;           /*!< Byte order (big or little endian) */
    can_callback_funcPtr callbackFunction; /*!< CAN message callback after message is sent */
    uint8_t *pMuxId; /*!< for multiplexed signals: callback can use this as pointer to a mux variable, NULL_PTR if
                            unused*/
} CAN_MSG_TX_TYPE_s;

/** type definition for structure of an RX CAN message */
typedef struct CAN_MSG_RX_TYPE {
    canBASE_t *canNode;                    /*!< CAN node on which the message is received */
    uint32_t id;                           /*!< message ID */
    uint8_t dlc;                           /*!< data length */
    CAN_ENDIANNESS_e endianness;           /*!< Byte order (big or little endian) */
    can_callback_funcPtr callbackFunction; /*!< CAN message callback after message is received */
} CAN_MSG_RX_TYPE_s;

/** variable for storing and passing on the local database table handles */
extern const CAN_SHIM_s can_kShim;

/*========== Extern Constant and Variable Declarations ======================*/
extern const CAN_MSG_TX_TYPE_s can_txMessages[];
extern const CAN_MSG_RX_TYPE_s can_rxMessages[];

/** array length for transmission CAN0 message definition @{*/
extern const uint8_t can_txLength;
extern const uint8_t can_rxLength;
/**@}*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__CAN_CFG_H_ */
