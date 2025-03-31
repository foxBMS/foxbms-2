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
 * @file    plausibility.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Plausibility checks for cell voltage and cell temperatures
 * @details TODO
 *
 */

#ifndef FOXBMS__PLAUSIBILITY_H_
#define FOXBMS__PLAUSIBILITY_H_

/*========== Includes =======================================================*/
#include "plausibility_cfg.h"

#include "database.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Pack voltage plausibility check between LTC and current sensor values
 *
 * @param  voltageAfe_mV            pack voltage measured by AFE
 * @param  voltageCurrentSensor_mV  pack voltage measured by current sensor
 *
 * @return #STD_OK if pack voltage valid, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e PL_CheckStringVoltage(int32_t voltageAfe_mV, int32_t voltageCurrentSensor_mV);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

/**
 * @brief Cell voltage plausibility check between two redundant cell voltage measurement values
 *
 * @param[in]  baseCellVoltage         cell voltage from base measurement
 * @param[in]  redundancy0CellVoltage  cell voltage from redundant measurement
 * @param[out] pCellVoltage            output cell voltage after plausibility check
 *
 * @return #STD_OK if cell voltage valid, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e PL_CheckCellVoltage(
    int16_t baseCellVoltage,
    int16_t redundancy0CellVoltage,
    int16_t *pCellVoltage);

/**
 * @brief Cell temperature plausibility check between two redundant cell temperature measurement values
 *
 * @param[in]  baseCellTemperature         cell temperature from base measurement
 * @param[in]  redundancy0CellTemperature  cell temperature from redundant measurement
 * @param[out] pCellTemperature            output cell temperature after plausibility check
 *
 * @return #STD_OK if cell voltage valid, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e PL_CheckCellTemperature(
    int16_t baseCellTemperature,
    int16_t redundancy0CellTemperature,
    int16_t *pCellTemperature);

/**
 * @brief  Cell voltage spread plausibility check
 *
 * @param[in,out]  pCellVoltages     pointer to cell voltage database entry
 * @param[in]  pMinMaxAverageValues  pointer to minimum/maximum/average database entry
 *
 * @return #STD_OK if no issue detected, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e PL_CheckVoltageSpread(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

/**
 * @brief  Cell temperature spread plausibility check
 *
 * @param[in,out]  pCellTemperatures pointer to cell temperature database entry
 * @param[in]  pMinMaxAverageValues  pointer to minimum/maximum/average database entry
 *
 * @return #STD_OK if no issue detected, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e PL_CheckTemperatureSpread(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);

#endif /* FOXBMS__PLAUSIBILITY_H_ */
