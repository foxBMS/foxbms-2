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
 * @file    mxm_crc8.h
 * @author  foxBMS Team
 * @date    2019-02-05 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Headers for the CRC8 calculation for Maxim Integrated
 *          Monitoring devices
 *
 * @details This module supports the calculation of a CRC8 based on the
 *          polynomial described in the Maxim data sheets.
 *          The polynomial is 0xA6.
 *
 */

#ifndef FOXBMS__MXM_CRC8_H_
#define FOXBMS__MXM_CRC8_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Compute CRC8 with initial value set to 0x00.
 * @details Computes the CRC8 for the given data with the given length
 *          with the polynomial 0xA6. This function calls
 *          #MXM_CRC8WithInitValue() with CRC set to 0x00.
 *          The result should be 0x00 if you include the CRC into
 *          the datastream.
 * @param[in]   pData   array-pointer with data to be cyclic redundancy checked
 * @param[in]   lenData length of array
 * @return      uint8_t containing the computed CRC
 */
extern uint8_t MXM_CRC8(uint16_t *pData, int32_t lenData);

/**
 * @brief   Test the CRC8-algorithm with a known pattern.
 * @details Tests the CRC8-algorithm with known patterns and
 *          asserts that the implementation matches the required algorithm.
 * @return  #STD_OK if the selfcheck is successful and #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e must_check_return MXM_CRC8SelfTest(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_CRC8_H_ */
