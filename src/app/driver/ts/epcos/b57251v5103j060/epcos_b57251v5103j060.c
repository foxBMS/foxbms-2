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
 * @file    epcos_b57251v5103j060.c
 * @author  foxBMS Team
 * @date    2018-10-30 (date of creation)
 * @updated 2021-08-06 (date of last update)
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 *
 */

/*========== Includes =======================================================*/
#include "epcos_b57251v5103j060.h"

#include "foxmath.h"
#include "temperature_sensor_defs.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/* clang-format off */
/** LUT filled from higher resistance to lower resistance */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_b57251v5103j060Lut[] = {
    { -550, 961580.00f },
    { -500, 668920.00f },
    { -450, 471270.00f },
    { -400, 336060.00f },
    { -350, 242430.00f },
    { -300, 176810.00f },
    { -250, 130320.00f },
    { -200, 97020.00f },
    { -150, 72923.00f },
    { -100, 55314.00f },
    {  -50, 42325.00f },
    {   0, 32657.00f },
    {   50, 25400.00f },
    {  100, 19907.00f },
    {  150, 15716.00f },
    {  200, 12494.00f },
    {  250, 10000.00f },
    {  300, 8055.20f },
    {  350, 6528.80f },
    {  400, 5322.90f },
    {  450, 4364.50f },
    {  500, 3598.10f },
    {  550, 2981.90f },
    {  600, 2483.70f },
    {  650, 2078.70f },
    {  700, 1747.90f },
    {  750, 1476.30f },
    {  800, 1252.30f },
    {  850, 1066.70f },
    {  900, 912.27f },
    {  950, 783.19f },
    { 1000, 674.88f },
    { 1050, 583.63f },
    { 1100, 506.47f },
    { 1150, 440.98f },
    { 1200, 385.20f },
    { 1250, 337.52f },
    { 1300, 296.63f },
    { 1350, 261.46f },
    { 1400, 231.11f },
    { 1450, 204.84f },
    { 1500, 182.03f }
};
/* clang-format on */

/** size of the #ts_b57251v5103j060Lut LUT */
static uint16_t b57251v5103j060LutSize = sizeof(ts_b57251v5103j060Lut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

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
#if TS_EPCOS_B57251V5103J060_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
#define TS_EPCOS_B57251V5103J060_ADC_VOLTAGE_V_MAX_V \
    (float)((TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57251v5103j060Lut[b57251v5103j060LutSize-1].resistance_Ohm) / (ts_b57251v5103j060Lut[b57251v5103j060LutSize-1].resistance_Ohm+TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define TS_EPCOS_B57251V5103J060_ADC_VOLTAGE_V_MIN_V \
    (float)((TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57251v5103j060Lut[0].resistance_Ohm) / (ts_b57251v5103j060Lut[0].resistance_Ohm+TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#else /* TS_EPCOS_B57251V5103J060_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
#define TS_EPCOS_B57251V5103J060_ADC_VOLTAGE_V_MIN_V \
    (float)((TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57251v5103j060Lut[b57251v5103j060LutSize-1].resistance_Ohm) / (ts_b57251v5103j060Lut[b57251v5103j060LutSize-1].resistance_Ohm+TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#define TS_EPCOS_B57251V5103J060_ADC_VOLTAGE_V_MAX_V \
    (float)((TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_b57251v5103j060Lut[0].resistance_Ohm) / (ts_b57251v5103j060Lut[0].resistance_Ohm+TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Epc00GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    int16_t temperature_ddegC = 0;
    float resistance_Ohm      = 0.0f;
    float adcVoltage_V        = adcVoltage_mV / 1000.0f; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_EPCOS_B57251V5103J060_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_EPCOS_B57251V5103J060_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if TS_EPCOS_B57251V5103J060_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        resistance_Ohm = TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         ((TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* TS_EPCOS_B57251V5103J060_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == false */
        /* R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        resistance_Ohm = TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
                         (adcVoltage_V / (TS_EPCOS_B57251V5103J060_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* TS_EPCOS_B57251V5103J060_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 1; i < b57251v5103j060LutSize; i++) {
            if (resistance_Ohm < ts_b57251v5103j060Lut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low > b57251v5103j060LutSize))) {       /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_b57251v5103j060Lut[between_low].resistance_Ohm,
                ts_b57251v5103j060Lut[between_low].temperature_ddegC,
                ts_b57251v5103j060Lut[between_high].resistance_Ohm,
                ts_b57251v5103j060Lut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Epc00GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    float temperature_degC = 0.0;
    float vadc_V           = adcVoltage_mV / 1000.0;
    float vadc2            = vadc_V * vadc_V;
    float vadc3            = vadc2 * vadc_V;
    float vadc4            = vadc3 * vadc_V;
    float vadc5            = vadc4 * vadc_V;
    float vadc6            = vadc5 * vadc_V;

    temperature_degC = (6.8405f * vadc6) - (74.815f * vadc5) + (317.48f * vadc4) - (669.16f * vadc3) +
                       (740.82f * vadc2) - (444.97f * vadc_V) + 166.48f;

    return (int16_t)(temperature_degC * 10.0f); /* Convert deg into deci &deg;C */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
