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
 * @file    redundancy_cfg.h
 * @author  foxBMS Team
 * @date    2026-07-22 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION_CONFIGURATION
 * @prefix  MCR
 *
 * @brief   Configuration of redundant measurement deviation tolerances
 * @details Define the tolerances for the redundant measurement deviation
 *          checks for
 *          - cell voltage, and
 *          - cell temperatures
 */

#ifndef FOXBMS__REDUNDANCY_CFG_H_
#define FOXBMS__REDUNDANCY_CFG_H_

/*========== Includes =======================================================*/

/*========== Macros and Definitions =========================================*/

/**
 * Maximum time between AFE measurements before the
 * redundancy module raises an error because a
 * measurement is not updated anymore.
 *
 * The redundancy module will wait a maximum of this
 * time for new values from the base AFE measurement and
 * AFE redundant measurements. If no new values are updated
 * from both measurement sources within this time frame
 * it will validate the measurement values it has up to
 * this point if possible.
 */
#define MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms (250u)

/**
 * @brief   Maximum difference between redundant cell voltage measurement
 * @ptype   int
 * \par Range:
 * [0, 10000]
 */
#define MCR_CELL_VOLTAGE_TOLERANCE_mV (10)

/**
 * @brief   Maximum difference between redundant cell temperature measurements
 *          in deci kelvin
 * @ptype   int
 * \par Range:
 * [0, 100]
 */
#define MCR_CELL_TEMPERATURE_TOLERANCE_dK (50)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__REDUNDANCY_CFG_H_ */
