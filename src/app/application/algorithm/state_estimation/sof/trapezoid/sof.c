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
 * @file    sof.c
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup APPLICATION_CONFIGURATION
 * @prefix  SOX
 *
 * @brief   SOX module responsible for current derating calculation
 *
 */

/*========== Includes =======================================================*/
#include "sof.h"

#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"

#include "bms.h"
#include "database.h"
#include "foxmath.h"

#include <float.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** @{
 * module-local static Variables that are calculated at startup and used later to avoid divisions at runtime
 */
static SOF_CURVE_s sof_curveRecommendedOperatingCurrent;
static SOF_CURVE_s sof_curveMol;
static SOF_CURVE_s sof_curveRsl;
static SOF_CURVE_s sof_curveMsl;
/** @} */

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_MIN_MAX_s sof_tableMinimumMaximumValues = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_SOF_s sof_tableSofValues                = {.header.uniqueId = DATA_BLOCK_ID_SOF};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   calculate SOF curve depending on configured configuration values
 *
 * @param[in]  pConfigurationValues         SOF curve configuration values
 * @param[out] pCalculatedSofCurveValues    calculate SOF curve
 */
static void SOF_CalculateCurves(const SOF_CONFIG_s *pConfigurationValues, SOF_CURVE_s *pCalculatedSofCurveValues);

/**
 *  @brief  calculates the SoF from voltage data (i.e., minimum and maximum voltage)
 *
 * @param[in]  minimumCellVoltage_mV        minimum cell voltage
 * @param[in]  maximumCellVoltage_mV        maximum cell voltage
 * @param[out] pAllowedVoltageBasedCurrent  Voltage-based SOF
 * @param[in]  pConfigLimitValues           pointer to the SOF configuration structure
 * @param[in]  pCalculatedSofCurves         pointer to the SOF curve structure
 */
static void SOF_CalculateVoltageBasedCurrentLimit(
    int16_t minimumCellVoltage_mV,
    int16_t maximumCellVoltage_mV,
    SOF_CURRENT_LIMITS_s *pAllowedVoltageBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves);

/**
 * @brief   calculates the SoF from temperature data (i.e., minimum and maximum temperature of cells)
 *
 * @param[in]  minimumCellTemperature_ddegC      minimum temperature of cells
 * @param[in]  maximumCellTemperature_ddegC      maximum temperature of cells
 * @param[out] pAllowedTemperatureBasedCurrent   pointer where to store the results
 * @param[in]  pConfigLimitValues                pointer to the structure used for SOF calculation
 * @param[in]  pCalculatedSofCurves              pointer to the structure containing limit values
 */
static void SOF_CalculateTemperatureBasedCurrentLimit(
    int16_t minimumCellTemperature_ddegC,
    int16_t maximumCellTemperature_ddegC,
    SOF_CURRENT_LIMITS_s *pAllowedTemperatureBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves);

/**
 * @brief   get the minimum current values of all variants of SoF calculation
 *
 * @param[in]  voltageBasedLimits       voltage constrained current derating values
 * @param[in]  temperatureBasedLimits   temperature constrained current derating values
 *
 * @return minimum SoF current values
 */
static SOF_CURRENT_LIMITS_s SOF_MinimumOfTwoSofValues(
    SOF_CURRENT_LIMITS_s voltageBasedLimits,
    SOF_CURRENT_LIMITS_s temperatureBasedLimits);

/*========== Static Function Implementations ================================*/
static void SOF_CalculateCurves(const SOF_CONFIG_s *pConfigurationValues, SOF_CURVE_s *pCalculatedSofCurveValues) {
    FAS_ASSERT(pConfigurationValues != NULL_PTR);
    FAS_ASSERT(pCalculatedSofCurveValues != NULL_PTR);

    /* Calculating SOF curve for the maximum allowed current for MOL/RSL/MSL */
    pCalculatedSofCurveValues->slopeLowTemperatureDischarge =
        (pConfigurationValues->maximumDischargeCurrent_mA - pConfigurationValues->limpHomeCurrent_mA) /
        (pConfigurationValues->cutoffLowTemperatureDischarge_ddegC -
         pConfigurationValues->limitLowTemperatureDischarge_ddegC);
    pCalculatedSofCurveValues->offsetLowTemperatureDischarge =
        pConfigurationValues->limpHomeCurrent_mA - (pCalculatedSofCurveValues->slopeLowTemperatureDischarge *
                                                    pConfigurationValues->limitLowTemperatureDischarge_ddegC);

    pCalculatedSofCurveValues->slopeHighTemperatureDischarge =
        (0.0f - pConfigurationValues->maximumDischargeCurrent_mA) /
        (pConfigurationValues->limitHighTemperatureDischarge_ddegC -
         pConfigurationValues->cutoffHighTemperatureDischarge_ddegC);
    pCalculatedSofCurveValues->offsetHighTemperatureDischarge =
        0.0f - (pCalculatedSofCurveValues->slopeHighTemperatureDischarge *
                pConfigurationValues->limitHighTemperatureDischarge_ddegC);

    pCalculatedSofCurveValues->slopeLowTemperatureCharge = (pConfigurationValues->maximumChargeCurrent_mA - 0.0f) /
                                                           (pConfigurationValues->cutoffLowTemperatureCharge_ddegC -
                                                            pConfigurationValues->limitLowTemperatureCharge_ddegC);
    pCalculatedSofCurveValues->offsetLowTemperatureCharge =
        0.0f -
        (pCalculatedSofCurveValues->slopeLowTemperatureCharge * pConfigurationValues->limitLowTemperatureCharge_ddegC);

    pCalculatedSofCurveValues->slopeHighTemperatureCharge = (0.0f - pConfigurationValues->maximumChargeCurrent_mA) /
                                                            (pConfigurationValues->limitHighTemperatureCharge_ddegC -
                                                             pConfigurationValues->cutoffHighTemperatureCharge_ddegC);
    pCalculatedSofCurveValues->offsetHighTemperatureCharge = 0.0f -
                                                             (pCalculatedSofCurveValues->slopeHighTemperatureCharge *
                                                              pConfigurationValues->limitHighTemperatureCharge_ddegC);

    pCalculatedSofCurveValues->slopeUpperCellVoltage =
        (pConfigurationValues->maximumDischargeCurrent_mA - 0.0f) /
        (pConfigurationValues->cutoffLowerCellVoltage_mV - pConfigurationValues->limitLowerCellVoltage_mV);
    pCalculatedSofCurveValues->offsetUpperCellVoltage =
        0.0f - (pCalculatedSofCurveValues->slopeUpperCellVoltage * pConfigurationValues->limitLowerCellVoltage_mV);

    pCalculatedSofCurveValues->slopeLowerCellVoltage =
        (pConfigurationValues->maximumChargeCurrent_mA - 0.0f) /
        (pConfigurationValues->cutoffUpperCellVoltage_mV - pConfigurationValues->limitUpperCellVoltage_mV);
    pCalculatedSofCurveValues->offsetLowerCellVoltage =
        0.0f - (pCalculatedSofCurveValues->slopeLowerCellVoltage * pConfigurationValues->limitLowerCellVoltage_mV);
}

static void SOF_CalculateVoltageBasedCurrentLimit(
    int16_t minimumCellVoltage_mV,
    int16_t maximumCellVoltage_mV,
    SOF_CURRENT_LIMITS_s *pAllowedVoltageBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves) {
    FAS_ASSERT(pAllowedVoltageBasedCurrent != NULL_PTR);
    FAS_ASSERT(pConfigLimitValues != NULL_PTR);
    FAS_ASSERT(pCalculatedSofCurves != NULL_PTR);

    /* minimum cell voltage calculation */
    if (minimumCellVoltage_mV <= pConfigLimitValues->limitLowerCellVoltage_mV) {
        pAllowedVoltageBasedCurrent->continuousDischargeCurrent_mA = 0.0f;
        pAllowedVoltageBasedCurrent->peakDischargeCurrent_mA       = 0.0f;
    } else {
        if (minimumCellVoltage_mV <= pConfigLimitValues->cutoffLowerCellVoltage_mV) {
            pAllowedVoltageBasedCurrent->continuousDischargeCurrent_mA =
                (pCalculatedSofCurves->slopeUpperCellVoltage *
                 (minimumCellVoltage_mV - pConfigLimitValues->limitLowerCellVoltage_mV));
            pAllowedVoltageBasedCurrent->peakDischargeCurrent_mA =
                pAllowedVoltageBasedCurrent->continuousDischargeCurrent_mA;
        } else {
            pAllowedVoltageBasedCurrent->continuousDischargeCurrent_mA = pConfigLimitValues->maximumDischargeCurrent_mA;
            pAllowedVoltageBasedCurrent->peakDischargeCurrent_mA       = pConfigLimitValues->maximumDischargeCurrent_mA;
        }
    }
    /* maximum cell voltage calculation */
    if (maximumCellVoltage_mV >= pConfigLimitValues->limitUpperCellVoltage_mV) {
        pAllowedVoltageBasedCurrent->continuousChargeCurrent_mA = 0.0f;
        pAllowedVoltageBasedCurrent->peakChargeCurrent_mA       = 0.0f;
    } else {
        if (maximumCellVoltage_mV >= pConfigLimitValues->cutoffUpperCellVoltage_mV) {
            pAllowedVoltageBasedCurrent->continuousChargeCurrent_mA =
                (pCalculatedSofCurves->slopeLowerCellVoltage *
                 (maximumCellVoltage_mV - pConfigLimitValues->limitUpperCellVoltage_mV));
            pAllowedVoltageBasedCurrent->peakChargeCurrent_mA = pAllowedVoltageBasedCurrent->continuousChargeCurrent_mA;
        } else {
            pAllowedVoltageBasedCurrent->continuousChargeCurrent_mA = pConfigLimitValues->maximumChargeCurrent_mA;
            pAllowedVoltageBasedCurrent->peakChargeCurrent_mA       = pConfigLimitValues->maximumChargeCurrent_mA;
        }
    }
}

static void SOF_CalculateTemperatureBasedCurrentLimit(
    int16_t minimumCellTemperature_ddegC,
    int16_t maximumCellTemperature_ddegC,
    SOF_CURRENT_LIMITS_s *pAllowedTemperatureBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves) {
    FAS_ASSERT(pAllowedTemperatureBasedCurrent != NULL_PTR);
    FAS_ASSERT(pConfigLimitValues != NULL_PTR);
    FAS_ASSERT(pCalculatedSofCurves != NULL_PTR);

    SOF_CURRENT_LIMITS_s temporaryCurrentLimits = {0.0f, 0.0f, 0.0f, 0.0f};
    /* Temperature low Discharge */
    if (minimumCellTemperature_ddegC <= pConfigLimitValues->limitLowTemperatureDischarge_ddegC) {
        pAllowedTemperatureBasedCurrent->continuousDischargeCurrent_mA = pConfigLimitValues->limpHomeCurrent_mA;
        pAllowedTemperatureBasedCurrent->peakDischargeCurrent_mA       = pConfigLimitValues->limpHomeCurrent_mA;
    } else {
        if (minimumCellTemperature_ddegC <= pConfigLimitValues->cutoffLowTemperatureDischarge_ddegC) {
            pAllowedTemperatureBasedCurrent->continuousDischargeCurrent_mA =
                (pCalculatedSofCurves->slopeLowTemperatureDischarge * minimumCellTemperature_ddegC) +
                pCalculatedSofCurves->offsetLowTemperatureDischarge;
            pAllowedTemperatureBasedCurrent->peakDischargeCurrent_mA =
                pAllowedTemperatureBasedCurrent->continuousDischargeCurrent_mA;
        } else {
            pAllowedTemperatureBasedCurrent->continuousDischargeCurrent_mA =
                pConfigLimitValues->maximumDischargeCurrent_mA;
            pAllowedTemperatureBasedCurrent->peakDischargeCurrent_mA = pConfigLimitValues->maximumDischargeCurrent_mA;
        }
    }
    /* Temperature low charge */
    if (minimumCellTemperature_ddegC <= pConfigLimitValues->limitLowTemperatureCharge_ddegC) {
        pAllowedTemperatureBasedCurrent->continuousChargeCurrent_mA = 0;
        pAllowedTemperatureBasedCurrent->peakChargeCurrent_mA       = 0;
    } else {
        if (minimumCellTemperature_ddegC <= pConfigLimitValues->cutoffLowTemperatureCharge_ddegC) {
            pAllowedTemperatureBasedCurrent->continuousChargeCurrent_mA =
                (pCalculatedSofCurves->slopeLowTemperatureCharge * minimumCellTemperature_ddegC) +
                pCalculatedSofCurves->offsetLowTemperatureCharge;
            pAllowedTemperatureBasedCurrent->peakChargeCurrent_mA =
                pAllowedTemperatureBasedCurrent->continuousChargeCurrent_mA;
        } else {
            pAllowedTemperatureBasedCurrent->continuousChargeCurrent_mA = pConfigLimitValues->maximumChargeCurrent_mA;
            pAllowedTemperatureBasedCurrent->peakChargeCurrent_mA       = pConfigLimitValues->maximumChargeCurrent_mA;
        }
    }
    /* Temperature high discharge */
    if (maximumCellTemperature_ddegC >= pConfigLimitValues->limitHighTemperatureDischarge_ddegC) {
        pAllowedTemperatureBasedCurrent->continuousDischargeCurrent_mA = 0.0f;
        pAllowedTemperatureBasedCurrent->peakDischargeCurrent_mA       = 0.0f;
    } else {
        if (maximumCellTemperature_ddegC >= pConfigLimitValues->cutoffHighTemperatureDischarge_ddegC) {
            temporaryCurrentLimits.continuousDischargeCurrent_mA =
                (pCalculatedSofCurves->slopeHighTemperatureDischarge * maximumCellTemperature_ddegC) +
                pCalculatedSofCurves->offsetHighTemperatureDischarge;
            temporaryCurrentLimits.peakDischargeCurrent_mA = temporaryCurrentLimits.continuousDischargeCurrent_mA;
        } else {
            /* do nothing because this situation is handled with minimumCellTemperature_ddegC */
            temporaryCurrentLimits.continuousDischargeCurrent_mA = pConfigLimitValues->maximumDischargeCurrent_mA;
            temporaryCurrentLimits.peakDischargeCurrent_mA       = pConfigLimitValues->maximumDischargeCurrent_mA;
        }
        /* Derating value for minimum cell temperature has already been calculated and result is saved in
           pAllowedTemperatureBasedCurrentCheck. Check now if newly calculated derating value for maximum
           cell temperatures is smaller than the previously calculated value */
        pAllowedTemperatureBasedCurrent->continuousDischargeCurrent_mA = MATH_MinimumOfTwoFloats(
            pAllowedTemperatureBasedCurrent->continuousDischargeCurrent_mA,
            temporaryCurrentLimits.continuousDischargeCurrent_mA);
        pAllowedTemperatureBasedCurrent->peakDischargeCurrent_mA = MATH_MinimumOfTwoFloats(
            pAllowedTemperatureBasedCurrent->peakDischargeCurrent_mA, temporaryCurrentLimits.peakDischargeCurrent_mA);
    }
    /* Temperature high Charge */
    if (maximumCellTemperature_ddegC >= pConfigLimitValues->limitHighTemperatureCharge_ddegC) {
        pAllowedTemperatureBasedCurrent->continuousChargeCurrent_mA = 0.0f;
        pAllowedTemperatureBasedCurrent->peakChargeCurrent_mA       = 0.0f;
    } else {
        if (maximumCellTemperature_ddegC >= pConfigLimitValues->cutoffHighTemperatureCharge_ddegC) {
            temporaryCurrentLimits.continuousChargeCurrent_mA =
                (pCalculatedSofCurves->slopeHighTemperatureCharge * maximumCellTemperature_ddegC) +
                pCalculatedSofCurves->offsetHighTemperatureCharge;
            temporaryCurrentLimits.peakChargeCurrent_mA = temporaryCurrentLimits.continuousChargeCurrent_mA;
        } else {
            /* do nothing because this situation is handled with minimumCellTemperature_ddegC */
            temporaryCurrentLimits.continuousChargeCurrent_mA = pConfigLimitValues->maximumChargeCurrent_mA;
            temporaryCurrentLimits.peakChargeCurrent_mA       = pConfigLimitValues->maximumChargeCurrent_mA;
        }
        /* Derating value for minimum cell temperature has already been calculated and result is saved in
           pAllowedTemperatureBasedCurrentCheck. Check now if newly calculated derating value for maximum
           cell temperatures is smaller than the previously calculated value */
        pAllowedTemperatureBasedCurrent->continuousChargeCurrent_mA = MATH_MinimumOfTwoFloats(
            pAllowedTemperatureBasedCurrent->continuousChargeCurrent_mA,
            temporaryCurrentLimits.continuousChargeCurrent_mA);
        pAllowedTemperatureBasedCurrent->peakChargeCurrent_mA = MATH_MinimumOfTwoFloats(
            pAllowedTemperatureBasedCurrent->peakChargeCurrent_mA, temporaryCurrentLimits.peakChargeCurrent_mA);
    }
}

static SOF_CURRENT_LIMITS_s SOF_MinimumOfTwoSofValues(
    SOF_CURRENT_LIMITS_s voltageBasedLimits,
    SOF_CURRENT_LIMITS_s temperatureBasedLimits) {
    SOF_CURRENT_LIMITS_s retval       = {0};
    retval.continuousChargeCurrent_mA = MATH_MinimumOfTwoFloats(
        voltageBasedLimits.continuousChargeCurrent_mA, temperatureBasedLimits.continuousChargeCurrent_mA);
    retval.peakChargeCurrent_mA =
        MATH_MinimumOfTwoFloats(voltageBasedLimits.peakChargeCurrent_mA, temperatureBasedLimits.peakChargeCurrent_mA);
    retval.continuousDischargeCurrent_mA = MATH_MinimumOfTwoFloats(
        voltageBasedLimits.continuousDischargeCurrent_mA, temperatureBasedLimits.continuousDischargeCurrent_mA);
    retval.peakDischargeCurrent_mA = MATH_MinimumOfTwoFloats(
        voltageBasedLimits.peakDischargeCurrent_mA, temperatureBasedLimits.peakDischargeCurrent_mA);
    return retval;
}

/*========== Extern Function Implementations ================================*/
extern void SOF_Init(void) {
    /* Calculating SOF curve for the recommended operating current */
    SOF_CalculateCurves(&sof_recommendedCurrent, &sof_curveRecommendedOperatingCurrent);

#if BMS_CHECK_SOF_CURRENT_LIMITS == true
    /* Calculating SOF curve for maximum operating limit */
    SOF_CalculateCurves(&sof_maximumOperatingLimit, &sof_curveMol);

    /* Calculating SOF curve for recommended safety limit */
    SOF_CalculateCurves(&sof_recommendedSafetyLimit, &sof_curveRsl);

    /* Calculating SOF curve for maximum safety limit */
    SOF_CalculateCurves(&sof_configMaximumSafetyLimit, &sof_curveMsl);
#endif
}

extern void SOF_Calculation(void) {
    SOF_CURRENT_LIMITS_s allowedCurrent = {0};

    DATA_READ_DATA(&sof_tableMinimumMaximumValues);

    /* Reset allowed current values */
    sof_tableSofValues.recommendedContinuousPackChargeCurrent_mA    = 0.0f;
    sof_tableSofValues.recommendedContinuousPackDischargeCurrent_mA = 0.0f;
    sof_tableSofValues.recommendedPeakPackChargeCurrent_mA          = 0.0f;
    sof_tableSofValues.recommendedPeakPackDischargeCurrent_mA       = 0.0f;

    uint8_t nrClosedStrings = 0;
    float minDischarge_mA   = FLT_MAX;
    float minCharge_mA      = FLT_MAX;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        SOF_CURRENT_LIMITS_s voltageBasedSof     = {0};
        SOF_CURRENT_LIMITS_s temperatureBasedSof = {0};
        /* Calculate allowed current if string is connected */
        if (true == BMS_IsStringClosed(stringNumber)) {
            SOF_CalculateVoltageBasedCurrentLimit(
                (float)sof_tableMinimumMaximumValues.minimumCellVoltage_mV[stringNumber],
                (float)sof_tableMinimumMaximumValues.maximumCellVoltage_mV[stringNumber],
                &voltageBasedSof,
                &sof_recommendedCurrent,
                &sof_curveRecommendedOperatingCurrent);
            SOF_CalculateTemperatureBasedCurrentLimit(
                (float)sof_tableMinimumMaximumValues.minimumTemperature_ddegC[stringNumber],
                (float)sof_tableMinimumMaximumValues.maximumTemperature_ddegC[stringNumber],
                &temperatureBasedSof,
                &sof_recommendedCurrent,
                &sof_curveRecommendedOperatingCurrent);
            allowedCurrent = SOF_MinimumOfTwoSofValues(voltageBasedSof, temperatureBasedSof);

            sof_tableSofValues.recommendedContinuousChargeCurrent_mA[stringNumber] =
                allowedCurrent.continuousChargeCurrent_mA;
            sof_tableSofValues.recommendedContinuousDischargeCurrent_mA[stringNumber] =
                allowedCurrent.continuousDischargeCurrent_mA;
            sof_tableSofValues.recommendedPeakChargeCurrent_mA[stringNumber] = allowedCurrent.peakChargeCurrent_mA;
            sof_tableSofValues.recommendedPeakDischargeCurrent_mA[stringNumber] =
                allowedCurrent.peakDischargeCurrent_mA;

            nrClosedStrings++;
            if (minCharge_mA > sof_tableSofValues.recommendedContinuousChargeCurrent_mA[stringNumber]) {
                minCharge_mA = sof_tableSofValues.recommendedContinuousChargeCurrent_mA[stringNumber];
            }
            if (minDischarge_mA > sof_tableSofValues.recommendedContinuousDischargeCurrent_mA[stringNumber]) {
                minDischarge_mA = sof_tableSofValues.recommendedContinuousDischargeCurrent_mA[stringNumber];
            }
        } else {
            sof_tableSofValues.recommendedContinuousChargeCurrent_mA[stringNumber]    = 0.0f;
            sof_tableSofValues.recommendedContinuousDischargeCurrent_mA[stringNumber] = 0.0f;
            sof_tableSofValues.recommendedPeakChargeCurrent_mA[stringNumber]          = 0.0f;
            sof_tableSofValues.recommendedPeakDischargeCurrent_mA[stringNumber]       = 0.0f;
        }

#if BMS_CHECK_SOF_CURRENT_LIMITS == true
        /* Calculate maximum allowed current MOL level */
        SOF_CalculateVoltageBasedCurrentLimit(
            (float)sof_tableMinimumMaximumValues.minimumCellVoltage_mV[stringNumber],
            (float)sof_tableMinimumMaximumValues.maximumCellVoltage_mV[stringNumber],
            &voltageBasedSof,
            &sof_maximumOperatingLimit,
            &sof_curveMol);
        SOF_CalculateTemperatureBasedCurrentLimit(
            (float)sof_tableMinimumMaximumValues.minimumTemperature_ddegC[stringNumber],
            (float)sof_tableMinimumMaximumValues.maximumTemperature_ddegC[stringNumber],
            &temperatureBasedSof,
            &sof_maximumOperatingLimit,
            &sof_curveMol);
        allowedCurrent = SOF_MinimumOfTwoSofValues(voltageBasedSof, temperatureBasedSof);
        sof_tableSofValues.continuousMolChargeCurrent_mA[stringNumber] = allowedCurrent.continuousChargeCurrent_mA;
        sof_tableSofValues.continuousMolDischargeCurrent_mA[stringNumber] =
            allowedCurrent.continuousDischargeCurrent_mA;
        sof_tableSofValues.peakMolChargeCurrent_mA[stringNumber]    = allowedCurrent.peakChargeCurrent_mA;
        sof_tableSofValues.peakMolDischargeCurrent_mA[stringNumber] = allowedCurrent.peakDischargeCurrent_mA;

        /* Calculate maximum allowed current RSL level */
        SOF_CalculateVoltageBasedCurrentLimit(
            (float)sof_tableMinimumMaximumValues.minimumCellVoltage_mV[stringNumber],
            (float)sof_tableMinimumMaximumValues.maximumCellVoltage_mV[stringNumber],
            &voltageBasedSof,
            &sof_recommendedSafetyLimit,
            &sof_curveRsl);
        SOF_CalculateTemperatureBasedCurrentLimit(
            (float)sof_tableMinimumMaximumValues.minimumTemperature_ddegC[stringNumber],
            (float)sof_tableMinimumMaximumValues.maximumTemperature_ddegC[stringNumber],
            &temperatureBasedSof,
            &sof_recommendedSafetyLimit,
            &sof_curveRsl);
        allowedCurrent = SOF_MinimumOfTwoSofValues(voltageBasedSof, temperatureBasedSof);
        sof_tableSofValues.continuousRslChargeCurrent_mA[stringNumber] = allowedCurrent.continuousChargeCurrent_mA;
        sof_tableSofValues.continuousRslDischargeCurrent_mA[stringNumber] =
            allowedCurrent.continuousDischargeCurrent_mA;
        sof_tableSofValues.peakRslChargeCurrent_mA[stringNumber]    = allowedCurrent.peakChargeCurrent_mA;
        sof_tableSofValues.peakRslDischargeCurrent_mA[stringNumber] = allowedCurrent.peakDischargeCurrent_mA;

        /* Calculate maximum allowed current MSL level */
        SOF_CalculateVoltageBasedCurrentLimit(
            (float)sof_tableMinimumMaximumValues.minimumCellVoltage_mV[stringNumber],
            (float)sof_tableMinimumMaximumValues.maximumCellVoltage_mV[stringNumber],
            &voltageBasedSof,
            &sof_configMaximumSafetyLimit,
            &sof_curveMsl);
        SOF_CalculateTemperatureBasedCurrentLimit(
            (float)sof_tableMinimumMaximumValues.minimumTemperature_ddegC[stringNumber],
            (float)sof_tableMinimumMaximumValues.maximumTemperature_ddegC[stringNumber],
            &temperatureBasedSof,
            &sof_configMaximumSafetyLimit,
            &sof_curveMsl);
        allowedCurrent = SOF_MinimumOfTwoSofValues(voltageBasedSof, temperatureBasedSof);
        sof_tableSofValues.continuousMslChargeCurrent_mA[stringNumber] = allowedCurrent.continuousChargeCurrent_mA;
        sof_tableSofValues.continuousMslDischargeCurrent_mA[stringNumber] =
            allowedCurrent.continuousDischargeCurrent_mA;
        sof_tableSofValues.peakMslChargeCurrent_mA[stringNumber]    = allowedCurrent.peakChargeCurrent_mA;
        sof_tableSofValues.peakMslDischargeCurrent_mA[stringNumber] = allowedCurrent.peakDischargeCurrent_mA;
#else  /* BMS_CHECK_SOF_CURRENT_LIMITS == false */
        sof_tableSofValues.continuousMolChargeCurrent_mA[stringNumber]    = BC_CURRENT_MAX_CHARGE_MOL_mA;
        sof_tableSofValues.continuousMolDischargeCurrent_mA[stringNumber] = BC_CURRENT_MAX_DISCHARGE_MOL_mA;
        sof_tableSofValues.peakMolChargeCurrent_mA[stringNumber]          = BC_CURRENT_MAX_CHARGE_MOL_mA;
        sof_tableSofValues.peakMolDischargeCurrent_mA[stringNumber]       = BC_CURRENT_MAX_DISCHARGE_MOL_mA;

        sof_tableSofValues.continuousRslChargeCurrent_mA[stringNumber]    = BC_CURRENT_MAX_CHARGE_RSL_mA;
        sof_tableSofValues.continuousRslDischargeCurrent_mA[stringNumber] = BC_CURRENT_MAX_DISCHARGE_RSL_mA;
        sof_tableSofValues.peakRslChargeCurrent_mA[stringNumber]          = BC_CURRENT_MAX_CHARGE_RSL_mA;
        sof_tableSofValues.peakRslDischargeCurrent_mA[stringNumber]       = BC_CURRENT_MAX_DISCHARGE_RSL_mA;

        sof_tableSofValues.continuousMslChargeCurrent_mA[stringNumber]    = BC_CURRENT_MAX_CHARGE_MSL_mA;
        sof_tableSofValues.continuousMslDischargeCurrent_mA[stringNumber] = BC_CURRENT_MAX_DISCHARGE_MSL_mA;
        sof_tableSofValues.peakMslChargeCurrent_mA[stringNumber]          = BC_CURRENT_MAX_CHARGE_MSL_mA;
        sof_tableSofValues.peakMslDischargeCurrent_mA[stringNumber]       = BC_CURRENT_MAX_DISCHARGE_MSL_mA;
#endif /* BMS_CHECK_SOF_CURRENT_LIMITS == true */
    }

    if (minCharge_mA > (float)BS_MAXIMUM_STRING_CURRENT_mA) {
        minCharge_mA = (float)BS_MAXIMUM_STRING_CURRENT_mA;
    }
    if (minDischarge_mA > (float)BS_MAXIMUM_STRING_CURRENT_mA) {
        minDischarge_mA = (float)BS_MAXIMUM_STRING_CURRENT_mA;
    }

    /* Compute recommended pack values */
    sof_tableSofValues.recommendedContinuousPackChargeCurrent_mA    = (float)nrClosedStrings * minCharge_mA;
    sof_tableSofValues.recommendedContinuousPackDischargeCurrent_mA = (float)nrClosedStrings * minDischarge_mA;
    sof_tableSofValues.recommendedPeakPackChargeCurrent_mA          = (float)nrClosedStrings * minCharge_mA;
    sof_tableSofValues.recommendedPeakPackDischargeCurrent_mA       = (float)nrClosedStrings * minDischarge_mA;

    DATA_WRITE_DATA(&sof_tableSofValues);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_SOF_CalculateCurves(const SOF_CONFIG_s *pConfigurationValues, SOF_CURVE_s *pCalculatedSofCurveValues) {
    SOF_CalculateCurves(pConfigurationValues, pCalculatedSofCurveValues);
}
extern void TEST_SOF_CalculateVoltageBasedCurrentLimit(
    int16_t minimumCellVoltage_mV,
    int16_t maximumCellVoltage_mV,
    SOF_CURRENT_LIMITS_s *pAllowedVoltageBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves) {
    SOF_CalculateVoltageBasedCurrentLimit(
        minimumCellVoltage_mV,
        maximumCellVoltage_mV,
        pAllowedVoltageBasedCurrent,
        pConfigLimitValues,
        pCalculatedSofCurves);
}
extern void TEST_SOF_CalculateTemperatureBasedCurrentLimit(
    int16_t minimumCellTemperature_ddegC,
    int16_t maximumCellTemperature_ddegC,
    SOF_CURRENT_LIMITS_s *pAllowedTemperatureBasedCurrent,
    const SOF_CONFIG_s *pConfigLimitValues,
    SOF_CURVE_s *pCalculatedSofCurves) {
    SOF_CalculateTemperatureBasedCurrentLimit(
        minimumCellTemperature_ddegC,
        maximumCellTemperature_ddegC,
        pAllowedTemperatureBasedCurrent,
        pConfigLimitValues,
        pCalculatedSofCurves);
}
extern SOF_CURRENT_LIMITS_s TEST_SOF_MinimumOfTwoSofValues(
    SOF_CURRENT_LIMITS_s voltageBasedLimits,
    SOF_CURRENT_LIMITS_s temperatureBasedLimits) {
    return SOF_MinimumOfTwoSofValues(voltageBasedLimits, temperatureBasedLimits);
}
#endif
