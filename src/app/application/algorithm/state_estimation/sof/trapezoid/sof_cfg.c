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
 * @file    sof_cfg.c
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2020-10-07 (date of last update)
 * @ingroup APPLICATION
 * @prefix  SOF
 *
 * @brief   SOF module configuration file
 *
 */

/*========== Includes =======================================================*/
#include "sof_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

const SOF_CONFIG_s sof_recommendedCurrent = {
    .maximumChargeCurrent_mA              = SOF_STRING_CURRENT_CONTINUOUS_CHARGE_mA,
    .maximumDischargeCurrent_mA           = SOF_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA,
    .limpHomeCurrent_mA                   = SOF_STRING_CURRENT_LIMP_HOME_mA,
    .cutoffLowTemperatureDischarge_ddegC  = SOF_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC,
    .limitLowTemperatureDischarge_ddegC   = SOF_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC,
    .cutoffLowTemperatureCharge_ddegC     = SOF_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC,
    .limitLowTemperatureCharge_ddegC      = SOF_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC,
    .cutoffHighTemperatureDischarge_ddegC = SOF_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC,
    .limitHighTemperatureDischarge_ddegC  = SOF_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC,
    .cutoffHighTemperatureCharge_ddegC    = SOF_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC,
    .limitHighTemperatureCharge_ddegC     = SOF_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC,
    .limitUpperCellVoltage_mV             = SOF_VOLTAGE_LIMIT_CHARGE_mV,
    .cutoffUpperCellVoltage_mV            = SOF_VOLTAGE_CUTOFF_CHARGE_mV,
    .limitLowerCellVoltage_mV             = SOF_VOLTAGE_LIMIT_DISCHARGE_mV,
    .cutoffLowerCellVoltage_mV            = SOF_VOLTAGE_CUTOFF_DISCHARGE_mV};

const SOF_CONFIG_s sof_maximumOperatingLimit = {
    .maximumChargeCurrent_mA              = SOF_MOL_STRING_CURRENT_CONTINUOUS_CHARGE_mA,
    .maximumDischargeCurrent_mA           = SOF_MOL_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA,
    .limpHomeCurrent_mA                   = SOF_MOL_STRING_CURRENT_LIMP_HOME_mA,
    .cutoffLowTemperatureDischarge_ddegC  = SOF_MOL_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC,
    .limitLowTemperatureDischarge_ddegC   = SOF_MOL_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC,
    .cutoffLowTemperatureCharge_ddegC     = SOF_MOL_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC,
    .limitLowTemperatureCharge_ddegC      = SOF_MOL_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC,
    .cutoffHighTemperatureDischarge_ddegC = SOF_MOL_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC,
    .limitHighTemperatureDischarge_ddegC  = SOF_MOL_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC,
    .cutoffHighTemperatureCharge_ddegC    = SOF_MOL_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC,
    .limitHighTemperatureCharge_ddegC     = SOF_MOL_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC,
    .limitUpperCellVoltage_mV             = SOF_MOL_VOLTAGE_LIMIT_CHARGE_mV,
    .cutoffUpperCellVoltage_mV            = SOF_MOL_VOLTAGE_CUTOFF_CHARGE_mV,
    .limitLowerCellVoltage_mV             = SOF_MOL_VOLTAGE_LIMIT_DISCHARGE_mV,
    .cutoffLowerCellVoltage_mV            = SOF_MOL_VOLTAGE_CUTOFF_DISCHARGE_mV};

const SOF_CONFIG_s sof_recommendedSafetyLimit = {
    .maximumChargeCurrent_mA              = SOF_RSL_STRING_CURRENT_CONTINUOUS_CHARGE_mA,
    .maximumDischargeCurrent_mA           = SOF_RSL_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA,
    .limpHomeCurrent_mA                   = SOF_RSL_STRING_CURRENT_LIMP_HOME_mA,
    .cutoffLowTemperatureDischarge_ddegC  = SOF_RSL_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC,
    .limitLowTemperatureDischarge_ddegC   = SOF_RSL_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC,
    .cutoffLowTemperatureCharge_ddegC     = SOF_RSL_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC,
    .limitLowTemperatureCharge_ddegC      = SOF_RSL_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC,
    .cutoffHighTemperatureDischarge_ddegC = SOF_RSL_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC,
    .limitHighTemperatureDischarge_ddegC  = SOF_RSL_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC,
    .cutoffHighTemperatureCharge_ddegC    = SOF_RSL_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC,
    .limitHighTemperatureCharge_ddegC     = SOF_RSL_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC,
    .limitUpperCellVoltage_mV             = SOF_RSL_VOLTAGE_LIMIT_CHARGE_mV,
    .cutoffUpperCellVoltage_mV            = SOF_RSL_VOLTAGE_CUTOFF_CHARGE_mV,
    .limitLowerCellVoltage_mV             = SOF_RSL_VOLTAGE_LIMIT_DISCHARGE_mV,
    .cutoffLowerCellVoltage_mV            = SOF_RSL_VOLTAGE_CUTOFF_DISCHARGE_mV};

const SOF_CONFIG_s sof_configMaximumSafetyLimit = {
    .maximumChargeCurrent_mA              = SOF_MSL_STRING_CURRENT_CONTINUOUS_CHARGE_mA,
    .maximumDischargeCurrent_mA           = SOF_MSL_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA,
    .limpHomeCurrent_mA                   = SOF_MSL_STRING_CURRENT_LIMP_HOME_mA,
    .cutoffLowTemperatureDischarge_ddegC  = SOF_MSL_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC,
    .limitLowTemperatureDischarge_ddegC   = SOF_MSL_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC,
    .cutoffLowTemperatureCharge_ddegC     = SOF_MSL_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC,
    .limitLowTemperatureCharge_ddegC      = SOF_MSL_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC,
    .cutoffHighTemperatureDischarge_ddegC = SOF_MSL_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC,
    .limitHighTemperatureDischarge_ddegC  = SOF_MSL_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC,
    .cutoffHighTemperatureCharge_ddegC    = SOF_MSL_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC,
    .limitHighTemperatureCharge_ddegC     = SOF_MSL_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC,
    .limitUpperCellVoltage_mV             = SOF_MSL_VOLTAGE_LIMIT_CHARGE_mV,
    .cutoffUpperCellVoltage_mV            = SOF_MSL_VOLTAGE_CUTOFF_CHARGE_mV,
    .limitLowerCellVoltage_mV             = SOF_MSL_VOLTAGE_LIMIT_DISCHARGE_mV,
    .cutoffLowerCellVoltage_mV            = SOF_MSL_VOLTAGE_CUTOFF_DISCHARGE_mV};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
