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
 * @file    nxp_mc33775a_cfg.h
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  N77X
 *
 * @brief   Header for the configuration for the MC33775A analog front-end.
 * @details TODO
 */

#ifndef FOXBMS__NXP_MC33775A_CFG_H_
#define FOXBMS__NXP_MC33775A_CFG_H_

/*========== Includes =======================================================*/
#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"

#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define N77X_MAXIMUM_NUMBER_OF_SUPPORTED_CELL_MEASUREMENTS (14u)
#if SLV_USE_MUX_FOR_TEMP == false
#define N77X_MAXIMUM_NUMBER_OF_SUPPORTED_TEMP_SENSORS (8u)
#else
#define N77X_MAXIMUM_NUMBER_OF_SUPPORTED_TEMP_SENSORS (16u)
#endif

#if BS_NR_OF_CELL_BLOCKS_PER_MODULE > N77X_MAXIMUM_NUMBER_OF_SUPPORTED_CELL_MEASUREMENTS
#error "Number of cell blocks per module cannot be higher than maximum number of supported cells per IC"
#endif

#if BS_NR_OF_TEMP_SENSORS_PER_MODULE > N77X_MAXIMUM_NUMBER_OF_SUPPORTED_TEMP_SENSORS
#error "Number of temp sensors per module cannot be higher than maximum number of supported temp sensors per IC/Slave"
#endif

/* Alarm over and undervoltage checking: If the measured voltage on one of the
   enabled pins is over the OV or under one of the UV voltages, an alarm occurs */

/* Enable all pins for OV & UV alarm checks */
#define N77X_PRMM_VC_OV_UV_CFG (0x3fff)

#define N77X_OV_UV_CONVERSION_FACTOR (154.0e-6f)

/* Alarm thresholds in (V / 154uV) */
/** Overvoltage threshold */
#define N77X_PRMM_VC_OV_TH_CFG ((uint16_t)((BC_VOLTAGE_MAX_MOL_mV / 1000.0f) / N77X_OV_UV_CONVERSION_FACTOR))
/** Individual undervoltage threshold */
#define N77X_PRMM_VC_UV0_TH_CFG ((uint16_t)((BC_VOLTAGE_MIN_MOL_mV / 1000.0f) / N77X_OV_UV_CONVERSION_FACTOR))
/** Global undervoltage threshold */
#define N77X_PRMM_VC_UV1_TH_CFG ((uint16_t)((BC_VOLTAGE_MIN_MOL_mV / 1000.0f) / N77X_OV_UV_CONVERSION_FACTOR))

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC33775A_CFG_H_ */
