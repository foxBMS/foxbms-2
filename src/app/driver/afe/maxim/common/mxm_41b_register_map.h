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
 * @file    mxm_41b_register_map.h
 * @author  foxBMS Team
 * @date    2020-06-25 (date of creation)
 * @updated 2021-06-16 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Register map of the MAX17841 bridge IC
 *
 */

#ifndef FOXBMS__MXM_41B_REGISTER_MAP_H_
#define FOXBMS__MXM_41B_REGISTER_MAP_H_

/*========== Includes =======================================================*/

/*========== Macros and Definitions =========================================*/
/**
 * @brief Type for MAX17841B buffer transaction commands
 *
 * SPI commands for buffer transactions of the MAX17841B.
 */
typedef uint8_t MXM_41B_BUF_CMD_t;

/**
 * @brief MAX17841B register addresses
 *
 * Register addresses for the MAX17841B.
 * Valid register addresses range from 0x01 to 0x1B
 * and 0x95 to 0x9B
 * Odd addresses are read addresses and even addresses are
 * write addresses. Read-only registers have no write address.
 */
typedef uint8_t MXM_41B_REG_ADD_t;

/* AXIVION Disable Style MisraC2012-2.5: For completeness, this section lists all register addresses even though the driver does not use them. */
/**
 * @brief Reset transmit buffer to default state and clear TX_Q and LD_Q
 */
#define MXM_BUF_CLR_TX_BUF ((MXM_41B_BUF_CMD_t)0x20u)

/**
 * @brief Read receive buffer starting at RX_RD_Pointer
 */
#define MXM_BUF_RD_MSG ((MXM_41B_BUF_CMD_t)0x91u)

/**
 * @brief Read receive buffer starting at the oldest unread message
 */
#define MXM_BUF_RD_NXT_MSG ((MXM_41B_BUF_CMD_t)0x93u)

/**
 * @brief Select next load queue and write starting from location 0
 */
#define MXM_BUF_WR_NXT_LD_Q_0 ((MXM_41B_BUF_CMD_t)0xB0u)

/**
 * @brief Select next load queue and write starting from location 1
 */
#define MXM_BUF_WR_NXT_LD_Q_1 ((MXM_41B_BUF_CMD_t)0xB2u)

/**
 * @brief Select next load queue and write starting from location 2
 */
#define MXM_BUF_WR_NXT_LD_Q_2 ((MXM_41B_BUF_CMD_t)0xB4u)

/**
 * @brief Select next load queue and write starting from location 3
 */
#define MXM_BUF_WR_NXT_LD_Q_3 ((MXM_41B_BUF_CMD_t)0xB6u)

/**
 * @brief Select next load queue and write starting from location 4
 */
#define MXM_BUF_WR_NXT_LD_Q_4 ((MXM_41B_BUF_CMD_t)0xB8u)

/**
 * @brief Select next load queue and write starting from location 5
 */
#define MXM_BUF_WR_NXT_LD_Q_5 ((MXM_41B_BUF_CMD_t)0xBAu)

/**
 * @brief Select next load queue and write starting from location 6
 */
#define MXM_BUF_WR_NXT_LD_Q_6 ((MXM_41B_BUF_CMD_t)0xBCu)

/**
 * @brief Write load queue starting from location 0
 */
#define MXM_BUF_WR_LD_Q_0 ((MXM_41B_BUF_CMD_t)0xC0u)

/**
 * @brief Write load queue starting from location 1
 */
#define MXM_BUF_WR_LD_Q_1 ((MXM_41B_BUF_CMD_t)0xC2u)

/**
 * @brief Write load queue starting from location 2
 */
#define MXM_BUF_WR_LD_Q_2 ((MXM_41B_BUF_CMD_t)0xC4u)

/**
 * @brief Write load queue starting from location 3
 */
#define MXM_BUF_WR_LD_Q_3 ((MXM_41B_BUF_CMD_t)0xC6u)

/**
 * @brief Write load queue starting from location 4
 */
#define MXM_BUF_WR_LD_Q_4 ((MXM_41B_BUF_CMD_t)0xC8u)

/**
 * @brief Write load queue starting from location 5
 */
#define MXM_BUF_WR_LD_Q_5 ((MXM_41B_BUF_CMD_t)0xCAu)

/**
 * @brief Write load queue starting from location 6
 */
#define MXM_BUF_WR_LD_Q_6 ((MXM_41B_BUF_CMD_t)0xCCu)

/**
 * @brief Read load queue starting from location 0
 */
#define MXM_BUF_RD_LD_Q_0 ((MXM_41B_BUF_CMD_t)0xC1u)

/**
 * @brief Read load queue starting from location 1
 */
#define MXM_BUF_RD_LD_Q_1 ((MXM_41B_BUF_CMD_t)0xC3u)

/**
 * @brief Read load queue starting from location 2
 */
#define MXM_BUF_RD_LD_Q_2 ((MXM_41B_BUF_CMD_t)0xC5u)

/**
 * @brief Read load queue starting from location 3
 */
#define MXM_BUF_RD_LD_Q_3 ((MXM_41B_BUF_CMD_t)0xC7u)

/**
 * @brief Read load queue starting from location 4
 */
#define MXM_BUF_RD_LD_Q_4 ((MXM_41B_BUF_CMD_t)0xC9u)

/**
 * @brief Read load queue starting from location 5
 */
#define MXM_BUF_RD_LD_Q_5 ((MXM_41B_BUF_CMD_t)0xCBu)

/**
 * @brief Read load queue starting from location 6
 */
#define MXM_BUF_RD_LD_Q_6 ((MXM_41B_BUF_CMD_t)0xCDu)

/**
 * @brief Reset receive buffer and pointers to default state
 */
#define MXM_BUF_CLR_RX_BUF ((MXM_41B_BUF_CMD_t)0xE0u)

/**
 * @brief RX status register read address
 */
#define MXM_REG_RX_STATUS_R ((MXM_41B_REG_ADD_t)0x01u)

/**
 * @brief TX status register read address
 */
#define MXM_REG_TX_STATUS_R ((MXM_41B_REG_ADD_t)0x03u)

/**
 * @brief RX interrupt enable register read address
 */
#define MXM_REG_RX_INTERRUPT_ENABLE_R ((MXM_41B_REG_ADD_t)0x05u)

/**
 * @brief RX interrupt enable register write address
 */
#define MXM_REG_RX_INTERRUPT_ENABLE_W ((MXM_41B_REG_ADD_t)0x04u)

/**
 * @brief TX interrupt enable register read address
 */
#define MXM_REG_TX_INTERRUPT_ENABLE_R ((MXM_41B_REG_ADD_t)0x07u)

/**
 * @brief TX interrupt enable register write address
 */
#define MXM_REG_TX_INTERRUPT_ENABLE_W ((MXM_41B_REG_ADD_t)0x06u)

/**
 * @brief RX interrupt flags register read address
 */
#define MXM_REG_RX_INTERRUPT_FLAGS_R ((MXM_41B_REG_ADD_t)0x09u)

/**
 * @brief RX interrupt flags register write address
 */
#define MXM_REG_RX_INTERRUPT_FLAGS_W ((MXM_41B_REG_ADD_t)0x08u)

/**
 * @brief TX interrupt flags register read address
 */
#define MXM_REG_TX_INTERRUPT_FLAGS_R ((MXM_41B_REG_ADD_t)0x0Bu)

/**
 * @brief TX interrupt flags register write address
 */
#define MXM_REG_TX_INTERRUPT_FLAGS_W ((MXM_41B_REG_ADD_t)0x0Au)

/**
 * @brief Configuration 1 register read address
 */
#define MXM_REG_CONFIGURATION_1_R ((MXM_41B_REG_ADD_t)0x0Du)

/**
 * @brief Configuration 1 register write address
 */
#define MXM_REG_CONFIGURATION_1_W ((MXM_41B_REG_ADD_t)0x0Cu)

/**
 * @brief Configuration 2 register read address
 */
#define MXM_REG_CONFIGURATION_2_R ((MXM_41B_REG_ADD_t)0x0Fu)

/**
 * @brief Configuration 2 register write address
 */
#define MXM_REG_CONFIGURATION_2_W ((MXM_41B_REG_ADD_t)0x0Eu)

/**
 * @brief Configuration 3 register read address
 */
#define MXM_REG_CONFIGURATION_3_R ((MXM_41B_REG_ADD_t)0x11u)

/**
 * @brief Configuration 3 register write address
 */
#define MXM_REG_CONFIGURATION_3_W ((MXM_41B_REG_ADD_t)0x10u)

/**
 * @brief FMEA register read address
 */
#define MXM_REG_FMEA_R ((MXM_41B_REG_ADD_t)0x13u)

/**
 * @brief Model register read address
 */
#define MXM_REG_MODEL_R ((MXM_41B_REG_ADD_t)0x15u)

/**
 * @brief Version register read address
 */
#define MXM_REG_VERSION_R ((MXM_41B_REG_ADD_t)0x17u)

/**
 * @brief RX byte register read address
 */
#define MXM_REG_RX_BYTE_R ((MXM_41B_REG_ADD_t)0x19u)

/**
 * @brief RX space register read address
 */
#define MXM_REG_RX_SPACE_R ((MXM_41B_REG_ADD_t)0x1Bu)

/**
 * @brief TX queue selects register read address
 */
#define MXM_REG_TX_QUEUE_SELECTS_R ((MXM_41B_REG_ADD_t)0x95u)

/**
 * @brief RX read pointer register read address
 */
#define MXM_REG_RX_READ_POINTER_R ((MXM_41B_REG_ADD_t)0x97u)

/**
 * @brief RX write pointer register read address
 */
#define MXM_REG_RX_WRITE_POINTER_R ((MXM_41B_REG_ADD_t)0x99u)

/**
 * @brief RX next message register read address
 */
#define MXM_REG_RX_NEXT_MESSAGE_R ((MXM_41B_REG_ADD_t)0x9Bu)

/* AXIVION Enable Style MisraC2012-2.5: */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_41B_REGISTER_MAP_H_ */
