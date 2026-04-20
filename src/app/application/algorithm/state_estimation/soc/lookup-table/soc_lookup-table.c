/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    soc_lookup-table.c
 * @author  foxBMS Team
 * @date    2025-06-02 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  SOC
 *
 * @brief   SOC module responsible for calculation of state-of-charge (SOC)
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "soc_lookup-table_cfg.h"

#include "bms.h"
#include "database.h"
#include "foxmath.h"
#include "fram.h"
#include "state_estimation.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
typedef struct {
    bool socInitialized;                 /*!< true if the initialization has passed, false otherwise */
    bool sensorCcUsed[BS_NR_OF_STRINGS]; /*!< bool if coulomb counting functionality from current sensor is used */
    float_t ccScalingAverage[BS_NR_OF_STRINGS];       /*!< current sensor offset scaling for average SOC */
    float_t ccScalingMinimum[BS_NR_OF_STRINGS];       /*!< current sensor offset scaling value for minimum SOC */
    float_t ccScalingMaximum[BS_NR_OF_STRINGS];       /*!< current sensor offset scaling value for maximum SOC */
    float_t chargeThroughput_As[BS_NR_OF_STRINGS];    /*!< Charge throughput */
    float_t dischargeThroughput_As[BS_NR_OF_STRINGS]; /*!< Discharge throughput */
    float_t previousCurrentCountingValue_As[BS_NR_OF_STRINGS]; /*!< Charge throughput */
    uint32_t previousTimestamp; /*!< timestamp buffer to check if voltage data has been updated */
} SOC_STATE_s;

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_MIN_MAX_s soc_tableMinMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
/**@}*/

/** Maximum SOC in percentage */
#define SOC_MAXIMUM_SOC_perc (100.0f)
/** Minimum SOC in percentage */
#define SOC_MINIMUM_SOC_perc (0.0f)

/*========== Static Constant and Variable Definitions =======================*/
static SOC_STATE_s soc_state = {
    .socInitialized                  = false,
    .sensorCcUsed                    = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
    .ccScalingAverage                = {GEN_REPEAT_U(0.0f, GEN_STRIP(BS_NR_OF_STRINGS))},
    .ccScalingMinimum                = {GEN_REPEAT_U(0.0f, GEN_STRIP(BS_NR_OF_STRINGS))},
    .ccScalingMaximum                = {GEN_REPEAT_U(0.0f, GEN_STRIP(BS_NR_OF_STRINGS))},
    .chargeThroughput_As             = {GEN_REPEAT_U(0.0f, GEN_STRIP(BS_NR_OF_STRINGS))},
    .dischargeThroughput_As          = {GEN_REPEAT_U(0.0f, GEN_STRIP(BS_NR_OF_STRINGS))},
    .previousCurrentCountingValue_As = {GEN_REPEAT_U(0u, GEN_STRIP(BS_NR_OF_STRINGS))},
    .previousTimestamp               = 0u,
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Check if all database SOC percentage values are within [0.0, 100.0]
 *          Limits SOC values to limit values if outside of this range.
 * @param[in,out] pTableSoc  pointer to database struct with SOC values
 * @param[in] stringNumber   string that is checked
 */
static void SOC_CheckDatabaseSocPercentageLimits(DATA_BLOCK_SOC_s *pTableSoc, uint8_t stringNumber);

/**
 * @brief   Set SOC-related values in non-volatile memory
 * @param[in] pTableSoc      pointer to database struct with SOC values
 * @param[in] stringNumber   addressed string
 */
static void SOC_UpdateNvmValues(DATA_BLOCK_SOC_s *pTableSoc, uint8_t stringNumber);

/*========== Static Function Implementations ================================*/
static void SOC_CheckDatabaseSocPercentageLimits(DATA_BLOCK_SOC_s *pTableSoc, uint8_t stringNumber) {
    FAS_ASSERT(pTableSoc != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    if (pTableSoc->averageSoc_perc[stringNumber] > SOC_MAXIMUM_SOC_perc) {
        pTableSoc->averageSoc_perc[stringNumber] = SOC_MAXIMUM_SOC_perc;
    }
    if (pTableSoc->averageSoc_perc[stringNumber] < SOC_MINIMUM_SOC_perc) {
        pTableSoc->averageSoc_perc[stringNumber] = SOC_MINIMUM_SOC_perc;
    }
    if (pTableSoc->minimumSoc_perc[stringNumber] > SOC_MAXIMUM_SOC_perc) {
        pTableSoc->minimumSoc_perc[stringNumber] = SOC_MAXIMUM_SOC_perc;
    }
    if (pTableSoc->minimumSoc_perc[stringNumber] < SOC_MINIMUM_SOC_perc) {
        pTableSoc->minimumSoc_perc[stringNumber] = SOC_MINIMUM_SOC_perc;
    }
    if (pTableSoc->maximumSoc_perc[stringNumber] > SOC_MAXIMUM_SOC_perc) {
        pTableSoc->maximumSoc_perc[stringNumber] = SOC_MAXIMUM_SOC_perc;
    }
    if (pTableSoc->maximumSoc_perc[stringNumber] < SOC_MINIMUM_SOC_perc) {
        pTableSoc->maximumSoc_perc[stringNumber] = SOC_MINIMUM_SOC_perc;
    }
}

static void SOC_UpdateNvmValues(DATA_BLOCK_SOC_s *pTableSoc, uint8_t stringNumber) {
    FAS_ASSERT(pTableSoc != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    fram_soc.averageSoc_perc[stringNumber]        = pTableSoc->averageSoc_perc[stringNumber];
    fram_soc.minimumSoc_perc[stringNumber]        = pTableSoc->minimumSoc_perc[stringNumber];
    fram_soc.maximumSoc_perc[stringNumber]        = pTableSoc->maximumSoc_perc[stringNumber];
    fram_soc.chargeThroughput_As[stringNumber]    = pTableSoc->chargeThroughput_As[stringNumber];
    fram_soc.dischargeThroughput_As[stringNumber] = pTableSoc->dischargeThroughput_As[stringNumber];
}

/*========== Extern Function Implementations ================================*/
extern void SE_InitializeStateOfCharge(DATA_BLOCK_SOC_s *pSocValues, bool ccPresent, uint8_t stringNumber) {
    FAS_ASSERT(pSocValues != NULL_PTR);
    FAS_ASSERT((ccPresent == true) || (ccPresent == false)); /* LCOV_EXCL_LINE */
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    soc_state.socInitialized = true;
}

extern void SE_CalculateStateOfCharge(DATA_BLOCK_SOC_s *pSocValues) {
    FAS_ASSERT(pSocValues != NULL_PTR);
    bool continueFunction = true;

    if (soc_state.socInitialized == false) {
        /* Exit if SOC not initialized yet. Is init here really necessary? */
        /* continueFunction = false; */
    }

    if (continueFunction == true) {
        DATA_READ_DATA(&soc_tableMinMax);
        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            if (soc_state.previousTimestamp != soc_tableMinMax.header.timestamp) {
                soc_state.previousTimestamp = soc_tableMinMax.header.timestamp;

                pSocValues->maximumSoc_perc[s] =
                    SE_GetStateOfChargeFromVoltage(soc_tableMinMax.maximumCellVoltage_mV[s]);
                pSocValues->minimumSoc_perc[s] =
                    SE_GetStateOfChargeFromVoltage(soc_tableMinMax.minimumCellVoltage_mV[s]);
                pSocValues->averageSoc_perc[s] =
                    SE_GetStateOfChargeFromVoltage(soc_tableMinMax.averageCellVoltage_mV[s]);

                /* Limit SOC calculation to 0% respectively 100% */
                SOC_CheckDatabaseSocPercentageLimits(pSocValues, s);

                /* Update values in non-volatile memory */
                SOC_UpdateNvmValues(pSocValues, s);
            }
        }
        FRAM_WriteData(FRAM_BLOCK_ID_SOC);
    }
}

extern float_t SE_GetStateOfChargeFromVoltage(int16_t voltage_mV) {
    float_t soc_perc = 0.50f;

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
    if (!(((between_high == 0u) && (between_low == 0u)) ||       /* cell voltage > maximum LUT voltage */
          (between_low >= bc_stateOfChargeLookupTableLength))) { /* cell voltage < minimum LUT voltage */
        soc_perc = MATH_LinearInterpolation(
            (float_t)bc_stateOfChargeLookupTable[between_low].voltage_mV,
            bc_stateOfChargeLookupTable[between_low].value,
            (float_t)bc_stateOfChargeLookupTable[between_high].voltage_mV,
            bc_stateOfChargeLookupTable[between_high].value,
            (float_t)voltage_mV);
    } else if ((between_low >= bc_stateOfChargeLookupTableLength)) {
        /* LUT SOE values are in descending order: cell voltage < minimum LUT voltage */
        soc_perc = SOC_MINIMUM_SOC_perc;
    } else {
        /* cell voltage > maximum LUT voltage */
        soc_perc = SOC_MAXIMUM_SOC_perc;
    }
    return soc_perc;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern bool TEST_SE_GetSocStateInitialized(void) {
    return soc_state.socInitialized;
}
extern void TEST_SOC_CheckDatabaseSocPercentageLimits(DATA_BLOCK_SOC_s *TableSoc, uint8_t stringNumber) {
    SOC_CheckDatabaseSocPercentageLimits(TableSoc, stringNumber);
}
extern void TEST_SOC_UpdateNvmValues(DATA_BLOCK_SOC_s *TableSoc, uint8_t stringNumber) {
    SOC_UpdateNvmValues(TableSoc, stringNumber);
}
#endif
