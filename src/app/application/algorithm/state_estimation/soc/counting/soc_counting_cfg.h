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
 * @file    soc_counting_cfg.h
 * @author  foxBMS Team
 * @date    2020-10-07 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  SOC
 *
 * @brief   Header for SOC configuration
 * @details TODO
 */

#ifndef FOXBMS__SOC_COUNTING_CFG_H_
#define FOXBMS__SOC_COUNTING_CFG_H_

/*========== Includes =======================================================*/

#include "battery_system_cfg.h"

#include "battery_cell_cfg_types.h"
#include "database.h"

#include <math.h>

/*========== Macros and Definitions =========================================*/

/**
 * Cell capacity used for SOC calculation, in this case Ah counting
 * Specified once according to datasheet of cell usually.
 */
#define SOC_STRING_CAPACITY_mAh ((float_t)(BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK * BC_CAPACITY_mAh))

/** #SOC_STRING_CAPACITY_mAh in mAs */
#define SOC_STRING_CAPACITY_mAs ((float_t)(SOC_STRING_CAPACITY_mAh * 3600.0f))
/** #SOC_STRING_CAPACITY_mAs in As */
#define SOC_STRING_CAPACITY_As ((float_t)(SOC_STRING_CAPACITY_mAs / 1000.0f))

/*========== Extern Constant and Variable Declarations ======================*/
/** This structure contains all the variables relevant for the SOX */
typedef struct {
    bool socInitialized;                 /*!< true if the initialization has passed, false otherwise */
    bool sensorCcUsed[BS_NR_OF_STRINGS]; /*!< bool if coulomb counting functionality from current sensor is used */
    float_t ccScalingAverage[BS_NR_OF_STRINGS];       /*!< current sensor offset scaling for average SOC */
    float_t ccScalingMinimum[BS_NR_OF_STRINGS];       /*!< current sensor offset scaling value for minimum SOC */
    float_t ccScalingMaximum[BS_NR_OF_STRINGS];       /*!< current sensor offset scaling value for maximum SOC */
    float_t chargeThroughput_As[BS_NR_OF_STRINGS];    /*!< Charge throughput */
    float_t dischargeThroughput_As[BS_NR_OF_STRINGS]; /*!< Discharge throughput */
    float_t previousCurrentCountingValue_As[BS_NR_OF_STRINGS]; /*!< Charge throughput */
    uint32_t previousTimestamp[BS_NR_OF_STRINGS]; /*!< timestamp buffer to check if current/CC data has been updated */
} SOC_STATE_s;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_SOC_CheckDatabaseSocPercentageLimits(DATA_BLOCK_SOC_s *TableSoc, uint8_t stringNumber);
extern void TEST_SOC_UpdateNvmValues(DATA_BLOCK_SOC_s *TableSoc, uint8_t stringNumber);
extern void TEST_SetSocStateValues(SOC_STATE_s *stateValues);
extern SOC_STATE_s *TEST_GetSocStateValues(void);
extern void TEST_SOC_SetValue(
    DATA_BLOCK_SOC_s *pTableSoc,
    float_t socMinimumValue_perc,
    float_t socMaximumValue_perc,
    float_t socAverageValue_perc,
    uint8_t stringNumber);
extern float_t TEST_SOC_GetStringSocPercentageFromCharge(int32_t charge_As);

#endif

#endif /* FOXBMS__SOC_COUNTING_CFG_H_ */
