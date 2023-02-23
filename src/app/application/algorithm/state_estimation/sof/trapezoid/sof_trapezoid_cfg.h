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
 * @file    sof_trapezoid_cfg.h
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup APPLICATION_CONFIGURATION
 * @prefix  SOF
 *
 * @brief   Header for SOF configuration
 *
 */

#ifndef FOXBMS__SOF_TRAPEZOID_CFG_H_
#define FOXBMS__SOF_TRAPEZOID_CFG_H_

/*========== Includes =======================================================*/

#include "battery_cell_cfg.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * Maximum current in mA in charge direction that a string can sustain.
 * Normally set once for the specific battery cell from data sheet
 */
#define SOF_STRING_CURRENT_CONTINUOUS_CHARGE_mA ((float_t)BC_CURRENT_MAX_CHARGE_MOL_mA)

/**
 * Maximum current in mA in discharge direction that a string can deliver.
 * Normally set once for the specific battery cell from data sheet.
 */
#define SOF_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA ((float_t)BC_CURRENT_MAX_DISCHARGE_MOL_mA)

/**
 * Current in mA that the string should be able to discharge when in
 * limp home mode, i.e., something noncritical went wrong but it should be able
 * to drive home. The value is chosen by the system engineer.
 */
#define SOF_STRING_CURRENT_LIMP_HOME_mA (20000.00f)

/**
 * Cold temperature in deci &deg;C where the derating of maximum discharge
 * current starts, i.e., below this temperature battery pack should not deliver
 * full discharge current.
 */
#define SOF_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC (BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC)

/**
 * Cold temperature in deci &deg;C where the derating of maximum discharge
 * current is fully applied, i.e., below this temperature battery pack should
 * not deliver any current in discharge direction.
 */
#define SOF_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC (BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC)

/**
 * Cold temperature in deci &deg;C where the derating of maximum charge current
 * starts, i.e., below this temperature battery pack should not deliver full
 * charge current.
 */
#define SOF_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC (BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC)

/**
 * Cold temperature in deci &deg;C where the derating of maximum charge current
 * is fully applied, i.e., below this temperature battery pack should not
 * deliver any current in charge direction.
 */
#define SOF_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC (BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC)

/**
 * Hot temperature in deci &deg;C where the derating of maximum discharge
 * current starts, i.e., above this temperature battery pack should not deliver
 * full discharge current.
 */
#define SOF_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC (BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC)

/**
 * Hot temperature in deci &deg;C where the derating of maximum discharge
 * current is fully applied, i.e., above this temperature battery pack should
 * not deliver any current in discharge direction.
 */
#define SOF_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC (BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC)

/**
 * Hot temperature in deci &deg;C where the derating of maximum charge current
 * starts, i.e., above this temperature battery pack should not deliver full
 * charge current.
 */
#define SOF_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC (BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC)

/**
 * Hot temperature in deci &deg;C  where the derating of maximum charge current
 * is fully applied, i.e., above this temperature battery pack should not
 * deliver any current in charge direction.
 */
#define SOF_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC (BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC)

/**
 * Above this voltage value battery pack should not be exposed to full current in charge direction.
 */
#define SOF_VOLTAGE_CUTOFF_CHARGE_mV (BC_VOLTAGE_MAX_MOL_mV)

/**
 * Above this voltage value battery pack should not be exposed to any current in charge direction.
 */
#define SOF_VOLTAGE_LIMIT_CHARGE_mV (BC_VOLTAGE_MAX_RSL_mV)

/**
 * Below this voltage value battery pack should not deliver full current in discharge direction.
 */
#define SOF_VOLTAGE_CUTOFF_DISCHARGE_mV (BC_VOLTAGE_MIN_MOL_mV)

/**
 * Below this voltage value battery pack should not deliver any current in discharge direction.
 */
#define SOF_VOLTAGE_LIMIT_DISCHARGE_mV (BC_VOLTAGE_MIN_RSL_mV)

/**
 * structure for configuration of SoF Calculation
 */
typedef struct {
    /** Current derating limits @{ */
    float_t maximumDischargeCurrent_mA;
    float_t maximumChargeCurrent_mA;
    float_t limpHomeCurrent_mA;
    /**@}*/

    /** Low temperature derating limits @{ */
    int16_t cutoffLowTemperatureDischarge_ddegC;
    int16_t limitLowTemperatureDischarge_ddegC;
    int16_t cutoffLowTemperatureCharge_ddegC;
    int16_t limitLowTemperatureCharge_ddegC;
    /**@}*/

    /** High temperature derating limits @{ */
    int16_t cutoffHighTemperatureDischarge_ddegC;
    int16_t limitHighTemperatureDischarge_ddegC;
    int16_t cutoffHighTemperatureCharge_ddegC;
    int16_t limitHighTemperatureCharge_ddegC;
    /**@}*/

    /** Cell voltage derating limits @{ */
    int16_t cutoffUpperCellVoltage_mV;
    int16_t limitUpperCellVoltage_mV;
    int16_t cutoffLowerCellVoltage_mV;
    int16_t limitLowerCellVoltage_mV;
    /**@}*/
} SOF_CONFIG_s;

/*========== Extern Constant and Variable Declarations ======================*/

/**
 * Configuration values for the SOF window of the recommended battery current
 */
extern const SOF_CONFIG_s sof_recommendedCurrent;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__SOF_TRAPEZOID_CFG_H_ */
