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
 * @file    plausibility_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION_CONFIGURATION
 * @prefix  PL
 *
 * @brief   Configuration of plausibility tolerances
 * @details Define the tolerances for the plausibility checks for
 *          - string voltage,
 *          - cell voltage, and
 *          - cell temperatures
 */

#ifndef FOXBMS__PLAUSIBILITY_CFG_H_
#define FOXBMS__PLAUSIBILITY_CFG_H_

/*========== Includes =======================================================*/
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * @brief   Maximum difference between pack voltage measurement from
 *          AFE and current sensor
 * @ptype   int
 * \par Range:
 * [0, 10000]
 */
#define PL_STRING_VOLTAGE_TOLERANCE_mV (3000)

/**
 * @brief   Maximum deviation between a single cell voltage measurement and the
 *          average cell voltage
 * @ptype   int
 * \par Range:
 * [0, 10000]
 */
#define PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV (300)

/**
 * @brief   Maximum deviation between a single cell temperature measurement and
 *          the average cell temperature in deci kelvin
 * @ptype   int
 * \par Range:
 * [0, 100]
 */
#define PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK (100)

/**
 * Maximum time between measurements before the
 * redundancy module raises an error because a
 * measurement is not updated anymore.
 *
 * The redundancy module will wait a maximum of this time for new current
 * values. If no new values are updated within this time frame it
 * will invalidate the measurement values.
 */
#define PL_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms (250u)

/**
 * Maximum time between current sensor high voltage, current
 * and power measurements before the redundancy module raises
 * an error because a measurement is not updated anymore.
 *
 * The redundancy module will wait a maximum of this
 * time for new values from the current sensor. If no
 * new values are updated within this time frame it will
 * validate the measurement values it has up to this point
 * if possible.
 */
#define PL_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms (300u)

/**
 * If both, the current sensor and the AFE measurement have no valid values
 * we try to construct the string voltage by replacing invalid cell voltage
 * measurements with the average cell voltage in this string. The result of
 * this estimation will be flagged as invalid if more than the number of
 * allowed invalid cell voltages are detected. The result will be marked as
 * valid if less then this number of cells are detected as invalid.
 */
#define PL_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES (5u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__PLAUSIBILITY_CFG_H_ */
