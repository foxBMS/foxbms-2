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
 * @file    plausibility.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  PL
 *
 * @brief   Plausibility checks for cell voltage and cell temperatures
 * @details TODO
 * @requirements REQ-001, REQ-002, REQ-003, REQ-004, REQ-005, REQ-006, REQ-007, REQ-008, REQ-009, REQ-010, REQ-011
 *
 */

/*========== Includes =======================================================*/
#include "plausibility.h"

#include "battery_system_cfg.h"

#include "diag.h"
#include "foxmath.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e PL_CheckStringVoltage(int32_t voltageAfe_mV, int32_t voltageCurrentSensor_mV) {
    STD_RETURN_TYPE_e result = STD_NOT_OK;

    /* REQ-001: 计算 AFE 与电流传感器总压偏差，与容差阈值比较 */
    int32_t diff_mV = voltageAfe_mV - voltageCurrentSensor_mV;

    if (abs(diff_mV) < PL_STRING_VOLTAGE_TOLERANCE_mV) {
        result = STD_OK;
    }
    return result;
}

extern STD_RETURN_TYPE_e PL_CheckCellVoltage(
    int16_t baseCellVoltage,
    int16_t redundancy0CellVoltage,
    int16_t *pCellVoltage) {
    /* AXIVION Routine Generic-MissingParameterAssert: baseCellVoltage: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: redundancy0CellVoltage: parameter accepts whole range */
    /* REQ-008: 输出指针非空断言 */
    FAS_ASSERT(pCellVoltage != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;

    /* REQ-002: 冗余单体电压偏差与容差阈值比较 */
    if (abs(baseCellVoltage - redundancy0CellVoltage) > PL_CELL_VOLTAGE_TOLERANCE_mV) {
        retval = STD_NOT_OK;
    }
    /* REQ-002: 取冗余测量值的算术平均值作为输出 */
    *pCellVoltage = (baseCellVoltage + redundancy0CellVoltage) / 2;
    return retval;
}

extern STD_RETURN_TYPE_e PL_CheckCellTemperature(
    int16_t baseCellTemperature,
    int16_t redundancy0CellTemperature,
    int16_t *pCellTemperature) {
    /* AXIVION Routine Generic-MissingParameterAssert: baseCellTemperature: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: redundancy0CellTemperature: parameter accepts whole range */
    /* REQ-008: 输出指针非空断言 */
    FAS_ASSERT(pCellTemperature != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    /* REQ-003: 冗余单体温度偏差与容差阈值比较 */
    if (abs(baseCellTemperature - redundancy0CellTemperature) > PL_CELL_TEMPERATURE_TOLERANCE_dK) {
        retval = STD_NOT_OK;
    }
    /* REQ-003: 取冗余测量值的算术平均值作为输出 */
    *pCellTemperature = (baseCellTemperature + redundancy0CellTemperature) / 2;
    return retval;
}

extern STD_RETURN_TYPE_e PL_CheckVoltageSpread(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltages,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    /* REQ-009: 数据库指针非空断言 */
    FAS_ASSERT(pCellVoltages != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    /* REQ-004: 遍历所有电芯串/模组/电芯块 */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e plausibilityIssueDetected = STD_OK;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                /* REQ-010: 仅对有效电压执行分布检查 */
                if (pCellVoltages->invalidCellVoltage[s][m][cb] == false) {
                    /* REQ-004: 电压与平均值的偏差超过容差阈值则标记无效 */
                    if (abs(pCellVoltages->cellVoltage_mV[s][m][cb] - pMinMaxAverageValues->averageCellVoltage_mV[s]) >
                        PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV) {
                        /* Voltage difference too large */
                        plausibilityIssueDetected = STD_NOT_OK;
                        retval                    = STD_NOT_OK;
                        /* REQ-004: 将超限电芯标记为无效 */
                        pCellVoltages->invalidCellVoltage[s][m][cb] = true;
                    }
                }
            }
        }
        /* REQ-006: 按电芯串上报电压分布异常诊断事件 */
        DIAG_CheckEvent(plausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD, DIAG_STRING, s);
    }
    return retval;
}

extern STD_RETURN_TYPE_e PL_CheckTemperatureSpread(
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatures,
    DATA_BLOCK_MIN_MAX_s *pMinMaxAverageValues) {
    /* REQ-009: 数据库指针非空断言 */
    FAS_ASSERT(pCellTemperatures != NULL_PTR);
    FAS_ASSERT(pMinMaxAverageValues != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    /* REQ-005: 遍历所有电芯串/模组/温度传感器 */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        STD_RETURN_TYPE_e plausibilityIssueDetected = STD_OK;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                /* REQ-011: 仅对有效温度执行分布检查 */
                if (pCellTemperatures->invalidCellTemperature[s][m][ts] == false) {
                    /* REQ-005: 温度与平均值的偏差超过容差阈值则标记无效 */
                    if (abs(pCellTemperatures->cellTemperature_ddegC[s][m][ts] -
                            (int16_t)pMinMaxAverageValues->averageTemperature_ddegC[s]) >
                        PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK) {
                        /* temperature difference too large */
                        plausibilityIssueDetected = STD_NOT_OK;
                        retval                    = STD_NOT_OK;
                        /* REQ-005: 将超限温度传感器标记为无效 */
                        pCellTemperatures->invalidCellTemperature[s][m][ts] = true;
                    } else {
                        /* REQ-011: 通过检查的温度传感器递增有效计数 */
                        pCellTemperatures->nrValidTemperatures[s]++;
                    }
                }
            }
        }
        /* REQ-007: 按电芯串上报温度分布异常诊断事件 */
        DIAG_CheckEvent(plausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD, DIAG_STRING, s);
    }
    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
