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
 * @file    beta.c
 * @author  foxBMS Team
 * @date    2020-01-17 (date of creation)
 * @updated 2021-03-22 (date of last update)
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  BETA
 *
 * @brief   Resistive divider used for measuring temperature
 *
 */

/*========== Includes =======================================================*/
#include "beta.h"

#include "foxmath.h"

/*========== Macros and Definitions =========================================*/

/** inverse temperature coefficient for ideal gas */
#define BETA_KELVIN (273.15f)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/** Defines for calculating the ADC voltage on the ends of the operating range.
 * The ADC voltage is calculated with the following formula:
 *
 * V_adc = ( ( V_supply * R_ntc ) / ( R + R_ntc ) )
 *
 * Depending on the position of the NTC in the voltage resistor (R_1/R_2),
 * different R_ntc values are used for the calculation.
 */
/**@{*/
#if BETA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
#define BETA_ADC_VOLTAGE_V_MAX_V \
    (float)((BETA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * BETA_ResistanceFromTemperature(1400)) / (BETA_ResistanceFromTemperature(1400) + BETA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define BETA_ADC_VOLTAGE_V_MIN_V \
    (float)((BETA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * BETA_ResistanceFromTemperature(-400)) / (BETA_ResistanceFromTemperature(-400) + BETA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#else /* BETA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define BETA_ADC_VOLTAGE_V_MIN_V \
    (float)((BETA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * BETA_ResistanceFromTemperature(1400)) / (BETA_ResistanceFromTemperature(1400) + BETA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define BETA_ADC_VOLTAGE_V_MAX_V \
    (float)((BETA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * BETA_ResistanceFromTemperature(-400)) / (BETA_ResistanceFromTemperature(-400) + BETA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t BETA_GetTemperatureFromBeta(uint16_t adcVoltage_mV) {
    int16_t temperature_ddegC = 0;
    float resistance_Ohm      = 0.0;
    float adcVoltage_V        = (float)adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > BETA_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < BETA_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if BETA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = BETA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((BETA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* BETA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / (V_supply - V_adc ) ) */
        resistance_Ohm = BETA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (BETA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* BETA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */
        /* Use BETA formula to compute temperature with resistance*/
        temperature_ddegC = BETA_TemperatureFromResistance(resistance_Ohm);
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t BETA_TemperatureFromResistance(float resistance_Ohm) {
    int16_t temperature_ddegC = 0;
    if (resistance_Ohm > 0.0f) {
        float temperature_degC = (1.0 / ((log(resistance_Ohm / BETA_R_REF_Ohm) / BETA_BETACOEFFICIENT) +
                                         (1.0 / (BETA_T_REF_C + BETA_KELVIN)))) -
                                 BETA_KELVIN;
        temperature_ddegC = (int16_t)(10.0f * temperature_degC); /* Convert to deci &deg;C */
    } else {
        /* Invalid value if as resistance can not be negative */
        temperature_ddegC = INT16_MIN;
    }
    return temperature_ddegC;
}

extern float BETA_ResistanceFromTemperature(int16_t temperature_ddegC) {
    float resistance_Ohm = 0.0f;
    resistance_Ohm       = BETA_R_REF_Ohm *
                     exp(BETA_BETACOEFFICIENT * ((1.0f / (((float)temperature_ddegC / 10.0f) + BETA_KELVIN)) -
                                                 (1.0f / (BETA_T_REF_C + BETA_KELVIN))));
    return resistance_Ohm;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
