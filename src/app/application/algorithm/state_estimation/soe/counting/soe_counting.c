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
 * @file    soe_counting.c
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2020-10-07 (date of last update)
 * @ingroup APPLICATION
 * @prefix  SOE
 *
 * @brief   SOE module responsible for calculation of SOE
 *
 */

/*========== Includes =======================================================*/
#include "soe_counting.h"

#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"
#include "soe_counting_cfg.h"

#include "bms.h"
#include "database.h"
#include "foxmath.h"
#include "fram.h"

/*========== Macros and Definitions =========================================*/
/**
 * This structure contains all the variables relevant for the SOX.
 */
typedef struct {
    bool soeInitialized;                      /*!< true if the initialization has passed, false otherwise */
    bool sensor_ec_used[BS_NR_OF_STRINGS];    /*!< true if energy counting functionality of current sensor is used */
    float ecScalingAverage[BS_NR_OF_STRINGS]; /*!< current sensor offset scaling for average SOE */
    float ecScalingMinimum[BS_NR_OF_STRINGS]; /*!< current sensor offset scaling for minimum SOE */
    float ecScalingMaximum[BS_NR_OF_STRINGS]; /*!< current sensor offset scaling for maximum SOE */
    uint32_t previousTimestamp
        [BS_NR_OF_STRINGS]; /*!< last used timestamp of current or energy counting value for SOE estimation */
} SOE_STATE_s;

/*========== Static Constant and Variable Definitions =======================*/

/**
 * contains the state of the SOE estimation
 */
static SOE_STATE_s soe_state = {
    .soeInitialized = false,
};

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_SOX_s soe_tableSoeValues                = {.header.uniqueId = DATA_BLOCK_ID_SOX};
static DATA_BLOCK_CURRENT_SENSOR_s soe_tableCurrentSensor = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   calculates string energy in Wh from passed SOE in percentage
 *
 * @param[in] stringSoe_perc   string SOE in percentage [0.0, 100.0]
 *
 * @return returns corresponding string energy in Wh
 */
static uint32_t SOE_GetStringEnergyFromSoePercentage(float stringSoe_perc);

/**
 * @brief   calculates string SOE in percentage from passed string energy in Wh
 *
 * @param[in] energy_Wh   string energy in Wh
 *
 * @return returns corresponding string SOE in percentage [0.0, 100.0]
 */
static float SOE_GetStringSoePercentageFromEnergy(uint32_t energy_Wh);

/**
 * @brief   initializes database and FRAM SOE values via lookup table (average, min and max).
 */
static void SOE_RecalibrateViaLookupTable(void);

/**
 * @brief   look-up table for SOE initialization
 *
 * @param[in] voltage_mV  cell voltage of battery cell
 *
 * @return  SOE value in percentage [0.0 - 100.0]
 */
static float SOE_GetFromVoltage(int16_t voltage_mV);

/**
 * @brief   sets SOE value with a parameter between 0.0 and 100.0.
 * @details limits the SOE value to 0.0 respectively 100.0 if a value outside
 *          of the allowed SOE range is passed. Updates local fram and database
 *          struct but does *NOT* write them
 * @param[in]   soeMinimumValue_perc  SOE min value to set
 * @param[in]   soeMaximumValue_perc  SOE max value to set
 * @param[in]   soeAverageValue_perc  SOE average value to set
 * @param[in]   stringNumber     string addressed
 */
static void SOE_SetValue(
    float soeMinimumValue_perc,
    float soeMaximumValue_perc,
    float soeAverageValue_perc,
    uint8_t stringNumber);

/**
 * @brief   Check if all database SOE percentage values are within [0.0, 100.0]
 *          Limits SOE values to limit values if outside of this range.
 *
 * @param[in,out] pTableSoe  pointer to database struct with SOE values
 * @param[in] stringNumber   string that is checked
 */
static void SOE_CheckDatabaseSoePercentageLimits(DATA_BLOCK_SOX_s *pTableSoe, uint8_t stringNumber);

/*========== Static Function Implementations ================================*/
static float SOE_GetStringSoePercentageFromEnergy(uint32_t energy_Wh) {
    float stringSoe_perc = 0.0f;
    if (energy_Wh >= (uint32_t)SOE_STRING_ENERGY_Wh) {
        stringSoe_perc = 100.0f;
    } else {
        stringSoe_perc = 100.0f * ((float)energy_Wh / SOE_STRING_ENERGY_Wh);
    }
    return stringSoe_perc;
}

static uint32_t SOE_GetStringEnergyFromSoePercentage(float stringSoe_perc) {
    float energy_Wh = 0.0f;
    if (stringSoe_perc >= 100.0f) {
        energy_Wh = SOE_STRING_ENERGY_Wh;
    } else if (stringSoe_perc <= 0.0f) {
        energy_Wh = 0.0f;
    } else {
        energy_Wh = SOE_STRING_ENERGY_Wh * (stringSoe_perc / 100.0f);
    }
    return (uint32_t)energy_Wh;
}

static void SOE_RecalibrateViaLookupTable(void) {
    DATA_BLOCK_MIN_MAX_s tableMinimumMaximumAverage = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};

    DATA_READ_DATA(&tableMinimumMaximumAverage);

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        SOE_SetValue(
            SOE_GetFromVoltage(tableMinimumMaximumAverage.minimumCellVoltage_mV[stringNumber]),
            SOE_GetFromVoltage(tableMinimumMaximumAverage.maximumCellVoltage_mV[stringNumber]),
            SOE_GetFromVoltage(tableMinimumMaximumAverage.averageCellVoltage_mV[stringNumber]),
            stringNumber);
    }
    FRAM_Write(FRAM_BLOCK_ID_SOE);
    DATA_WRITE_DATA(&soe_tableSoeValues);
}

static float SOE_GetFromVoltage(int16_t voltage_mV) {
    float soe_perc = 50.0f;
    /* Variables for interpolating LUT value */
    uint16_t between_high = 0;
    uint16_t between_low  = 0;

    /* Cell voltages are inserted in LUT in descending order -> start with 1 as we do not want to extrapolate. */
    for (uint16_t i = 1u; i < bc_stateOfEnergyLookupTableLength; i++) {
        if (voltage_mV < bc_stateOfEnergyLookupTable[i].voltage_mV) {
            between_low  = i + 1u;
            between_high = i;
        }
    }

    /* Interpolate between LUT values, but do not extrapolate LUT! */
    if (!(((0u == between_high) && (0u == between_low)) ||      /* cell voltage > maximum LUT voltage */
          (between_low > bc_stateOfEnergyLookupTableLength))) { /* cell voltage < minimum LUT voltage */
        soe_perc = MATH_linearInterpolation(
            (float)bc_stateOfEnergyLookupTable[between_low].voltage_mV,
            bc_stateOfEnergyLookupTable[between_low].value,
            (float)bc_stateOfEnergyLookupTable[between_high].voltage_mV,
            bc_stateOfEnergyLookupTable[between_high].value,
            (float)voltage_mV);
    } else if ((between_low > bc_stateOfEnergyLookupTableLength)) {
        /* LUT SOE values are in descending order: cell voltage < minimum LUT voltage */
        soe_perc = 0.0f;
    } else {
        /* cell voltage > maximum LUT voltage */
        soe_perc = 100.0f;
    }
    return soe_perc;
}

static void SOE_SetValue(
    float soeMinimumValue_perc,
    float soeMaximumValue_perc,
    float soeAverageValue_perc,
    uint8_t stringNumber) {
    /* Update FRAM value */
    fram_soe.averageSoe_perc[stringNumber] = soeAverageValue_perc;
    fram_soe.minimumSoe_perc[stringNumber] = soeMinimumValue_perc;
    fram_soe.maximumSoe_perc[stringNumber] = soeMaximumValue_perc;

    /* Update database values */
    soe_tableSoeValues.averageSoe_perc[stringNumber] = soeAverageValue_perc;
    soe_tableSoeValues.minimumSoe_perc[stringNumber] = soeMinimumValue_perc;
    soe_tableSoeValues.maximumSoe_perc[stringNumber] = soeMaximumValue_perc;

    soe_tableSoeValues.maximumSoe_Wh[stringNumber] = SOE_GetStringEnergyFromSoePercentage(soeMaximumValue_perc);
    soe_tableSoeValues.averageSoe_Wh[stringNumber] = SOE_GetStringEnergyFromSoePercentage(soeAverageValue_perc);
    soe_tableSoeValues.minimumSoe_Wh[stringNumber] = SOE_GetStringEnergyFromSoePercentage(soeMinimumValue_perc);

    /* Calculate scaling values depending on EC counting value and current SOE */
    if (soe_state.sensor_ec_used[stringNumber] == true) {
        DATA_READ_DATA(&soe_tableCurrentSensor);

        float ecOffset =
            SOE_GetStringSoePercentageFromEnergy((uint32_t)abs(soe_tableCurrentSensor.energyCounter_Wh[stringNumber]));

#if POSITIVE_DISCHARGE_CURRENT == false
        ecOffset *= (-1.0f); /* negate calculated delta SOE in perc */

#endif /* POSITIVE_DISCHARGE_CURRENT == false */

        soe_state.ecScalingAverage[stringNumber] = fram_soe.averageSoe_perc[stringNumber] + ecOffset;
        soe_state.ecScalingMinimum[stringNumber] = fram_soe.minimumSoe_perc[stringNumber] + ecOffset;
        soe_state.ecScalingMaximum[stringNumber] = fram_soe.maximumSoe_perc[stringNumber] + ecOffset;
    }
}

static void SOE_CheckDatabaseSoePercentageLimits(DATA_BLOCK_SOX_s *pTableSoe, uint8_t stringNumber) {
    FAS_ASSERT(pTableSoe != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    if (pTableSoe->averageSoe_perc[stringNumber] > 100.0f) {
        pTableSoe->averageSoe_perc[stringNumber] = 100.0f;
    }
    if (pTableSoe->averageSoe_perc[stringNumber] < 0.0f) {
        pTableSoe->averageSoe_perc[stringNumber] = 0.0f;
    }
    if (pTableSoe->minimumSoe_perc[stringNumber] > 100.0f) {
        pTableSoe->minimumSoe_perc[stringNumber] = 100.0f;
    }
    if (pTableSoe->minimumSoe_perc[stringNumber] < 0.0f) {
        pTableSoe->minimumSoe_perc[stringNumber] = 0.0f;
    }
    if (pTableSoe->maximumSoe_perc[stringNumber] > 100.0f) {
        pTableSoe->maximumSoe_perc[stringNumber] = 100.0f;
    }
    if (pTableSoe->maximumSoe_perc[stringNumber] < 0.0f) {
        pTableSoe->maximumSoe_perc[stringNumber] = 0.0f;
    }
    return;
}

/*========== Extern Function Implementations ================================*/

extern void SOE_Init(bool ec_present, uint8_t stringNumber) {
    FRAM_Read(FRAM_BLOCK_ID_SOE);

    soe_tableSoeValues.averageSoe_perc[stringNumber] = fram_soe.averageSoe_perc[stringNumber];
    soe_tableSoeValues.minimumSoe_perc[stringNumber] = fram_soe.minimumSoe_perc[stringNumber];
    soe_tableSoeValues.maximumSoe_perc[stringNumber] = fram_soe.maximumSoe_perc[stringNumber];

    /* Limit SOE values [0.0f, 100.0f] */
    SOE_CheckDatabaseSoePercentageLimits(&soe_tableSoeValues, stringNumber);

    /* Calculate string energy in Wh */
    soe_tableSoeValues.maximumSoe_Wh[stringNumber] =
        SOE_GetStringEnergyFromSoePercentage(soe_tableSoeValues.maximumSoe_perc[stringNumber]);
    soe_tableSoeValues.minimumSoe_Wh[stringNumber] =
        SOE_GetStringEnergyFromSoePercentage(soe_tableSoeValues.minimumSoe_perc[stringNumber]);
    soe_tableSoeValues.averageSoe_Wh[stringNumber] =
        SOE_GetStringEnergyFromSoePercentage(soe_tableSoeValues.averageSoe_perc[stringNumber]);

    if (true == ec_present) {
        DATA_READ_DATA(&soe_tableCurrentSensor);
        soe_state.sensor_ec_used[stringNumber] = true;

        /* Set scaling values */
        float ecOffset =
            SOE_GetStringSoePercentageFromEnergy((uint32_t)abs(soe_tableCurrentSensor.energyCounter_Wh[stringNumber]));

#if POSITIVE_DISCHARGE_CURRENT == false
        ecOffset *= (-1.0f); /* negate calculated delta SOE in perc */

#endif /* POSITIVE_DISCHARGE_CURRENT == false */

        soe_state.ecScalingMinimum[stringNumber] = fram_soe.minimumSoe_perc[stringNumber] + ecOffset;
        soe_state.ecScalingMaximum[stringNumber] = fram_soe.maximumSoe_perc[stringNumber] + ecOffset;
        soe_state.ecScalingAverage[stringNumber] = fram_soe.averageSoe_perc[stringNumber] + ecOffset;
    }
    soe_state.soeInitialized = true;
    DATA_WRITE_DATA(&soe_tableCurrentSensor);
}

void SOE_Calculation(void) {
    bool continueFunction = true;
    if (false == soe_state.soeInitialized) {
        /* Exit if SOE not initialized yet */
        continueFunction = false;
    }

    if (true == continueFunction) {
        if (BMS_GetBatterySystemState() == BMS_AT_REST) {
            /* Recalibrate SOE via LUT */
            SOE_RecalibrateViaLookupTable();
        } else {
            /* Use energy counting/integrate */
            DATA_READ_DATA(&soe_tableCurrentSensor);

            for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                if (false == soe_state.sensor_ec_used[stringNumber]) {
                    /* no energy counting activated -> manually integrate energy */
                    uint32_t timestamp          = soe_tableCurrentSensor.timestampCurrent[stringNumber];
                    uint32_t previous_timestamp = soe_tableCurrentSensor.previousTimestampCurrent[stringNumber];

                    /* check if current measurement has been updated */
                    if (soe_state.previousTimestamp[stringNumber] != timestamp) {
                        float timestep_s = (((float)timestamp - (float)previous_timestamp)) / 1000.0f;
                        if (timestep_s > 0.0f) {
                            /* Current in charge direction negative means SOE increasing --> BAT naming, not ROB */
                            float deltaSOE_Wh = ((((float)soe_tableCurrentSensor.current_mA[stringNumber] /
                                                   1000.0f) * /* convert to A */
                                                  ((float)soe_tableCurrentSensor.highVoltage_mV[stringNumber][0] /
                                                   1000.0f)) / /* convert to V */
                                                 timestep_s) / /* unit: s */
                                                3600.0f;       /* convert Ws -> Wh */

#if POSITIVE_DISCHARGE_CURRENT == false
                            /* negate calculated delta SOE_Wh */
                            deltaSOE_Wh *= (-1.0f);
#endif /* POSITIVE_DISCHARGE_CURRENT == false */

                            soe_tableSoeValues.averageSoe_Wh[stringNumber] -= (uint32_t)deltaSOE_Wh;
                            soe_tableSoeValues.minimumSoe_Wh[stringNumber] -= (uint32_t)deltaSOE_Wh;
                            soe_tableSoeValues.maximumSoe_Wh[stringNumber] -= (uint32_t)deltaSOE_Wh;

                            soe_tableSoeValues.averageSoe_perc[stringNumber] =
                                SOE_GetStringSoePercentageFromEnergy(soe_tableSoeValues.averageSoe_Wh[stringNumber]);
                            soe_tableSoeValues.minimumSoe_perc[stringNumber] =
                                SOE_GetStringSoePercentageFromEnergy(soe_tableSoeValues.minimumSoe_Wh[stringNumber]);
                            soe_tableSoeValues.maximumSoe_perc[stringNumber] =
                                SOE_GetStringSoePercentageFromEnergy(soe_tableSoeValues.maximumSoe_Wh[stringNumber]);

                            /* update timestamp SOE state variable for next iteration */
                            soe_state.previousTimestamp[stringNumber] = timestamp;
                        }
                    } /* end check if current measurement has been updated */
                } else {
                    /* check if ec measurement has been updated */
                    if (soe_state.previousTimestamp[stringNumber] !=
                        soe_tableCurrentSensor.timestampEnergyCounting[stringNumber]) {
                        /* Calculate SOE value with current sensor EC value */
                        float deltaSoe_perc =
                            (((float)soe_tableCurrentSensor.energyCounter_Wh[stringNumber] / SOE_STRING_ENERGY_Wh) *
                             100.0f);

#if POSITIVE_DISCHARGE_CURRENT == false
                        /* negate calculated delta SOE_perc */
                        deltaSoe_perc *= (-1.0f);
#endif
                        /* Apply EC scaling offset to get actual string energy */
                        soe_tableSoeValues.averageSoe_perc[stringNumber] = soe_state.ecScalingAverage[stringNumber] -
                                                                           deltaSoe_perc;
                        soe_tableSoeValues.minimumSoe_perc[stringNumber] = soe_state.ecScalingMinimum[stringNumber] -
                                                                           deltaSoe_perc;
                        soe_tableSoeValues.maximumSoe_perc[stringNumber] = soe_state.ecScalingMaximum[stringNumber] -
                                                                           deltaSoe_perc;

                        /* Limit SOE values to [0.0, 100.0] */
                        SOE_CheckDatabaseSoePercentageLimits(&soe_tableSoeValues, stringNumber);

                        /* Calculate new Wh values */
                        soe_tableSoeValues.maximumSoe_Wh[stringNumber] =
                            SOE_GetStringEnergyFromSoePercentage(soe_tableSoeValues.maximumSoe_perc[stringNumber]);
                        soe_tableSoeValues.averageSoe_Wh[stringNumber] =
                            SOE_GetStringEnergyFromSoePercentage(soe_tableSoeValues.averageSoe_perc[stringNumber]);
                        soe_tableSoeValues.minimumSoe_Wh[stringNumber] =
                            SOE_GetStringEnergyFromSoePercentage(soe_tableSoeValues.minimumSoe_perc[stringNumber]);

                        /* Update timestamp for next iteration */
                        soe_state.previousTimestamp[stringNumber] =
                            soe_tableCurrentSensor.timestampEnergyCounting[stringNumber];
                    }
                }

                fram_soe.averageSoe_perc[stringNumber] = soe_tableSoeValues.averageSoe_perc[stringNumber];
                fram_soe.minimumSoe_perc[stringNumber] = soe_tableSoeValues.minimumSoe_perc[stringNumber];
                fram_soe.maximumSoe_perc[stringNumber] = soe_tableSoeValues.maximumSoe_perc[stringNumber];
            }

            /* Update database and FRAM value */
            FRAM_Write(FRAM_BLOCK_ID_SOE);
            DATA_WRITE_DATA(&soe_tableSoeValues);
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
