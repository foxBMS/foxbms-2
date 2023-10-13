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
 * @file    vishay_ntcle413e2103f102l.c
 * @author  foxBMS Team
 * @date    2021-11-03 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 *
 */

/*========== Includes =======================================================*/
#include "vishay_ntcle413e2103f102l.h"

#include "fassert.h"
#include "foxmath.h"
#include "temperature_sensor_defs.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** LUT filled from higher resistance to lower resistance */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_ntcle413e2103f102lLut[] = {
    {-40, 190953.0f}, {-35, 145953.0f}, {-30, 112440.0f}, {-25, 87285.0f}, {-20, 68260.0f}, {-15, 53762.0f},
    {-10, 42636.0f},  {-5, 34038.0f},   {0, 27348.0f},    {5, 22108.0f},   {10, 17979.0f},  {15, 17706.0f},
    {20, 12904.0f},   {25, 10000.0f},   {30, 8310.8f},    {35, 6941.1f},   {40, 5824.9f},   {45, 4910.6f},
    {50, 4158.3f},    {55, 3536.2f},    {60, 3019.7f},    {65, 2588.8f},   {70, 2228.0f},   {75, 1924.6f},
    {80, 1668.4f},    {85, 1451.3f},    {90, 1266.7f},    {95, 1109.2f},   {100, 974.26f},  {105, 858.33f}};

/** size of the #ts_ntcle413e2103f102lLut LUT */
static const uint16_t ts_ntcle413e2103f102lLutSize = sizeof(ts_ntcle413e2103f102lLut) /
                                                     sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

/*========== Extern Constant and Variable Definitions =======================*/
/**
 * Defines for calculating the ADC voltage on the ends of the operating range.
 * The ADC voltage is calculated with the following formula:
 *
 * V_adc = ( ( V_supply * R_ntc ) / ( R + R_ntc ) )
 *
 * Depending on the position of the NTC in the voltage resistor (R_1/R_2),
 * different R_ntc values are used for the calculation.
 * @{
 */
#if defined(TS_VISHAY_NTCLE413E2103F102L_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_VISHAY_NTCLE413E2103F102L_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
#define TS_VISHAY_NTCLE413E2103F102L_ADC_VOLTAGE_V_MAX_V \
    ((float_t)((TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle413e2103f102lLut[ts_ntcle413e2103f102lLutSize-1u].resistance_Ohm) / (ts_ntcle413e2103f102lLut[ts_ntcle413e2103f102lLutSize-1u].resistance_Ohm+TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_VISHAY_NTCLE413E2103F102L_ADC_VOLTAGE_V_MIN_V \
    ((float_t)((TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle413e2103f102lLut[0u].resistance_Ohm) / (ts_ntcle413e2103f102lLut[0u].resistance_Ohm+TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#else /* TS_VISHAY_NTCLE413E2103F102L_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 is false */
#define TS_VISHAY_NTCLE413E2103F102L_ADC_VOLTAGE_V_MIN_V \
    ((float_t)((TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle413e2103f102lLut[ts_ntcle413e2103f102lLutSize-1u].resistance_Ohm) / (ts_ntcle413e2103f102lLut[ts_ntcle413e2103f102lLutSize-1u].resistance_Ohm+TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_VISHAY_NTCLE413E2103F102L_ADC_VOLTAGE_V_MAX_V \
    ((float_t)((TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle413e2103f102lLut[0u].resistance_Ohm) / (ts_ntcle413e2103f102lLut[0u].resistance_Ohm+TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Vis02GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    int16_t temperature_ddegC = INT16_MIN;
    float_t adcVoltage_V      = (float_t)adcVoltage_mV / TS_SCALING_FACTOR_1V_IN_MV_FLOAT; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_VISHAY_NTCLE413E2103F102L_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_VISHAY_NTCLE413E2103F102L_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if defined(TS_VISHAY_NTCLE413E2103F102L_POSITION_IN_RESISTOR_DIVIDER_IS_R_1) && \
    (TS_VISHAY_NTCLE413E2103F102L_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true)
        /* formula: R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        const float_t resistance_Ohm =
            TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
            ((TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* TS_VISHAY_NTCLE413E2103F102L_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 is false */
        /* formula: R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        const float_t resistance_Ohm =
            TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
            (adcVoltage_V / (TS_VISHAY_NTCLE413E2103F102L_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* TS_VISHAY_NTCLE413E2103F102L_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 0u; i < ts_ntcle413e2103f102lLutSize; i++) {
            if (resistance_Ohm < ts_ntcle413e2103f102lLut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) ||  /* measured resistance > maximum LUT resistance */
              (between_low >= ts_ntcle413e2103f102lLutSize))) { /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_ntcle413e2103f102lLut[between_low].resistance_Ohm,
                (float_t)ts_ntcle413e2103f102lLut[between_low].temperature_ddegC,
                ts_ntcle413e2103f102lLut[between_high].resistance_Ohm,
                (float_t)ts_ntcle413e2103f102lLut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Vis02GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    (void)adcVoltage_mV;
    FAS_ASSERT(FAS_TRAP);
    int16_t temperature_ddegC = 0;
    /* TODO this is not implemented */
    return temperature_ddegC;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
