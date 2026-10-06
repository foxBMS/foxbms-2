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
 * @file    bms-values.h
 * @author  foxBMS Team
 * @date    2024-07-23 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  BMSVL
 *
 * @brief   Manage system values
 * @details Handle updating and management of system-wide battery values.
 */

#ifndef FOXBMS__BMS_VALUES_H_
#define FOXBMS__BMS_VALUES_H_

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"

#ifdef UNITY_UNIT_TEST
#include "database_cfg.h"
#endif /* UNITY_UNIT_TEST */

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * @brief   State of the BMS values module
 */
typedef struct {
    uint32_t lastStringPowerTimestamp[BS_NR_OF_STRINGS];
    uint32_t lastStringCurrentTimestamp[BS_NR_OF_STRINGS];
} BMSVL_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Initialize the module for the validation of the measured values.
 * @details Mark all measured values as invalid, as no valid measurements are
 *          available directly after startup. This affects:
 *          - all cell voltages, module voltages and string voltages of the
 *            validated cell voltage database entry
 *          - all cell temperatures of the validated cell temperature database
 *            entry
 *          - the string voltage, string current and string power as well as
 *            the pack current, battery voltage, high voltage bus voltage and
 *            pack power of the validated pack values database entry
 *          Afterwards, the invalidated database entries are written to the
 *          database, so that all consumers read invalid values until the first
 *          successful validation.
 * @pre     This function must be called once before #BMSVL_UpdateSystemValues
 *          is called for the first time.
 */
extern void BMSVL_Initialize(void);

/**
 * @brief   Validate and update all BMS-behavior relevant system values.
 * @details This function is the central entry point for the periodic
 *          validation of all measured system values. It performs the
 *          following steps:
 *          - read all required database entries (current, system voltages,
 *            power and the minimum/maximum values)
 *          - make the base cell voltage and cell temperature measurements
 *            available as validated measurements:
 *            - if redundant voltage/temperature measurement is enabled
 *              (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT set to 1), the
 *              measurements are validated by the measurement redundancy
 *              module
 *            - otherwise, the base measurements are simply copied into the
 *              validated database entries
 *          - derive all cell-related minimum and maximum values
 *          - validate the current, string voltage, battery voltage, high
 *            voltage bus and power measurements
 *          - write the validated database entries back to the database. In
 *            the redundant case, the validated cell voltage and cell
 *            temperature entries are only written if their validation was
 *            successful.
 * @pre     The AFE driver must have updated the base measurement database
 *          entries at least once, otherwise no validated cell voltage or cell
 *          temperature values are available.
 * @note    This function is intended to be called cyclically by the task
 *          scheduler (1ms/10ms task, depending on the configuration).
 */
extern void BMSVL_UpdateSystemValues(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
/* clang-format off */
extern void TEST_BMSVL_DeriveMinimumMaximumValues(void);
extern STD_RETURN_TYPE_e TEST_BMSVL_CalculateCellVoltageMinMaxAverage(const DATA_BLOCK_CELL_VOLTAGE_s *const pValidatedVoltages, DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);
extern STD_RETURN_TYPE_e TEST_BMSVL_CalculateCellTemperatureMinMaxAverage(const DATA_BLOCK_CELL_TEMPERATURE_s *const pValidatedTemperatures, DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues);
#if (defined(FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT) && (FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0))
extern void TEST_BMSVL_CopyBaseMeasurementsToValidatedMeasurements(DATA_BLOCK_CELL_VOLTAGE_s *pTableCellVoltages, DATA_BLOCK_CELL_TEMPERATURE_s *pTableCellTemperatures);
#endif /* FOXBMS_HAVE_REDUNDANT_V_T_MEASUREMENT == 0 */
/* clang-format on */
#endif /* UNITY_UNIT_TEST */

#endif /* FOXBMS__BMS_VALUES_H_ */
