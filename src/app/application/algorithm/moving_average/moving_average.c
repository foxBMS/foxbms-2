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
 * @file    moving_average.c
 * @author  foxBMS Team
 * @date    2017-12-18 (date of creation)
 * @updated 2021-11-10 (date of last update)
 * @ingroup ALGORITHMS
 * @prefix  ALGO
 *
 * @brief   moving average algorithm
 *
 */

/*========== Includes =======================================================*/
#include "moving_average.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/
/** TODO */
#define MEM_EXT_SDRAM

#if ALGO_TICK_MS > ISA_CURRENT_CYCLE_TIME_MS
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_1s  ((1000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_5s  ((5000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_10s ((10000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_30s ((30000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_60s ((60000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_CFG (MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ALGO_TICK_MS)
#else
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_1s  ((1000u) / ISA_CURRENT_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_5s  ((5000u) / ISA_CURRENT_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_10s ((10000u) / ISA_CURRENT_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_30s ((30000u) / ISA_CURRENT_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_60s ((60000u) / ISA_CURRENT_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_CUR_CFG (MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ISA_CURRENT_CYCLE_TIME_MS)
#endif

#if ALGO_TICK_MS > ISA_POWER_CYCLE_TIME_MS
#define ALGO_NUMBER_AVERAGE_VALUES_POW_1s  ((1000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_5s  ((5000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_10s ((10000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_30s ((30000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_60s ((60000u) / ALGO_TICK_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_CFG (MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ALGO_TICK_MS)
#else
#define ALGO_NUMBER_AVERAGE_VALUES_POW_1s  ((1000u) / ISA_POWER_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_5s  ((5000u) / ISA_POWER_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_10s ((10000u) / ISA_POWER_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_30s ((30000u) / ISA_POWER_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_60s ((60000u) / ISA_POWER_CYCLE_TIME_MS)
#define ALGO_NUMBER_AVERAGE_VALUES_POW_CFG (MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ISA_POWER_CYCLE_TIME_MS)
#endif

/*========== Static Constant and Variable Definitions =======================*/

/* Arrays in extern SDRAM to calculate moving average current and power */

/* Check if minimum algo cycle time > current sensor sample time */
#if ALGO_TICK_MS > ISA_CURRENT_CYCLE_TIME_MS
#if MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ALGO_TICK_MS > 60000u / ALGO_TICK_MS
/* If array length of configured time > 60s array take this array size */
static float MEM_EXT_SDRAM curValues[(MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ALGO_TICK_MS) + 1u] = {};
static uint32_t movingAverageCurrentLength = (MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ALGO_TICK_MS) + 1u;
#else
/* Take array size of 60s moving average */
static float MEM_EXT_SDRAM curValues[(60000u / ALGO_TICK_MS) + 1u] = {};
static uint32_t movingAverageCurrentLength                         = (60000u / ALGO_TICK_MS) + 1;
#endif
#else
/* If array length of configured time > 60s array take this array size */
#if (MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ISA_CURRENT_CYCLE_TIME_MS) > (60000u / ISA_CURRENT_CYCLE_TIME_MS)
static float MEM_EXT_SDRAM curValues[(MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ISA_CURRENT_CYCLE_TIME_MS) + 1u] = {};
static uint32_t movingAverageCurrentLength = (MOVING_AVERAGE_DURATION_CURRENT_CONFIG_MS / ISA_CURRENT_CYCLE_TIME_MS) +
                                             1u;
#else
/* Take array size of 60s moving average */
static float MEM_EXT_SDRAM curValues[(60000u / ISA_CURRENT_CYCLE_TIME_MS) + 1u] = {0.0f};
static uint32_t movingAverageCurrentLength = (60000u / ISA_CURRENT_CYCLE_TIME_MS) + 1u;
#endif
#endif

/* Check if minimum algo cycle time > current sensor sample time */
#if ALGO_TICK_MS > ISA_POWER_CYCLE_TIME_MS
#if (MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ALGO_TICK_MS) > (60000u / ALGO_TICK_MS)
/* If array length of configured time > 60s array take this array size */
static float MEM_EXT_SDRAM powValues[(MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ALGO_TICK_MS) + 1u] = {};
static uint32_t movingAveragePowerLength = (MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ALGO_TICK_MS) + 1u;
#else
/* Take array size of 60s moving average */
static float MEM_EXT_SDRAM powValues[(60000u / ALGO_TICK_MS) + 1] = {};
static uint32_t movingAveragePowerLength                          = (60000u / ALGO_TICK_MS) + 1u;
#endif
#else
#if (MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ISA_POWER_CYCLE_TIME_MS) > (60000u / ISA_POWER_CYCLE_TIME_MS)
/* If array length of configured time > 60s array take this array size */
static float MEM_EXT_SDRAM powValues[(MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ISA_POWER_CYCLE_TIME_MS) + 1u] = {};
static uint32_t movingAveragePowerLength = (MOVING_AVERAGE_DURATION_POWER_CONFIG_MS / ISA_POWER_CYCLE_TIME_MS) + 1u;
#else
/* Take array size of 60s moving average */
static float MEM_EXT_SDRAM powValues[(60000u / ISA_POWER_CYCLE_TIME_MS) + 1u] = {0.0f};
static uint32_t movingAveragePowerLength                                      = (60000u / ISA_POWER_CYCLE_TIME_MS) + 1u;
#endif
#endif

/** Pointer for current moving average calculation @{*/
static float *pMovingAverageCurrentNew  = &curValues[0];
static float *pMovingAverageCurrent_1s  = &curValues[0];
static float *pMovingAverageCurrent_5s  = &curValues[0];
static float *pMovingAverageCurrent_10s = &curValues[0];
static float *pMovingAverageCurrent_30s = &curValues[0];
static float *pMovingAverageCurrent_60s = &curValues[0];
static float *pMovingAverageCurrent_cfg = &curValues[0];
/**@}*/

/** Pointer for power moving average calculation @{*/
static float *pMovingAveragePowerNew  = &powValues[0];
static float *pMovingAveragePower_1s  = &powValues[0];
static float *pMovingAveragePower_5s  = &powValues[0];
static float *pMovingAveragePower_10s = &powValues[0];
static float *pMovingAveragePower_30s = &powValues[0];
static float *pMovingAveragePower_60s = &powValues[0];
static float *pMovingAveragePower_cfg = &powValues[0];
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void ALGO_MovAverage(void) {
    static uint8_t curCounter                            = 0u;
    static uint8_t powCounter                            = 0u;
    static DATA_BLOCK_CURRENT_SENSOR_s curPow_tab        = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
    static DATA_BLOCK_MOVING_AVERAGE_s movingAverage_tab = {.header.uniqueId = DATA_BLOCK_ID_MOVING_AVERAGE};
    static uint8_t curInit   = 0u; /* bit0: 1s, bit1: 5s, bit2: 10s, bit3: 30s, bit4: 60s, bit5: cfg */
    static uint8_t powInit   = 0u; /* bit0: 1s, bit1: 5s, bit2: 10s, bit3: 30s, bit4: 60s, bit5: cfg */
    static uint8_t newValues = 0u;
    float divider            = 0.0f;
    bool validValues         = true;

    DATA_READ_DATA(&curPow_tab);
    DATA_READ_DATA(&movingAverage_tab);

    /* Check if new current value */
    if (curCounter != curPow_tab.newCurrent) {
        curCounter = curPow_tab.newCurrent;

        /* Check if valid values */
        for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
            if (curPow_tab.invalidCurrentMeasurement[stringNumber] != 0u) {
                validValues = false;
            }
        }

        if (validValues == true) {
            /* new Values -> Save later in database */
            newValues = 1u;

            int32_t packCurrent = 0;
            for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                packCurrent += curPow_tab.current_mA[stringNumber];
            }

            /* Add value to array and calculate new moving average values */
            *pMovingAverageCurrentNew = packCurrent;

            /* Calculate new moving average - first add new value */
            divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_1s;
            movingAverage_tab.movingAverageCurrent1sInterval_mA += (*pMovingAverageCurrentNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_5s;
            movingAverage_tab.movingAverageCurrent5sInterval_mA += (*pMovingAverageCurrentNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_10s;
            movingAverage_tab.movingAverageCurrent10sInterval_mA += (*pMovingAverageCurrentNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_30s;
            movingAverage_tab.movingAverageCurrent30sInterval_mA += (*pMovingAverageCurrentNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_60s;
            movingAverage_tab.movingAverageCurrent60sInterval_mA += (*pMovingAverageCurrentNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_CFG;
            movingAverage_tab.movingAverageCurrentConfigurableInterval_mA += (*pMovingAverageCurrentNew) / divider;

            /* Then, increment pointer and substract oldest value when respective window is filled with data */
            pMovingAverageCurrentNew++;
            if ((curInit & 0x01u) == 0x01u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_1s;
                movingAverage_tab.movingAverageCurrent1sInterval_mA -= ((*pMovingAverageCurrent_1s) / divider);
                pMovingAverageCurrent_1s++;
            } else {
                if (pMovingAverageCurrentNew == &curValues[ALGO_NUMBER_AVERAGE_VALUES_CUR_1s]) {
                    curInit |= 0x01u;
                }
            }
            if ((curInit & 0x02u) == 0x02u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_5s;
                movingAverage_tab.movingAverageCurrent5sInterval_mA -= (*pMovingAverageCurrent_5s) / divider;
                pMovingAverageCurrent_5s++;
            } else {
                if (pMovingAverageCurrentNew == &curValues[ALGO_NUMBER_AVERAGE_VALUES_CUR_5s]) {
                    curInit |= 0x02u;
                }
            }
            if ((curInit & 0x04u) == 0x04u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_10s;
                movingAverage_tab.movingAverageCurrent10sInterval_mA -= (*pMovingAverageCurrent_10s) / divider;
                pMovingAverageCurrent_10s++;
            } else {
                if (pMovingAverageCurrentNew == &curValues[ALGO_NUMBER_AVERAGE_VALUES_CUR_10s]) {
                    curInit |= 0x04u;
                }
            }
            if ((curInit & 0x08u) == 0x08u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_30s;
                movingAverage_tab.movingAverageCurrent30sInterval_mA -= (*pMovingAverageCurrent_30s) / divider;
                pMovingAverageCurrent_30s++;
            } else {
                if (pMovingAverageCurrentNew == &curValues[ALGO_NUMBER_AVERAGE_VALUES_CUR_30s]) {
                    curInit |= 0x08u;
                }
            }
            if ((curInit & 0x10u) == 0x10u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_60s;
                movingAverage_tab.movingAverageCurrent60sInterval_mA -= (*pMovingAverageCurrent_60s) / divider;
                pMovingAverageCurrent_60s++;
            } else {
                if (pMovingAverageCurrentNew == &curValues[ALGO_NUMBER_AVERAGE_VALUES_CUR_60s]) {
                    curInit |= 0x10u;
                }
            }
            if ((curInit & 0x20u) == 0x20u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_CUR_CFG;
                movingAverage_tab.movingAverageCurrentConfigurableInterval_mA -= (*pMovingAverageCurrent_cfg) / divider;
                pMovingAverageCurrent_cfg++;
            } else {
                if (pMovingAverageCurrentNew == &curValues[ALGO_NUMBER_AVERAGE_VALUES_CUR_CFG]) {
                    curInit |= 0x20u;
                }
            }

            /* Check pointer for buffer overflow */
            if (pMovingAverageCurrentNew > &curValues[movingAverageCurrentLength - 1u]) {
                pMovingAverageCurrentNew = &curValues[0u];
            }
            if (pMovingAverageCurrent_1s > &curValues[movingAverageCurrentLength - 1u]) {
                pMovingAverageCurrent_1s = &curValues[0u];
            }
            if (pMovingAverageCurrent_5s > &curValues[movingAverageCurrentLength - 1u]) {
                pMovingAverageCurrent_5s = &curValues[0u];
            }
            if (pMovingAverageCurrent_10s > &curValues[movingAverageCurrentLength - 1u]) {
                pMovingAverageCurrent_10s = &curValues[0u];
            }
            if (pMovingAverageCurrent_30s > &curValues[movingAverageCurrentLength - 1u]) {
                pMovingAverageCurrent_30s = &curValues[0u];
            }
            if (pMovingAverageCurrent_60s > &curValues[movingAverageCurrentLength - 1u]) {
                pMovingAverageCurrent_60s = &curValues[0u];
            }
            if (pMovingAverageCurrent_cfg > &curValues[movingAverageCurrentLength - 1u]) {
                pMovingAverageCurrent_cfg = &curValues[0u];
            }
        }
    }

    validValues = true;

    /* Check if new power value */
    if (powCounter != curPow_tab.newPower) {
        powCounter = curPow_tab.newPower;

        /* Check if valid values */
        for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
            if (curPow_tab.invalidPowerMeasurement[stringNumber] != 0u) {
                validValues = false;
            }
        }
        if (validValues == true) {
            newValues = 1u;

            int32_t packPower = 0;
            for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                packPower += curPow_tab.power_W[stringNumber];
            }

            /* Add value to array and calculate new moving mean values */
            *pMovingAveragePowerNew = packPower;

            /* Calculate new moving means - first add new value */
            divider = ALGO_NUMBER_AVERAGE_VALUES_POW_1s;
            movingAverage_tab.movingAveragePower1sInterval_mA += (*pMovingAveragePowerNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_POW_5s;
            movingAverage_tab.movingAveragePower5sInterval_mA += (*pMovingAveragePowerNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_POW_10s;
            movingAverage_tab.movingAveragePower10sInterval_mA += (*pMovingAveragePowerNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_POW_30s;
            movingAverage_tab.movingAveragePower30sInterval_mA += (*pMovingAveragePowerNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_POW_60s;
            movingAverage_tab.movingAveragePower60sInterval_mA += (*pMovingAveragePowerNew) / divider;
            divider = ALGO_NUMBER_AVERAGE_VALUES_POW_CFG;
            movingAverage_tab.movingAveragePowerConfigurableInterval_mA += (*pMovingAveragePowerNew) / divider;

            /* Then, increment pointer and substract oldest value when respective window is filled with data */
            pMovingAveragePowerNew++;
            if ((powInit & 0x01u) == 0x01u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_POW_1s;
                movingAverage_tab.movingAveragePower1sInterval_mA -= ((*pMovingAveragePower_1s) / divider);
                pMovingAveragePower_1s++;
            } else {
                if (pMovingAveragePowerNew == &powValues[ALGO_NUMBER_AVERAGE_VALUES_POW_1s]) {
                    powInit |= 0x01u;
                }
            }
            if ((powInit & 0x02u) == 0x02u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_POW_5s;
                movingAverage_tab.movingAveragePower5sInterval_mA -= ((*pMovingAveragePower_5s) / divider);
                pMovingAveragePower_5s++;
            } else {
                if (pMovingAveragePowerNew == &powValues[ALGO_NUMBER_AVERAGE_VALUES_POW_5s]) {
                    powInit |= 0x02u;
                }
            }
            if ((powInit & 0x04u) == 0x04u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_POW_10s;
                movingAverage_tab.movingAveragePower10sInterval_mA -= ((*pMovingAveragePower_10s) / divider);
                pMovingAveragePower_10s++;
            } else {
                if (pMovingAveragePowerNew == &powValues[ALGO_NUMBER_AVERAGE_VALUES_POW_10s]) {
                    powInit |= 0x04u;
                }
            }
            if ((powInit & 0x08u) == 0x08u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_POW_30s;
                movingAverage_tab.movingAveragePower30sInterval_mA -= ((*pMovingAveragePower_30s) / divider);
                pMovingAveragePower_30s++;
            } else {
                if (pMovingAveragePowerNew == &powValues[ALGO_NUMBER_AVERAGE_VALUES_POW_30s]) {
                    powInit |= 0x08u;
                }
            }
            if ((powInit & 0x10u) == 0x10u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_POW_60s;
                movingAverage_tab.movingAveragePower60sInterval_mA -= ((*pMovingAveragePower_60s) / divider);
                pMovingAveragePower_60s++;
            } else {
                if (pMovingAveragePowerNew == &powValues[ALGO_NUMBER_AVERAGE_VALUES_POW_60s]) {
                    powInit |= 0x10u;
                }
            }
            if ((powInit & 0x20u) == 0x20u) {
                divider = ALGO_NUMBER_AVERAGE_VALUES_POW_CFG;
                movingAverage_tab.movingAveragePowerConfigurableInterval_mA -= ((*pMovingAveragePower_cfg) / divider);
                pMovingAveragePower_cfg++;
            } else {
                if (pMovingAveragePowerNew == &powValues[ALGO_NUMBER_AVERAGE_VALUES_POW_CFG]) {
                    powInit |= 0x20u;
                }
            }

            /* Check pointer for buffer overflow */
            if (pMovingAveragePowerNew > &powValues[movingAveragePowerLength - 1u]) {
                pMovingAveragePowerNew = &powValues[0u];
            }
            if (pMovingAveragePower_1s > &powValues[movingAveragePowerLength - 1u]) {
                pMovingAveragePower_1s = &powValues[0u];
            }
            if (pMovingAveragePower_5s > &powValues[movingAveragePowerLength - 1u]) {
                pMovingAveragePower_5s = &powValues[0u];
            }
            if (pMovingAveragePower_10s > &powValues[movingAveragePowerLength - 1u]) {
                pMovingAveragePower_10s = &powValues[0u];
            }
            if (pMovingAveragePower_30s > &powValues[movingAveragePowerLength - 1u]) {
                pMovingAveragePower_30s = &powValues[0u];
            }
            if (pMovingAveragePower_60s > &powValues[movingAveragePowerLength - 1u]) {
                pMovingAveragePower_60s = &powValues[0u];
            }
            if (pMovingAveragePower_cfg > &powValues[movingAveragePowerLength - 1u]) {
                pMovingAveragePower_cfg = &powValues[0u];
            }
        }
    }

    if (newValues == 1u) {
        newValues = 0;

        DATA_WRITE_DATA(&movingAverage_tab);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
