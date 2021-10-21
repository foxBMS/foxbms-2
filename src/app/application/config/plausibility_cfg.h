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
 * @file    plausibility_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2020-02-24 (date of last update)
 * @ingroup APPLICATION_CONFIGURATION
 * @prefix  PL
 *
 * @brief   plausibility checks for cell voltage and cell temperatures
 *
 */

#ifndef FOXBMS__PLAUSIBILITY_CFG_H_
#define FOXBMS__PLAUSIBILITY_CFG_H_

/*========== Includes =======================================================*/
#include "battery_cell_cfg.h"

/*========== Macros and Definitions =========================================*/
/**
 * @ingroup CONFIG_PLAUSIBILITY
 * @brief   Maximum difference between pack voltage measurement from
 *          AFE and current sensor
 * @ptype   int
 * \par Range:
 * [0, 10000]
 */
#define PL_STRING_VOLTAGE_TOLERANCE_mV (3000)

/**
 * @ingroup CONFIG_PLAUSIBILITY
 * @brief   Maximum difference between redundant cell voltage measurement
 * @ptype   int
 * \par Range:
 * [0, 10000]
 */
#define PL_CELL_VOLTAGE_TOLERANCE_mV (10)

/**
 * @ingroup CONFIG_PLAUSIBILITY
 * @brief   Maximum difference between redundant cell temperature measurements
 *          in deci kelvin
 * @ptype   int
 * \par Range:
 * [0, 100]
 */
#define PL_CELL_TEMPERATURE_TOLERANCE_dK (50)

/**
 * @ingroup CONFIG_PLAUSIBILITY
 * @brief   Maximum deviation between a single cell voltage measurement and the
 *          average cell voltage
 * @ptype   int
 * \par Range:
 * [0, 10000]
 */
#define PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV (300)

/**
 * @ingroup CONFIG_PLAUSIBILITY
 * @brief   Maximum deviation between a single cell temperature measurement and
 *          the average cell temperature in deci kelvin
 * @ptype   int
 * \par Range:
 * [0, 100]
 */
#define PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK (100)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__PLAUSIBILITY_CFG_H_ */
