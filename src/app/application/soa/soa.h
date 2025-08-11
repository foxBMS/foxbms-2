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
 * @file    soa.h
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup APPLICATION
 * @prefix  SOA
 *
 * @brief   Header for SOA module, responsible for checking battery parameters
 *          against safety limits
 * @details TODO
 */

#ifndef FOXBMS__SOA_H_
#define FOXBMS__SOA_H_

/*========== Includes =======================================================*/
#include "soa_cfg.h"

#include "database.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   checks the abidance by the safe operating area
 * @param[in]   pMinimumMaximumCellVoltages  pointer to database entry with
 *                                           minimum and maximum cell voltages
 * @details verify for cell voltage measurements (U), if minimum and maximum
 *          values are out of range
 */
extern void SOA_CheckVoltages(DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellVoltages);

/**
 * @brief   checks the abidance by the safe operating area
 * @param[in]   pMinimumMaximumCellTemperatures  pointer to database entry with
 * @param[in]   pCurrent                         pointer to pack value database entry
 * @details verify for cell temperature measurements (T), if minimum and
 *          maximum values are out of range
 */
extern void SOA_CheckTemperatures(
    DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellTemperatures,
    DATA_BLOCK_PACK_VALUES_s *pCurrent);

/**
 * @brief   checks the abidance by the safe operating area
 * @param[in]   pTablePackValues   pointer to pack values database entry
 * @details verify for cell current measurements (I), if minimum and maximum
 *          values are out of range
 */
extern void SOA_CheckCurrent(DATA_BLOCK_PACK_VALUES_s *pTablePackValues);

/**
 * @brief   FOR FUTURE COMPATIBILITY; DUMMY FUNCTION; DO NOT USE
 * @details FOR FUTURE COMPATIBILITY; DUMMY FUNCTION; DO NOT USE
 */
extern void SOA_CheckSlaveTemperatures(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__SOA_H_ */
