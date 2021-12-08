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
 * @file    mxm_bitextract.c
 * @author  foxBMS Team
 * @date    2019-01-15 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Bit extraction function for mxm_17841b
 *
 * @details This module supplies a specific bit extraction functionality to
 *          read & write into status registers. The different statuses that are
 *          read from or write into are indicated in the enum.
 *          Battery monitoring driver for MAX1785x battery monitoring ICs.
 *
 */

/*========== Includes =======================================================*/
#include "mxm_bitextract.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief       Extract a bit value function
 * @details     The function is used to extract some number of bits (numOfBits)
 *              from a position (pos) to write or read from the configuration
 *              and status registers.
 * @param[in]   value           value of the bits that shall be written or read
 * @param[in]   numberOfBits    number of bits to be extracted from the value
 *                              register
 * @param[in]   position        position of bits to be extracted from the value
 *                              register, start from right to left
 * @param[in]   shift           number of bit-shift to the left for read
 *                              function the shift value is zero.
 * @return      bit values for registers
 */
static MXM_41B_REG_BIT_VALUE bitValue(
    MXM_41B_REG_BIT_VALUE value,
    uint8_t numberOfBits,
    MXM_41B_REG_BITS position,
    MXM_41B_REG_BITS shift);

/*========== Static Function Implementations ================================*/
static MXM_41B_REG_BIT_VALUE bitValue(
    MXM_41B_REG_BIT_VALUE value,
    uint8_t numberOfBits,
    MXM_41B_REG_BITS position,
    MXM_41B_REG_BITS shift) {
    MXM_41B_REG_BIT_VALUE sBit;
    sBit = (((1u << numberOfBits) - 1u) & (((uint8_t)value) >> (position - 1u)));
    return (MXM_41B_REG_BIT_VALUE)((((uint8_t)sBit) << shift) & (uint8_t)UINT8_MAX);
}

/*========== Extern Function Implementations ================================*/
extern MXM_41B_REG_BIT_VALUE mxm_41bReadValue(uint8_t reg, uint8_t numberOfBits, MXM_41B_REG_BITS position) {
    return bitValue(reg, numberOfBits, (position + 1u), 0);
}

extern MXM_41B_REG_BIT_VALUE mxm_41bWriteValue(
    MXM_41B_REG_BIT_VALUE value,
    uint8_t numberOfBits,
    MXM_41B_REG_BITS shift,
    uint8_t reg) {
    uint8_t outputRegister = reg;
    outputRegister         = outputRegister & ~(MXM_REG_MASK(shift, numberOfBits));
    outputRegister         = bitValue(value, numberOfBits, 1, shift) | outputRegister;
    return outputRegister;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
