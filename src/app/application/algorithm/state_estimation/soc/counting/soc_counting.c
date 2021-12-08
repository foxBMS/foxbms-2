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
 * @file    soc_counting.c
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup APPLICATION
 * @prefix  SOC
 *
 * @brief   SOC module responsible for calculation of SOC
 *
 */

/*========== Includes =======================================================*/
#include "soc_counting.h"

#include "bms.h"
#include "database.h"
#include "foxmath.h"
#include "fram.h"

/*========== Macros and Definitions =========================================*/
/** This structure contains all the variables relevant for the SOX */
typedef struct SOC_STATE {
    bool socInitialized;                      /*!< true if the initialization has passed, false otherwise */
    bool sensorCcUsed[BS_NR_OF_STRINGS];      /*!< bool if coulomb counting functionality from current sensor is used */
    float ccScalingAverage[BS_NR_OF_STRINGS]; /*!< current sensor offset scaling for average SOC */
    float ccScalingMinimum[BS_NR_OF_STRINGS]; /*!< current sensor offset scaling value for minimum SOC */
    float ccScalingMaximum[BS_NR_OF_STRINGS]; /*!< current sensor offset scaling value for maximum SOC */
    uint32_t previousTimestamp[BS_NR_OF_STRINGS]; /*!< timestamp buffer to check if current/CC data has been updated */
} SOC_STATE_s;

/** defines for maximum and minimum SOC */
#define MAXIMUM_SOC_PERC (100.0f)
#define MINIMUM_SOC_PERC (0.0f)

/*========== Static Constant and Variable Definitions =======================*/
/** state variable for SOC module */
static SOC_STATE_s soc_state = {
    .socInitialized    = false,
    .sensorCcUsed      = {REPEAT_U(false, STRIP(BS_NR_OF_STRINGS))},
    .ccScalingAverage  = {REPEAT_U(0.0f, STRIP(BS_NR_OF_STRINGS))},
    .ccScalingMinimum  = {REPEAT_U(0.0f, STRIP(BS_NR_OF_STRINGS))},
    .ccScalingMaximum  = {REPEAT_U(0.0f, STRIP(BS_NR_OF_STRINGS))},
    .previousTimestamp = {REPEAT_U(0u, STRIP(BS_NR_OF_STRINGS))},
};

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CURRENT_SENSOR_s soc_tableCurrentSensor = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   calculates string SOC in percentage from passed string charge in As
 * @param[in] charge_As   charge in As
 * @return returns corresponding string SOC in percentage [0.0, 100.0]
 */
static float SOC_GetStringSocPercentageFromCharge(uint32_t charge_As);

/**
 * @brief   initializes database and FRAM SOC values via lookup table (average,
 *          minimum and maximum).
 * @param[out] pTableSoc  pointer to database enty with SOC values
 */
static void SOC_RecalibrateViaLookupTable(DATA_BLOCK_SOX_s *pTableSoc);

/**
 * @brief   sets SOC value with a parameter between 0.0 and 100.0.
 * @details limits the SOE value to 0.0 respectively 100.0 if a value outside
 *          of the allowed SOE range is passed. Updates local fram and database
 *          struct but does *NOT* write them
 * @param[out]  pTableSoc  pointer to SOC database entry
 * @param[in]   socMinimumValue_perc  SOC min value to set
 * @param[in]   socMaximumValue_perc  SOC max value to set
 * @param[in]   socAverageValue_perc  SOC average value to set
 * @param[in]   stringNumber     addressed string
 */
static void SOC_SetValue(
    DATA_BLOCK_SOX_s *pTableSoc,
    float socMinimumValue_perc,
    float socMaximumValue_perc,
    float socAverageValue_perc,
    uint8_t stringNumber);

/**
 * @brief   Check if all database SOC percentage values are within [0.0, 100.0]
 *          Limits SOC values to limit values if outside of this range.
 * @param[in,out] pTableSoc  pointer to database struct with SOC values
 * @param[in] stringNumber   string that is checked
 */
static void SOC_CheckDatabaseSocPercentageLimits(DATA_BLOCK_SOX_s *pTableSoc, uint8_t stringNumber);

/**
 * @brief   Set SOC-related values in non-volatile memory
 * @param[in] pTableSoc      pointer to database struct with SOC values
 * @param[in] stringNumber   addressed string
 */
static void SOC_UpdateNvmValues(DATA_BLOCK_SOX_s *pTableSoc, uint8_t stringNumber);

/*========== Static Function Implementations ================================*/
static float SOC_GetStringSocPercentageFromCharge(uint32_t charge_As) {
    const float charge_mAs = (float)charge_As * UNIT_CONVERSION_FACTOR_1000_FLOAT;
    return UNIT_CONVERSION_FACTOR_100_FLOAT * (charge_mAs / SOC_STRING_CAPACITY_mAs);
}

static void SOC_RecalibrateViaLookupTable(DATA_BLOCK_SOX_s *pTableSoc) {
    FAS_ASSERT(pTableSoc != NULL_PTR);
    DATA_BLOCK_MIN_MAX_s tableMinMaxCellVoltages = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    DATA_READ_DATA(&tableMinMaxCellVoltages);

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        SOC_SetValue(
            pTableSoc,
            SOC_GetFromVoltage(tableMinMaxCellVoltages.minimumCellVoltage_mV[stringNumber]),
            SOC_GetFromVoltage(tableMinMaxCellVoltages.maximumCellVoltage_mV[stringNumber]),
            SOC_GetFromVoltage(tableMinMaxCellVoltages.averageCellVoltage_mV[stringNumber]),
            stringNumber);
    }
    FRAM_Write(FRAM_BLOCK_ID_SOC);
}

static void SOC_SetValue(
    DATA_BLOCK_SOX_s *pTableSoc,
    float socMinimumValue_perc,
    float socMaximumValue_perc,
    float socAverageValue_perc,
    uint8_t stringNumber) {
    FAS_ASSERT(pTableSoc != NULL_PTR);
    /* Set database values */
    pTableSoc->averageSoc_perc[stringNumber] = socAverageValue_perc;
    pTableSoc->minimumSoc_perc[stringNumber] = socMinimumValue_perc;
    pTableSoc->maximumSoc_perc[stringNumber] = socMaximumValue_perc;

    if (soc_state.sensorCcUsed[stringNumber] == true) {
        /* Current sensor database entry is read before the call of SOC_SetValue */
        float ccOffset_perc =
            SOC_GetStringSocPercentageFromCharge((uint32_t)abs(soc_tableCurrentSensor.currentCounter_As[stringNumber]));

#if POSITIVE_DISCHARGE_CURRENT == false
        ccOffset_perc *= (-1.0f);
#endif /* POSITIVE_DISCHARGE_CURRENT == false */

        /* Recalibrate scaling values */
        soc_state.ccScalingAverage[stringNumber] = pTableSoc->averageSoc_perc[stringNumber] + ccOffset_perc;
        soc_state.ccScalingMinimum[stringNumber] = pTableSoc->minimumSoc_perc[stringNumber] + ccOffset_perc;
        soc_state.ccScalingMaximum[stringNumber] = pTableSoc->maximumSoc_perc[stringNumber] + ccOffset_perc;
    }

    /* Limit SOC values to [0.0, 100.0] */
    SOC_CheckDatabaseSocPercentageLimits(pTableSoc, stringNumber);

    /* Update non-volatile memory values */
    SOC_UpdateNvmValues(pTableSoc, stringNumber);

    FRAM_Write(FRAM_BLOCK_ID_SOC);
}

static void SOC_CheckDatabaseSocPercentageLimits(DATA_BLOCK_SOX_s *pTableSoc, uint8_t stringNumber) {
    FAS_ASSERT(pTableSoc != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    if (pTableSoc->averageSoc_perc[stringNumber] > MAXIMUM_SOC_PERC) {
        pTableSoc->averageSoc_perc[stringNumber] = MAXIMUM_SOC_PERC;
    }
    if (pTableSoc->averageSoc_perc[stringNumber] < MINIMUM_SOC_PERC) {
        pTableSoc->averageSoc_perc[stringNumber] = MINIMUM_SOC_PERC;
    }
    if (pTableSoc->minimumSoc_perc[stringNumber] > MAXIMUM_SOC_PERC) {
        pTableSoc->minimumSoc_perc[stringNumber] = MAXIMUM_SOC_PERC;
    }
    if (pTableSoc->minimumSoc_perc[stringNumber] < MINIMUM_SOC_PERC) {
        pTableSoc->minimumSoc_perc[stringNumber] = MINIMUM_SOC_PERC;
    }
    if (pTableSoc->maximumSoc_perc[stringNumber] > MAXIMUM_SOC_PERC) {
        pTableSoc->maximumSoc_perc[stringNumber] = MAXIMUM_SOC_PERC;
    }
    if (pTableSoc->maximumSoc_perc[stringNumber] < MINIMUM_SOC_PERC) {
        pTableSoc->maximumSoc_perc[stringNumber] = MINIMUM_SOC_PERC;
    }
}

static void SOC_UpdateNvmValues(DATA_BLOCK_SOX_s *pTableSoc, uint8_t stringNumber) {
    FAS_ASSERT(pTableSoc != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    fram_soc.averageSoc_perc[stringNumber] = pTableSoc->averageSoc_perc[stringNumber];
    fram_soc.minimumSoc_perc[stringNumber] = pTableSoc->minimumSoc_perc[stringNumber];
    fram_soc.maximumSoc_perc[stringNumber] = pTableSoc->maximumSoc_perc[stringNumber];
}

/*========== Extern Function Implementations ================================*/

void SOC_Init(DATA_BLOCK_SOX_s *pSocValues, bool ccPresent, uint8_t stringNumber) {
    FAS_ASSERT(pSocValues != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    DATA_READ_DATA(&soc_tableCurrentSensor);

    FRAM_Read(FRAM_BLOCK_ID_SOC);

    if (ccPresent == true) {
        soc_state.sensorCcUsed[stringNumber] = true;

        float scalingOffset_perc =
            SOC_GetStringSocPercentageFromCharge((uint32_t)abs(soc_tableCurrentSensor.currentCounter_As[stringNumber]));

        if (soc_tableCurrentSensor.currentCounter_As[stringNumber] < 0) {
            scalingOffset_perc *= (-1.0f);
        }

#if POSITIVE_DISCHARGE_CURRENT == false
        scalingOffset_perc *= (-1.0f);
#endif /* POSITIVE_DISCHARGE_CURRENT == false */

        soc_state.ccScalingAverage[stringNumber] = fram_soc.averageSoc_perc[stringNumber] + scalingOffset_perc;
        soc_state.ccScalingMinimum[stringNumber] = fram_soc.minimumSoc_perc[stringNumber] + scalingOffset_perc;
        soc_state.ccScalingMaximum[stringNumber] = fram_soc.maximumSoc_perc[stringNumber] + scalingOffset_perc;

    } else {
        soc_state.previousTimestamp[stringNumber] = soc_tableCurrentSensor.timestampCurrent[stringNumber];
        soc_state.sensorCcUsed[stringNumber]      = false;
    }

    pSocValues->averageSoc_perc[stringNumber] = fram_soc.averageSoc_perc[stringNumber];
    pSocValues->minimumSoc_perc[stringNumber] = fram_soc.minimumSoc_perc[stringNumber];
    pSocValues->maximumSoc_perc[stringNumber] = fram_soc.maximumSoc_perc[stringNumber];

    SOC_CheckDatabaseSocPercentageLimits(pSocValues, stringNumber);

    /* Alternatively, SOC can be initialized with {V,SOC} lookup table if available */
    /* with the function SOC_Init_Lookup_Table() */

    soc_state.socInitialized = true;
}

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE cc-documentation-start-include */
void SOC_Calculation(DATA_BLOCK_SOX_s *pSocValues) {
    /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE cc-documentation-stop-include */
    FAS_ASSERT(pSocValues != NULL_PTR);
    bool continueFunction = true;
    if (false == soc_state.socInitialized) {
        /* Exit if SOC not initialized yet */
        continueFunction = false;
    }

    if (true == continueFunction) {
        /* Read current sensor entry for coulomb/current counting or CC recalibration */
        DATA_READ_DATA(&soc_tableCurrentSensor);

        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            /* Recalibrate SOC via LUT */
            SOC_RecalibrateViaLookupTable(pSocValues);
        } else {
            for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                if (soc_state.sensorCcUsed[stringNumber] == false) {
                    /* check if current measurement has been updated */
                    if (soc_state.previousTimestamp[stringNumber] !=
                        soc_tableCurrentSensor.timestampCurrent[stringNumber]) {
                        float timestep_s =
                            ((float)(soc_tableCurrentSensor.timestampCurrent[stringNumber] - soc_state.previousTimestamp[stringNumber])) /
                            1000.0f;

                        if (timestep_s > 0.0f) {
                            /* Current in charge direction negative means SOC increasing --> BAT naming, not ROB */

                            float deltaSOC_perc =
                                (((float)soc_tableCurrentSensor.current_mA[stringNumber] * timestep_s) /
                                 SOC_STRING_CAPACITY_mAs) *
                                100.0f / 1000.0f; /* ((mA) * 1s) / 1As) * 100% */

#if POSITIVE_DISCHARGE_CURRENT == false
                            deltaSOC_perc *= (-1.0f);
#endif /* POSITIVE_DISCHARGE_CURRENT == false */

                            pSocValues->averageSoc_perc[stringNumber] = pSocValues->averageSoc_perc[stringNumber] -
                                                                        deltaSOC_perc;
                            pSocValues->minimumSoc_perc[stringNumber] = pSocValues->minimumSoc_perc[stringNumber] -
                                                                        deltaSOC_perc;
                            pSocValues->maximumSoc_perc[stringNumber] = pSocValues->maximumSoc_perc[stringNumber] -
                                                                        deltaSOC_perc;

                            /* Limit SOC calculation to 0% respectively 100% */
                            SOC_CheckDatabaseSocPercentageLimits(pSocValues, stringNumber);

                            /* Update values in non-volatile memory */
                            SOC_UpdateNvmValues(pSocValues, stringNumber);
                        }
                        soc_state.previousTimestamp[stringNumber] =
                            soc_tableCurrentSensor.timestampCurrent[stringNumber];
                    } /* end check if current measurement has been updated */
                    /* update the variable for the next check */
                } else {
                    /* check if cc measurement has been updated */
                    if (soc_state.previousTimestamp[stringNumber] !=
                        soc_tableCurrentSensor.timestampCurrentCounting[stringNumber]) {
                        float deltaSoc_perc =
                            (((float)soc_tableCurrentSensor.currentCounter_As[stringNumber] / SOC_STRING_CAPACITY_As) *
                             100.0f));

#if POSITIVE_DISCHARGE_CURRENT == false
                        deltaSoc_perc *= (-1.0f);
#endif /* POSITIVE_DISCHARGE_CURRENT == false */

                        pSocValues->averageSoc_perc[stringNumber] = soc_state.ccScalingAverage[stringNumber] -
                                                                    deltaSoc_perc;
                        pSocValues->minimumSoc_perc[stringNumber] = soc_state.ccScalingMinimum[stringNumber] -
                                                                    deltaSoc_perc;
                        pSocValues->maximumSoc_perc[stringNumber] = soc_state.ccScalingMaximum[stringNumber] -
                                                                    deltaSoc_perc;

                        /* Limit SOC values to [0.0, 100.0] */
                        SOC_CheckDatabaseSocPercentageLimits(pSocValues, stringNumber);

                        /* Update values in non-volatile memory */
                        SOC_UpdateNvmValues(pSocValues, stringNumber);

                        soc_state.previousTimestamp[stringNumber] =
                            soc_tableCurrentSensor.timestampCurrentCounting[stringNumber];
                    } /* end check if cc measurement has been updated */
                }
            }
            /* Update database and FRAM value */
            FRAM_Write(FRAM_BLOCK_ID_SOC);
        }
    }
}

float SOC_GetFromVoltage(int16_t voltage_mV) {
    float soc_perc = 0.50f;

    /* Variables for interpolating LUT value */
    uint16_t between_high = 0;
    uint16_t between_low  = 0;

    /* Cell voltages are inserted in LUT in descending order -> start with 1 as we do not want to extrapolate. */
    for (uint16_t i = 1u; i < bc_stateOfChargeLookupTableLength; i++) {
        if (voltage_mV < bc_stateOfChargeLookupTable[i].voltage_mV) {
            between_low  = i + 1u;
            between_high = i;
        }
    }

    /* Interpolate between LUT values, but do not extrapolate LUT! */
    if (!(((0u == between_high) && (0u == between_low)) ||       /* cell voltage > maximum LUT voltage */
          (between_low >= bc_stateOfChargeLookupTableLength))) { /* cell voltage < minimum LUT voltage */
        soc_perc = MATH_LinearInterpolation(
            (float)bc_stateOfChargeLookupTable[between_low].voltage_mV,
            bc_stateOfChargeLookupTable[between_low].value,
            (float)bc_stateOfChargeLookupTable[between_high].voltage_mV,
            bc_stateOfChargeLookupTable[between_high].value,
            (float)voltage_mV);
    } else if ((between_low >= bc_stateOfChargeLookupTableLength)) {
        /* LUT SOE values are in descending order: cell voltage < minimum LUT voltage */
        soc_perc = MINIMUM_SOC_PERC;
    } else {
        /* cell voltage > maximum LUT voltage */
        soc_perc = 100.0f;
    }
    return soc_perc;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
