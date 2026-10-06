/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    redundancy.h
 * @author  foxBMS Team
 * @date    2020-07-31 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  MRC
 *
 * @brief   Header files for handling redundancy between redundant cell voltage
 *          and cell temperature measurements
 * @details TODO
 *
 */

#ifndef FOXBMS__REDUNDANCY_H_
#define FOXBMS__REDUNDANCY_H_

/*========== Includes =======================================================*/
#include "database_cfg.h"

#include "fstd_types.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

typedef struct {
    bool updatedCellVoltageDatabaseEntry; /*!< true if validated cell voltage database entry shall be updated */
    bool updatedTemperatureDatabaseEntry; /*!< true if validated cell temperature database entry shall be updated */
} MRC_REQUIRED_UPDATES_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Validate the measurement between redundant measurement
 *          values for cell voltage and cell temperature
 * @param[in] pTableCellVoltages     cell voltage database entry
 * @param[in] pTableCellTemperatures cell temperature database entry
 * @return  #MRC_REQUIRED_UPDATES_s structure with information about which
 *          database entries need to be updated
 */
extern MRC_REQUIRED_UPDATES_s MRC_ValidateAfeMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_MRC_ValidateCellVoltageMeasurement(
    DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0);
extern bool TEST_MRC_ValidateCellTemperatureMeasurement(
    DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0);
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellVoltage(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageBase,
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltageRedundancy0,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellVoltageValidation(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltage,
    DATA_BLOCK_CELL_VOLTAGE_s *pValidatedVoltages);
extern STD_RETURN_TYPE_e TEST_MRC_ValidateCellTemperature(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureBase,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatureRedundancy0,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperatures);
extern STD_RETURN_TYPE_e TEST_MRC_UpdateCellTemperatureValidation(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperature,
    DATA_BLOCK_CELL_TEMPERATURE_s *pValidatedTemperature);
extern void TEST_MRC_SetMrcState(
    uint32_t lastBaseCellVoltageTimestamp,
    uint32_t lastRedundancy0CellVoltageTimestamp,
    uint32_t lastBaseCellTemperatureTimestamp,
    uint32_t lastRedundancy0CellTemperatureTimestamp);

#endif /* UNITY_UNIT_TEST */

#endif /* FOXBMS__REDUNDANCY_H_ */
