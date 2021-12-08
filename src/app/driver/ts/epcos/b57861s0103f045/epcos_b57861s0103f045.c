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
 * @file    epcos_b57861s0103f045.c
 * @author  foxBMS Team
 * @date    2018-10-30 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 *
 */

/*========== Includes =======================================================*/
#include "epcos_b57861s0103f045.h"

#include "foxmath.h"
#include "temperature_sensor_defs.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/* clang-format off */
/** LUT filled from higher resistance to lower resistance */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_b57861s0103f045Lut[] = {
    { -550, 963000.00f },
    { -500, 670100.00f },
    { -450, 471700.00f },
    { -400, 336500.00f },
    { -350, 242600.00f },
    { -300, 177000.00f },
    { -250, 130400.00f },
    { -200, 97070.00f },
    { -150, 72930.00f },
    { -100, 55330.00f },
    {  -50, 42320.00f },
    {   0, 32650.00f },
    {   50, 25390.00f },
    {  100, 19900.00f },
    {  150, 15710.00f },
    {  200, 12490.00f },
    {  250, 10000.00f },
    {  300, 8057.00f },
    {  350, 6531.00f },
    {  400, 5327.00f },
    {  450, 4369.00f },
    {  500, 3603.00f },
    {  550, 2986.00f },
    {  600, 2488.00f },
    {  650, 2083.00f },
    {  700, 1752.00f },
    {  750, 1481.00f },
    {  800, 1258.00f },
    {  850, 1072.00f },
    {  900, 917.70f },
    {  950, 788.50f },
    { 1000, 680.00f },
    { 1050, 588.60f },
    { 1100, 511.20f },
    { 1150, 445.40f },
    { 1200, 389.30f },
    { 1250, 341.70f },
    { 1300, 300.90f },
    { 1350, 265.40f },
    { 1400, 234.80f },
    { 1450, 208.30f },
    { 1500, 185.30f },
    { 1550, 165.30f }
};
/* clang-format on */

/** size of the #ts_b57861s0103f045Lut LUT */
static uint16_t ts_b57861s0103f045LutSize = sizeof(ts_b57861s0103f045Lut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

/*========== Extern Constant and Variable Definitions =======================*/
/**
 * @brief   Defines for calculating the ADC voltage on the ends of the operating range.
 * @details The ADC voltage is calculated with the following formula:
 *
 *          V_adc = ((V_supply * R_ntc) / (R + R_ntc))
 *
 *          Depending on the position of the NTC in the voltage resistor (R1/R2),
 *          different R_ntc values are used for the calculation.
 */
/**@{*/
#if TS_EPCOS_B57861S0103F045_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
#define TS_EPCOS_B57861S0103F045_ADC_VOLTAGE_V_MAX_V \
    (float)((TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57861s0103f045Lut[ts_b57861s0103f045LutSize-1].resistance_Ohm) / (ts_b57861s0103f045Lut[ts_b57861s0103f045LutSize-1].resistance_Ohm+TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define TS_EPCOS_B57861S0103F045_ADC_VOLTAGE_V_MIN_V \
    (float)((TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57861s0103f045Lut[0].resistance_Ohm) / (ts_b57861s0103f045Lut[0].resistance_Ohm+TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#else /* TS_EPCOS_B57861S0103F045_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define TS_EPCOS_B57861S0103F045_ADC_VOLTAGE_V_MIN_V \
    (float)((TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57861s0103f045Lut[ts_b57861s0103f045LutSize-1].resistance_Ohm) / (ts_b57861s0103f045Lut[ts_b57861s0103f045LutSize-1].resistance_Ohm+TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define TS_EPCOS_B57861S0103F045_ADC_VOLTAGE_V_MAX_V \
    (float)((TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57861s0103f045Lut[0].resistance_Ohm) / (ts_b57861s0103f045Lut[0].resistance_Ohm+TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Epc01GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    int16_t temperature_ddegC = 0;
    float resistance_Ohm      = 0.0f;
    float adcVoltage_V        = adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_EPCOS_B57861S0103F045_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_EPCOS_B57861S0103F045_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if B57861S0103F045_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* B57861S0103F045_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        resistance_Ohm = TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (TS_EPCOS_B57861S0103F045_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* B57861S0103F045_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 1; i < ts_b57861s0103f045LutSize; i++) {
            if (resistance_Ohm < ts_b57861s0103f045Lut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low >= ts_b57861s0103f045LutSize))) {   /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_b57861s0103f045Lut[between_low].resistance_Ohm,
                ts_b57861s0103f045Lut[between_low].temperature_ddegC,
                ts_b57861s0103f045Lut[between_high].resistance_Ohm,
                ts_b57861s0103f045Lut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Epc01GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    float temperature_degC = 0.0f;
    float vadc_V           = adcVoltage_mV / 1000.0f;
    float vadc2            = vadc_V * vadc_V;
    float vadc3            = vadc2 * vadc_V;
    float vadc4            = vadc3 * vadc_V;
    float vadc5            = vadc4 * vadc_V;

    /* 5th grade polynomial for EPCOS B57861S0103F045 NTC-Thermistor, 10 kOhm, Series B57861S, Vref = 3V, R in series 10k */
    temperature_degC = (-6.2765f * vadc5) + (49.0397f * vadc4) - (151.3602f * vadc3) + (233.2521f * vadc2) -
                       (213.4588f * vadc_V) + 130.5822f;

    return (int16_t)(temperature_degC * 10.0f); /* Convert to deci &deg;C */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
