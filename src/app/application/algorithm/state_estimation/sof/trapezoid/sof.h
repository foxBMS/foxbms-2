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
 * @file    sof.h
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2020-10-07 (date of last update)
 * @ingroup APPLICATION
 * @prefix  SOX
 *
 * @brief   Header for SOX module, responsible for current derating calculation
 *
 */

#ifndef FOXBMS__SOF_H_
#define FOXBMS__SOF_H_

/*========== Includes =======================================================*/
#include "sof_cfg.h"

/*========== Macros and Definitions =========================================*/
/**
 * struct definition for 4 different values: in two current directions (charge,
 * discharge) for two use cases (peak and continuous)
 */
typedef struct SOF_CURRENT_LIMITS {
    float continuousChargeCurrent_mA;    /*!< maximum current for continues charging */
    float peakChargeCurrent_mA;          /*!< maximum current for peak charging */
    float continuousDischargeCurrent_mA; /*!< maximum current for continues discharging */
    float peakDischargeCurrent_mA;       /*!< maximum current for peak discharging */
} SOF_CURRENT_LIMITS_s;

/**
 * struct definition for calculating the linear SOF curve. The SOF curve is
 * voltage, temperature and charge/discharge dependent.
 */
typedef struct SOF_CURVE {
    float slopeLowTemperatureDischarge;   /*!< low temperature dependent derating slope for discharge current */
    float offsetLowTemperatureDischarge;  /*!< discharge current offset for low temperature dependent derating */
    float slopeHighTemperatureDischarge;  /*!< high temperature dependent derating slope for discharge current */
    float offsetHighTemperatureDischarge; /*!< discharge current offset for high temperature dependent derating */

    float slopeLowTemperatureCharge;   /*!< low temperature dependent derating slope for charge current */
    float offsetLowTemperatureCharge;  /*!< charge current offset for low temperature dependent derating */
    float slopeHighTemperatureCharge;  /*!< high temperature dependent derating slope for charge current */
    float offsetHighTemperatureCharge; /*!< charge current offset for high temperature dependent derating */

    float slopeUpperCellVoltage;  /*!< upper cell voltage derating slope for charge and discharge current */
    float offsetUpperCellVoltage; /*!< charge/discharge current offset for upper voltage derating */
    float slopeLowerCellVoltage;  /*!< lower cell voltage derating slope for charge and discharge current */
    float offsetLowerCellVoltage; /*!< charge/discharge current offset for lower voltage derating */
} SOF_CURVE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   initializes the area for SOF (where derating starts and is fully
 *          active).
 * @details Pseudocode for linear function parameter extraction with 2 points:
 *          - slope = (y2 - y1) / (x2-x1)
 *          - offset = y1 - (slope) * x1
 *          - function y= slope * x + offset
 */
extern void SOF_Init(void);

/**
 * @brief   triggers SOF calculation
 * @details TODO
 */
extern void SOF_Calculation(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_SOF_CalculateVoltageBasedCurrentLimit(
    int16_t minimumCellVoltage_mV,
    int16_t maximumCellVoltage_mV,
    SOF_CURRENT_LIMITS_s *pAllowedVoltageBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves);
extern void TEST_SOF_CalculateTemperatureBasedCurrentLimit(
    int16_t minimumCellTemperature_ddegC,
    int16_t maximumCellTemperature_ddegC,
    SOF_CURRENT_LIMITS_s *pAllowedTemperatureBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves);
extern SOF_CURRENT_LIMITS_s TEST_SOF_MinimumOfTwoSofValues(
    SOF_CURRENT_LIMITS_s voltageBasedLimits,
    SOF_CURRENT_LIMITS_s temperatureBasedLimits);
#endif /* UNITY_UNIT_TEST */
#endif /* FOXBMS__SOF_H_ */
