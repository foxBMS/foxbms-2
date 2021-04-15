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
 * @updated 2019-12-04 (date of last update)
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

#include "database.h"

/*========== Macros and Definitions =========================================*/

/** macro for the register that handles the CAN-interface */
#define CAN_HET1_GIO (hetREG1)

/** register pin that handles enable */
#define CAN_HET1_EN_PIN (14U)
/** register pin that handles standby */
#define CAN_HET1_STB_PIN (16U)

/** Buffer element used to store the ID and data of a CAN RX message */
typedef struct CAN_BUFFERELEMENT {
    uint32_t id;     /*!< ID of the CAN message */
    uint8_t data[8]; /*!< payload of the CAN message */
} CAN_BUFFERELEMENT_s;

/* **************************************************************************************
 *  CAN BUFFER OPTIONS
 *****************************************************************************************/

/** CAN node enabling */
#define CAN_USE_CAN_NODE0 (1U)

/** CAN node enabling */
#define CAN_USE_CAN_NODE1 (0U)

/** CAN message ID to perform a software reset */
#define CAN_ID_SOFTWARE_RESET_MSG (0x95U)

/** delay in &micro;s used in #CAN_InitializeTransceiver for pin-toggling */
#define CAN_PIN_TOGGLE_DELAY_US (5u)

/** enum for byte order */
typedef enum {
    littleEndian,
    bigEndian,
} CAN_byteOrder_e;

/** type definition for callback functions used in CAN messages */
typedef uint32_t (
    *can_callback_funcPtr)(uint32_t ID, uint8_t DLC, CAN_byteOrder_e byteorder, uint8_t *candata, uint32_t *pMuxId);

/** type definition for structure of a TX CAN message */
typedef struct CAN_MSG_TX_TYPE {
    uint32_t id;                           /*!< CAN message id */
    uint8_t dlc;                           /*!< CAN message data length code */
    uint32_t repetitionTime;               /*!< CAN message cycle time */
    uint32_t repetitionPhase;              /*!< CAN message startup (first send) offset */
    CAN_byteOrder_e byteOrder;             /*!< Byte order (big or little endian) */
    can_callback_funcPtr callbackFunction; /*!< CAN message callback after message is sent */
    uint32_t *pMuxId; /*!< for multiplexed signals: callback can use this as pointer to a mux variable */
} CAN_MSG_TX_TYPE_s;

/** type definition for structure of an RX CAN message */
typedef struct CAN_MSG_RX_TYPE {
    uint32_t id;                           /*!< message ID */
    uint8_t dlc;                           /*!< data length */
    uint8_t rtr;                           /*!< rtr bit */
    CAN_byteOrder_e byteOrder;             /*!< Byte order (big or little endian) */
    can_callback_funcPtr callbackFunction; /*!< CAN message callback after message is received */
} CAN_MSG_RX_TYPE_s;

/*========== Extern Constant and Variable Declarations ======================*/
extern const CAN_MSG_TX_TYPE_s can_txMessages[];
extern const CAN_MSG_RX_TYPE_s can_rxMessages[];

/** array length for transmission CAN0 message definition @{*/
extern const uint8_t can_txLength;
extern const uint8_t can_rxLength;
/**@}*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern DATA_BLOCK_CELL_VOLTAGE_s *TEST_CAN_GetCellvoltageTab(void);

/* TX callback functions */
extern uint32_t TEST_CAN_TxVoltage(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
extern uint32_t TEST_CAN_TxPcbTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
extern uint32_t TEST_CAN_TxExternalTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
extern uint32_t TEST_CAN_TxVoltageMinMax(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);

/* RX callback functions */
extern uint32_t TEST_CAN_RxRequest(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
extern uint32_t TEST_CAN_RxSwReset(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
extern uint32_t TEST_CAN_RxCurrentSensor(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
extern uint32_t TEST_CAN_RxDebug(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
extern uint32_t TEST_CAN_RxSwVersion(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId);
#endif

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__CAN_CFG_H_ */
