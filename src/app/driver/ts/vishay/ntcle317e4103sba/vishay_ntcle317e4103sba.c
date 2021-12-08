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
 * @file    vishay_ntcle317e4103sba.c
 * @author  foxBMS Team
 * @date    2021-11-03 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup TEMPERATURE_SENSORS
 * @prefix  TS
 *
 * @brief   Resistive divider used for measuring temperature
 *
 */

/*========== Includes =======================================================*/
#include "vishay_ntcle317e4103sba.h"

#include "foxmath.h"
#include "temperature_sensor_defs.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** LUT filled from higher resistance to lower resistance */
static const TS_TEMPERATURE_SENSOR_LUT_s ts_ntcle317e4103sbaLut[] = {
    {-550, 908973.8f}, {-540, 845010.0f}, {-530, 785955.0f}, {-520, 731403.2f}, {-510, 680984.5f}, {-500, 634361.3f},
    {-490, 591225.5f}, {-480, 551295.7f}, {-470, 514314.8f}, {-460, 480047.7f}, {-450, 448279.4f}, {-440, 418812.9f},
    {-430, 391468.1f}, {-420, 366079.9f}, {-410, 342496.8f}, {-400, 320580.1f}, {-390, 300202.2f}, {-380, 281246.4f},
    {-370, 263605.0f}, {-360, 247179.4f}, {-350, 231878.9f}, {-340, 217619.8f}, {-330, 204325.3f}, {-320, 191924.7f},
    {-310, 180352.7f}, {-300, 169549.3f}, {-290, 159459.0f}, {-280, 150030.8f}, {-270, 141217.3f}, {-260, 132975.1f},
    {-250, 125263.9f}, {-240, 118046.5f}, {-230, 111288.5f}, {-220, 104958.2f}, {-210, 99025.9f},  {-200, 93464.6f},
    {-190, 88248.8f},  {-180, 83355.3f},  {-170, 78762.3f},  {-160, 74449.7f},  {-150, 70398.9f},  {-140, 66592.4f},
    {-130, 63014.3f},  {-120, 59649.6f},  {-110, 56484.3f},  {-100, 53505.6f},  {-90, 50701.4f},   {-80, 48060.6f},
    {-70, 45572.8f},   {-60, 43228.3f},   {-50, 41017.9f},   {-40, 38933.5f},   {-30, 36967.0f},   {-20, 35111.2f},
    {-10, 33359.3f},   {0, 31704.8f},     {10, 30141.9f},    {20, 28665.0f},    {30, 27268.9f},    {40, 25948.8f},
    {50, 24700.1f},    {60, 23518.5f},    {70, 22400.2f},    {80, 21341.4f},    {90, 20338.6f},    {100, 19388.5f},
    {110, 18488.2f},   {120, 17634.7f},   {130, 16825.4f},   {140, 16057.8f},   {150, 15329.4f},   {160, 14638.2f},
    {170, 13981.9f},   {180, 13358.7f},   {190, 12766.7f},   {200, 12204.2f},   {210, 11669.6f},   {220, 11161.3f},
    {230, 10677.9f},   {240, 10218.2f},   {250, 9780.5f},    {260, 9363.4f},    {270, 8965.9f},    {280, 8587.8f},
    {290, 8227.4f},    {300, 7884.4f},    {310, 7557.1f},    {320, 7245.6f},    {330, 6948.3f},    {340, 6665.1f},
    {350, 6394.6f},    {360, 6136.5f},    {370, 5890.5f},    {380, 5655.4f},    {390, 5431.0f},    {400, 5216.6f},
    {410, 5012.1f},    {420, 4816.4f},    {430, 4629.4f},    {440, 4450.6f},    {450, 4279.7f},    {460, 4116.5f},
    {470, 3960.1f},    {480, 3810.4f},    {490, 3667.2f},    {500, 3530.2f},    {510, 3398.9f},    {520, 3273.2f},
    {530, 3152.8f},    {540, 3037.5f},    {550, 2926.9f},    {560, 2821.0f},    {570, 2719.4f},    {580, 2622.0f},
    {590, 2528.6f},    {600, 2439.0f},    {610, 2353.0f},    {620, 2270.5f},    {630, 2191.3f},    {640, 2115.2f},
    {650, 2042.2f},    {660, 1971.9f},    {670, 1904.5f},    {680, 1839.8f},    {690, 1777.6f},    {700, 1717.8f},
    {710, 1660.2f},    {720, 1604.9f},    {730, 1551.7f},    {740, 1500.6f},    {750, 1451.3f},    {760, 1403.9f},
    {770, 1358.3f},    {780, 1314.4f},    {790, 1272.1f},    {800, 1231.5f},    {810, 1192.2f},    {820, 1154.5f},
    {830, 1118.1f},    {840, 1083.0f},    {850, 1049.2f},    {860, 1016.4f},    {870, 984.7f},     {880, 954.2f},
    {890, 924.8f},     {900, 896.4f},     {910, 869.1f},     {920, 842.7f},     {930, 817.2f},     {940, 792.6f},
    {950, 768.9f},     {960, 745.9f},     {970, 723.8f},     {980, 702.5f},     {990, 681.8f},     {1000, 661.9f},
    {1010, 642.6f},    {1020, 624.0f},    {1030, 606.0f},    {1040, 588.7f},    {1050, 571.9f},    {1060, 555.6f},
    {1070, 539.9f},    {1080, 524.7f},    {1090, 510.0f},    {1100, 495.8f},    {1110, 482.0f},    {1120, 468.7f},
    {1130, 455.8f},    {1140, 443.4f},    {1150, 431.3f},    {1160, 419.6f},    {1170, 408.3f},    {1180, 397.3f},
    {1190, 386.7f},    {1200, 376.4f},    {1210, 366.4f},    {1220, 356.7f},    {1230, 347.4f},    {1240, 338.3f},
    {1250, 329.5f},    {1260, 321.0f},    {1270, 312.7f},    {1280, 304.7f},    {1290, 296.9f},    {1300, 289.3f},
    {1310, 282.0f},    {1320, 274.9f},    {1330, 268.0f},    {1340, 261.3f},    {1350, 254.8f},    {1360, 248.5f},
    {1370, 242.4f},    {1380, 236.4f},    {1390, 230.6f},    {1400, 225.0f},    {1410, 219.6f},    {1420, 214.3f},
    {1430, 209.1f},    {1440, 204.1f},    {1450, 199.3f},    {1460, 194.5f},    {1470, 189.9f},    {1480, 185.5f},
    {1490, 181.1f},    {1500, 176.9f},
};

/** size of the #ts_ntcle317e4103sbaLut LUT */
static const uint16_t ts_ntcle317e4103sbaLutSize = sizeof(ts_ntcle317e4103sbaLut) / sizeof(TS_TEMPERATURE_SENSOR_LUT_s);

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
#if TS_VISHAY_NTCLE317E4103SBA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
#define TS_VISHAY_NTCLE317E4103SBA_ADC_VOLTAGE_V_MAX_V \
    ((float)((TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle317e4103sbaLut[ts_ntcle317e4103sbaLutSize-1u].resistance_Ohm) / (ts_ntcle317e4103sbaLut[ts_ntcle317e4103sbaLutSize-1u].resistance_Ohm+TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_VISHAY_NTCLE317E4103SBA_ADC_VOLTAGE_V_MIN_V \
    ((float)((TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle317e4103sbaLut[0u].resistance_Ohm) / (ts_ntcle317e4103sbaLut[0u].resistance_Ohm+TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#else /* TS_VISHAY_NTCLE317E4103SBA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 is false */
#define TS_VISHAY_NTCLE317E4103SBA_ADC_VOLTAGE_V_MIN_V \
    ((float)((TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle317e4103sbaLut[ts_ntcle317e4103sbaLutSize-1u].resistance_Ohm) / (ts_ntcle317e4103sbaLut[ts_ntcle317e4103sbaLutSize-1u].resistance_Ohm+TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#define TS_VISHAY_NTCLE317E4103SBA_ADC_VOLTAGE_V_MAX_V \
    ((float)((TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V * ts_ntcle317e4103sbaLut[0u].resistance_Ohm) / (ts_ntcle317e4103sbaLut[0u].resistance_Ohm+TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm)))
#endif
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern int16_t TS_Vis01GetTemperatureFromLut(uint16_t adcVoltage_mV) {
    int16_t temperature_ddegC = INT16_MIN;
    float adcVoltage_V        = (float)adcVoltage_mV / TS_SCALING_FACTOR_1V_IN_MV_FLOAT; /* Convert mV to V */

    /* Check for valid ADC measurements to prevent undefined behavior */
    if (adcVoltage_V > TS_VISHAY_NTCLE317E4103SBA_ADC_VOLTAGE_V_MAX_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or disconnected/shorted */
        temperature_ddegC = INT16_MIN;
    } else if (adcVoltage_V < TS_VISHAY_NTCLE317E4103SBA_ADC_VOLTAGE_V_MIN_V) {
        /* Invalid measured ADC voltage -> sensor out of operating range or shorted/disconnected */
        temperature_ddegC = INT16_MAX;
    } else {
        /* Calculate NTC resistance based on measured ADC voltage */
#if TS_VISHAY_NTCLE317E4103SBA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 == true
        /* R_1 = R_2 * ( ( V_supply / V_adc ) - 1 ) */
        const float resistance_Ohm =
            TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
            ((TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V / adcVoltage_V) - 1);
#else  /* NTCLE317E4103SBA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 is false */
        /* formula: R_2 = R_1 * ( V_2 / ( V_supply - V_adc ) ) */
        const float resistance_Ohm =
            TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_RESISTANCE_R_1_R_2_Ohm *
            (adcVoltage_V / (TS_VISHAY_NTCLE317E4103SBA_RESISTOR_DIVIDER_SUPPLY_VOLTAGE_V - adcVoltage_V));
#endif /* NTCLE317E4103SBA_POSITION_IN_RESISTOR_DIVIDER_IS_R_1 */

        /* Variables for interpolating LUT value */
        uint16_t between_high = 0;
        uint16_t between_low  = 0;
        for (uint16_t i = 0u; i < ts_ntcle317e4103sbaLutSize; i++) {
            if (resistance_Ohm < ts_ntcle317e4103sbaLut[i].resistance_Ohm) {
                between_low  = i + 1u;
                between_high = i;
            }
        }

        /* Interpolate between LUT values, but do not extrapolate LUT! */
        if (!(((between_high == 0u) && (between_low == 0u)) || /* measured resistance > maximum LUT resistance */
              (between_low >= ts_ntcle317e4103sbaLutSize))) {  /* measured resistance < minimum LUT resistance */
            temperature_ddegC = (int16_t)MATH_LinearInterpolation(
                ts_ntcle317e4103sbaLut[between_low].resistance_Ohm,
                (float)ts_ntcle317e4103sbaLut[between_low].temperature_ddegC,
                ts_ntcle317e4103sbaLut[between_high].resistance_Ohm,
                (float)ts_ntcle317e4103sbaLut[between_high].temperature_ddegC,
                resistance_Ohm);
        }
    }

    /* Return temperature based on measured NTC resistance */
    return temperature_ddegC;
}

extern int16_t TS_Vis01GetTemperatureFromPolynomial(uint16_t adcVoltage_mV) {
    (void)adcVoltage_mV;
    FAS_ASSERT(FAS_TRAP);
    int16_t temperature_ddegC = 0;
    /* TODO this is not implemented */
    return temperature_ddegC;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
