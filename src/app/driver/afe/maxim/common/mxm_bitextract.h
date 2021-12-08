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
 * @file    mxm_bitextract.h
 * @author  foxBMS Team
 * @date    2019-01-15 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Bit extraction function for MXM_17841b
 *
 * @details This module supplies a specific bit extraction functionality to
 * read & write into status registers. The different statuses that are read
 * from or write into are indicated in the enum.
 * Battery monitoring driver for MAX1785x battery monitoring ICs.
 *
 */

#ifndef FOXBMS__MXM_BITEXTRACT_H_
#define FOXBMS__MXM_BITEXTRACT_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/**
 * @brief Bit-values for registers
 */
/* AXIVION Disable Style MisraC2012-2.5: Defines are based on datasheet and exist all for completeness. */
typedef uint8_t MXM_41B_REG_BIT_VALUE;
/** one bit, 0u */
#define MXM_41B_REG_FALSE ((MXM_41B_REG_BIT_VALUE)0x00u)
/** one bit, 1u */
#define MXM_41B_REG_TRUE ((MXM_41B_REG_BIT_VALUE)0x01u)
/** bit sequence for baud rate 500KBps */
#define MXM_41B_REG_BAUD_RATE_500KBPS ((MXM_41B_REG_BIT_VALUE)0x00u)
/** alternative bit sequence for baud rate 500KBps */
#define MXM_41B_REG_BAUD_RATE_500KBPS_ALT ((MXM_41B_REG_BIT_VALUE)0x01u)
/** bit sequence for baud rate 1MBps */
#define MXM_41B_REG_BAUD_RATE_1MBPS ((MXM_41B_REG_BIT_VALUE)0x02u)
/** bit sequence for baud rate 2MBps */
#define MXM_41B_REG_BAUD_RATE_2MBPS ((MXM_41B_REG_BIT_VALUE)0x03u)
/** bit sequence for a keep alive period of 0&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_0US ((MXM_41B_REG_BIT_VALUE)0x00u)
/** bit sequence for a keep alive period of 10&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_10US ((MXM_41B_REG_BIT_VALUE)0x01u)
/** bit sequence for a keep alive period of 20&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_20US ((MXM_41B_REG_BIT_VALUE)0x02u)
/** bit sequence for a keep alive period of 40&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_40US ((MXM_41B_REG_BIT_VALUE)0x03u)
/** bit sequence for a keep alive period of 80&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_80US ((MXM_41B_REG_BIT_VALUE)0x04u)
/** bit sequence for a keep alive period of 160&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_160US ((MXM_41B_REG_BIT_VALUE)0x05u)
/** bit sequence for a keep alive period of 320&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_320US ((MXM_41B_REG_BIT_VALUE)0x06u)
/** bit sequence for a keep alive period of 640&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_640US ((MXM_41B_REG_BIT_VALUE)0x07u)
/** bit sequence for a keep alive period of 1280&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_1280US ((MXM_41B_REG_BIT_VALUE)0x08u)
/** bit sequence for a keep alive period of 2560&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_2560US ((MXM_41B_REG_BIT_VALUE)0x09u)
/** bit sequence for a keep alive period of 5120&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_5120US ((MXM_41B_REG_BIT_VALUE)0x0Au)
/** bit sequence for a keep alive period of 10240&micro;s */
#define MXM_41B_REG_KEEP_ALIVE_10240US ((MXM_41B_REG_BIT_VALUE)0x0Bu)
/** bit sequence for a infinite keep alive period (disabled) */
#define MXM_41B_REG_KEEP_ALIVE_INF_DLY ((MXM_41B_REG_BIT_VALUE)0x0Fu)
/* AXIVION Enable Style MisraC2012-2.5: */

/**
 * @brief define for creating a bitmask
 */
#define MXM_REG_MASK(start, end) (((1u << ((end) - (start) + 1u)) - 1u) << (start))

/** types for bits in the register of MAX17841B */
typedef uint8_t MXM_41B_REG_BITS;
/** bit for the feature TX Preambles */
#define MXM_41B_TX_PREAMBLES ((MXM_41B_REG_BITS)5u)
/** bit for the feature Keep Alive */
#define MXM_41B_KEEP_ALIVE ((MXM_41B_REG_BITS)0u)
/** bit for the RX Error */
#define MXM_41B_RX_ERROR ((MXM_41B_REG_BITS)7u)
/** bit for the RX Busy Status */
#define MXM_41B_RX_BUSY_STATUS ((MXM_41B_REG_BITS)5u)
/** bit for the feature RX Overflow Interrupt */
#define MXM_41B_RX_OVERFLOW_INT_ENABLE ((MXM_41B_REG_BITS)3u)
/** bit for the RX Overflow Status */
#define MXM_41B_RX_OVERFLOW_STATUS ((MXM_41B_REG_BITS)3u)
/** bit for the RX Stop Status */
#define MXM_41B_RX_STOP_STATUS ((MXM_41B_REG_BITS)1u)
/** bit for the RX Empty Status */
#define MXM_41B_RX_EMPTY_STATUS ((MXM_41B_REG_BITS)0u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief       write a value to a register supplied as variable
 * @details     This function takes a register (reg), writes the data
 *              described by numberOFBits, shift and value to the register
 *              value and returns the updated register.
 * @param[in]   value           register value that should be written to the
 *                              register
 * @param[in]   numberOfBits    length of the entry that should be written in
 *                              bits
 * @param[in]   shift           start position in the register in bits
 * @param[in]   reg             complete value of the register
 * @return      updated value of the register
 */
extern MXM_41B_REG_BIT_VALUE mxm_41bWriteValue(
    MXM_41B_REG_BIT_VALUE value,
    uint8_t numberOfBits,
    MXM_41B_REG_BITS shift,
    uint8_t reg);

/**
 * @brief       read a value from a register supplied as variable
 * @details     This function takes a register (reg), and extracts the data
 *              described by its length in bits (numberOfBits) and position
 *              (pos). This value is then returned.
 * @param[in]   reg             value of the register to be read
 * @param[in]   numberOfBits    length og the bit sequence in bits
 * @param[in]   position        start position of the bit sequence
 * @returns     the isolated and shifted bit value
 */
extern MXM_41B_REG_BIT_VALUE mxm_41bReadValue(uint8_t reg, uint8_t numberOfBits, MXM_41B_REG_BITS position);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_BITEXTRACT_H_ */
