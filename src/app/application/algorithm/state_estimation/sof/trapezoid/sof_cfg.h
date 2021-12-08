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
 * @file    sof_cfg.h
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup APPLICATION_CONFIGURATION
 * @prefix  SOF
 *
 * @brief   Header for SOF configuration
 *
 */

#ifndef FOXBMS__SOF_CFG_H_
#define FOXBMS__SOF_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/**
 * Maximum current in mA in charge direction that a string can sustain.
 * Normally set once for the specific battery cell from data sheet
 */
#define SOF_STRING_CURRENT_CONTINUOUS_CHARGE_mA (120000.00f)

/**
 * Different alarm levels for the maximum continuous charge current in mA.
 * @{
 */
#define SOF_MOL_STRING_CURRENT_CONTINUOUS_CHARGE_mA (10000.00f)
#define SOF_RSL_STRING_CURRENT_CONTINUOUS_CHARGE_mA (11500.00f)
#define SOF_MSL_STRING_CURRENT_CONTINUOUS_CHARGE_mA (12200.00f)
/**@}*/

/**
 * Maximum current in mA in discharge direction that a string can deliver.
 * Normally set once for the specific battery cell from data sheet.
 */
#define SOF_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA (120000.00f)

/**
 * Different alarm levels for the maximum continuous string discharge current in mA.
 * @{
 */
#define SOF_MOL_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA (10000.00f)
#define SOF_RSL_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA (11500.00f)
#define SOF_MSL_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA (12200.00f)
/**@}*/

/**
 * Current in mA that the string should be able to discharge when in
 * limp home mode, i.e., something noncritical went wrong but it should be able
 * to drive home. The value is chosen by the system engineer.
 */
#define SOF_STRING_CURRENT_LIMP_HOME_mA (20000.00f)

/**
 * Different alarm levels for the limp home current in mA
 * @{
 */
#define SOF_MOL_STRING_CURRENT_LIMP_HOME_mA (20000.00f)
#define SOF_RSL_STRING_CURRENT_LIMP_HOME_mA (20000.00f)
#define SOF_MSL_STRING_CURRENT_LIMP_HOME_mA (20000.00f)
/**@}*/

/**
 * Cold temperature in deci &deg;C where the derating of maximum discharge
 * current starts, i.e., below this temperature battery pack should not deliver
 * full discharge current.
 */
#define SOF_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC (0)

/**
 * Different alarm levels for the cold temperature in deci &deg;C where the
 * derating of maximum discharge current starts, i.e., below this temperature
 * battery pack should not deliver full discharge current.
 * @{
 */
#define SOF_MOL_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC (0)
#define SOF_RSL_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC (0)
#define SOF_MSL_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC (0)
/**@}*/

/**
 * Cold temperature in deci &deg;C where the derating of maximum discharge
 * current is fully applied, i.e., below this temperature battery pack should
 * not deliver any current in discharge direction.
 */
#define SOF_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC (-100)

/**
 * Different alarm levels for the cold temperature in deci &deg;C where the
 * derating of maximum discharge current is fully applied. Below this
 * temperature the battery pack should not deliver any current in discharge
 * direction.
 * @{
 */
#define SOF_MOL_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC (-100)
#define SOF_RSL_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC (-100)
#define SOF_MSL_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC (-100)
/**@}*/

/**
 * Cold temperature in deci &deg;C where the derating of maximum charge current
 * starts, i.e., below this temperature battery pack should not deliver full
 * charge current.
 */
#define SOF_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC (0)

/**
 * Different alarm levels for the cold temperature in deci &deg;C where the
 * derating of maximum charge current starts, i.e., below this temperature
 * battery pack should not deliver full charge current
 * @{
 */
#define SOF_MOL_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC (0)
#define SOF_RSL_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC (0)
#define SOF_MSL_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC (0)
/**@}*/

/**
 * Cold temperature in deci &deg;C where the derating of maximum charge current
 * is fully applied, i.e., below this temperature battery pack should not
 * deliver any current in charge direction.
 */
#define SOF_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC (-100)

/**
 * Different alarm levels for the cold temperature in deci &deg;C where the
 * derating of maximum charge current is fully applied. Below this temperature
 * the battery pack should not deliver any current in charge direction.
 * @{
 */
#define SOF_MOL_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC (-100)
#define SOF_RSL_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC (-100)
#define SOF_MSL_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC (-100)
/**@}*/

/**
 * Hot temperature in deci &deg;C where the derating of maximum discharge
 * current starts, i.e., above this temperature battery pack should not deliver
 * full discharge current.
 */
#define SOF_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC (450)

/**
 * Different alarm levels for the high temperature in deci &deg;C where the
 * derating of maximum discharge current starts, i.e., above this temperature
 * battery pack should not deliver full discharge current
 * @{
 */
#define SOF_MOL_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC (450)
#define SOF_RSL_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC (450)
#define SOF_MSL_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC (450)
/**@}*/

/**
 * Hot temperature in deci &deg;C where the derating of maximum discharge
 * current is fully applied, i.e., above this temperature battery pack should
 * not deliver any current in discharge direction.
 */
#define SOF_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC (550)

/**
 * Different alarm levels for the high temperature in deci &deg;C where the
 * derating of maximum discharge current is fully applied. Above this
 * temperature the battery pack should not deliver any current in discharge
 * direction.
 * @{
 */
#define SOF_MOL_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC (550)
#define SOF_RSL_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC (550)
#define SOF_MSL_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC (550)
/**@}*/

/**
 * Hot temperature in deci &deg;C where the derating of maximum charge current
 * starts, i.e., above this temperature battery pack should not deliver full
 * charge current.
 */
#define SOF_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC (450)

/**
 * Different alarm levels for the high temperature in deci &deg;C where the
 * derating of maximum charge current starts, i.e., above this temperature
 * battery pack should not deliver full charge current
 * @{
 */
#define SOF_MOL_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC (450)
#define SOF_RSL_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC (450)
#define SOF_MSL_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC (450)
/**@}*/

/**
 * Hot temperature in deci &deg;C  where the derating of maximum charge current
 * is fully applied, i.e., above this temperature battery pack should not
 * deliver any current in charge direction.
 */
#define SOF_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC (550)

/**
 * Different alarm levels for the high temperature in deci &deg;C  where the
 * derating of maximum charge current is fully applied. Above this temperature
 * the battery pack should not deliver any current in charge direction.
 * @{
 */
#define SOF_MOL_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC (550)
#define SOF_RSL_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC (550)
#define SOF_MSL_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC (550)
/**@}*/

/**
 * Above this voltage value battery pack should not be exposed to full current in charge direction.
 */
#define SOF_VOLTAGE_CUTOFF_CHARGE_mV (2400)

/**
 * Different alarm levels for the upper voltage limit where the derating of
 * maximum charge current starts. Above this value the battery pack
 * should not deliver full current in charge direction.
 * @{
 */
#define SOF_MOL_VOLTAGE_CUTOFF_CHARGE_mV (2400)
#define SOF_RSL_VOLTAGE_CUTOFF_CHARGE_mV (2400)
#define SOF_MSL_VOLTAGE_CUTOFF_CHARGE_mV (2400)
/**@}*/

/**
 * Above this voltage value battery pack should not be exposed to any current in charge direction.
 */
#define SOF_VOLTAGE_LIMIT_CHARGE_mV (2550)

/**
 * Different alarm levels for the upper voltage limit where the derating of
 * maximum charge current is fully applied. Above this value the battery pack
 * should not deliver any current in charge direction.
 * @{
 */
#define SOF_MOL_VOLTAGE_LIMIT_CHARGE_mV (2550)
#define SOF_RSL_VOLTAGE_LIMIT_CHARGE_mV (2550)
#define SOF_MSL_VOLTAGE_LIMIT_CHARGE_mV (2550)
/**@}*/

/**
 * Below this voltage value battery pack should not deliver full current in discharge direction.
 */
#define SOF_VOLTAGE_CUTOFF_DISCHARGE_mV (2000)

/**
 * Different alarm levels for the lower voltage limit where the derating of
 * maximum discharge current starts. Below this value the battery pack
 * should not deliver full current in discharge direction.
 * @{
 */
#define SOF_MOL_VOLTAGE_CUTOFF_DISCHARGE_mV (2000)
#define SOF_RSL_VOLTAGE_CUTOFF_DISCHARGE_mV (2000)
#define SOF_MSL_VOLTAGE_CUTOFF_DISCHARGE_mV (2000)
/**@}*/

/**
 * Below this voltage value battery pack should not deliver any current in discharge direction.
 */
#define SOF_VOLTAGE_LIMIT_DISCHARGE_mV (1750)

/**
 * Different alarm levels for the lower voltage limit where the derating of
 * maximum discharge is fully applied. Below this value the battery pack
 * should not deliver any current in discharge direction.
 * @{
 */
#define SOF_MOL_VOLTAGE_LIMIT_DISCHARGE_mV (1750)
#define SOF_RSL_VOLTAGE_LIMIT_DISCHARGE_mV (1750)
#define SOF_MSL_VOLTAGE_LIMIT_DISCHARGE_mV (1750)
/**@}*/

/**
 * structure for configuration of SoF Calculation
 */
typedef struct SOF_CONFIG {
    /** Current derating limits @{ */
    float maximumDischargeCurrent_mA;
    float maximumChargeCurrent_mA;
    float limpHomeCurrent_mA;
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

/**
 * Configuration values for the SOF window of the different alarm levels (MOL,
 * RSL and MSL)
 * @{
 */
extern const SOF_CONFIG_s sof_maximumOperatingLimit;
extern const SOF_CONFIG_s sof_recommendedSafetyLimit;
extern const SOF_CONFIG_s sof_configMaximumSafetyLimit;
/**@}*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SOF_CFG_H_ */
