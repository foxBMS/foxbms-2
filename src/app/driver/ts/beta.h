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
 * @file    beta.h
 * @author  foxBMS Team
 * @date    2020-01-17 (date of creation)
 * @updated 2021-02-24 (date of last update)
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  BETA
 *
 * @brief   Resistive divider used for measuring temperature
 *
 *          V_supply
 *            --.--
 *              |
 *            +-.-+
 *            |   |
 *            |   |  R_1
 *            |   |
 *            +-.-+
 *              |
 *              .--- V_adc
 *              |
 *            +-.-+
 *            |   |
 *            |   |  R_2
 *            |   |
 *            +-.-+
 *              |
 *            --.--
 *             GND
 *
 */

#ifndef FOXBMS__BETA_H_
#define FOXBMS__BETA_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/**
 * Position of the NTC in the voltage resistor
 * true: NTC is positioned above the voltage tap for the ADC voltage.
 * This equals resistor R_1 in the above circuit diagram
 *
 * false: NTC is positioned below the voltage tap for the ADC voltage.
 * This equals resistor R2 in the above circuit diagram
 */
#define BETA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 (false)

/** Resistor divider supply voltage in volt */
#define BETA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V (4.096f)

/**
 * Resistance value of the other resistor (not the NTC) in the resistor
 * divider in kOhm.
 */
#define BETA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm (10000.0f)

/** Reference temperature */
#define BETA_T_REF_C (25.0f)

/** NTC resistance at reference temperature */
#define BETA_R_REF_Ohm (10000.0f)

/** BEta coefficient of the NTC */
#define BETA_BETACOEFFICIENT (3984.0f)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   returns temperature based on measured ADC voltage
 * @param   adcVoltage_mV   voltage in mV
 * @return  corresponding temperature in deci &deg;C or FLT_MAX/FLT_MIN if NTC
 *          is shorted or got disconnected. The caller of this functions needs
 *          to check for these return values to prevent invalid data.
 */
extern int16_t BETA_GetTemperatureFromBeta(uint16_t adcVoltage_mV);

/**
 * @brief   returns temperature corresponding to NTC resistance
 * @param   resistance_Ohm  resistance in Ohm
 * @return  corresponding temperature in deci &deg;C
 *
 */
extern int16_t BETA_TemperatureFromResistance(float resistance_Ohm);

/**
 * @brief   returns NTC resistance corresponding to temperature,
 *          used to compute Vmin and Vmax of the divider
 * @param   temperature_ddegC in deci &deg;C
 * @return  corresponding resistance in Ohm
 *
 */
extern float BETA_ResistanceFromTemperature(int16_t temperature_ddegC);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__BETA_H_ */
