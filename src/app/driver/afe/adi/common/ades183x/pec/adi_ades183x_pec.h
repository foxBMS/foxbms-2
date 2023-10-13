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
 * @file    adi_ades183x_pec.h
 * @author  foxBMS Team
 * @date    2019-12-12 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Headers for the PEC computations.
 * @details The CRC polynomials used are defined in the data sheets of the ICs.
 *
 */

#ifndef FOXBMS__ADI_ADES183X_PEC_H_
#define FOXBMS__ADI_ADES183X_PEC_H_

/*========== Includes =======================================================*/

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Defines for the PEC computation */
/**@{*/
#define ADI_PEC_BYTE_SIZE              (8u)
#define ADI_PEC_ONE_BYTE_MASK          (0xFFu)
#define ADI_PEC_PRECOMPUTED_TABLE_SIZE (256u)

#define ADI_PEC15_POLYNOMIAL_SIZE (15u)
#define ADI_PEC15_SEED            (0x10u)
#define ADI_PEC15_MASK            (0x7FFFu)

#define ADI_PEC10_POLYNOMIAL                   (0x48Fu)
#define ADI_PEC10_POLYNOMIAL_SIZE              (10u)
#define ADI_PEC10_SEED                         (0x10u)
#define ADI_PEC10_MASK                         (0x3FFu)
#define ADI_PEC10_MSB_MASK                     (0x200u)
#define ADI_PEC10_COMMAND_COUNTER_SIZE_IN_BITS (6u)
#define ADI_PEC10_COMMAND_COUNTER_MASK         (0xFCu)
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   calculates the PEC15
 * @param   length   Number of bytes that will be used to calculate a PEC
 * @param   data     Array of data that will be used to calculate  a PEC
 * @return  PEC15, shifted one bit to the left
 */
extern uint16_t ADI_Pec15(uint8_t length, uint8_t *data);

/**
 * @brief   calculates the PEC10
 *K
 * @param   length   Number of bytes that will be used to calculate a PEC
 * @param   data     Array of data that will be used to calculate  a PEC
 * @param   receive  true if data was received, false if data is to be sent
 * @return  PEC10
 */
extern uint16_t ADI_Pec10(uint8_t length, uint8_t *data, bool receive);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__ADI_ADES183X_PEC_H_ */
