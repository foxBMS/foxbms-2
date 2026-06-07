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
 * @file    soa.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  SOA
 *
 * @brief   SOA module responsible for checking battery parameters against
 *          safety limits
 * @requirements REQ-001, REQ-002, REQ-003, REQ-004, REQ-005, REQ-006, REQ-007,
 *              REQ-008, REQ-009, REQ-010, REQ-011, REQ-012, REQ-013, REQ-014,
 *              REQ-015, REQ-016, REQ-017
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "soa.h"

#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"

#include "bms.h"
#include "diag.h"
#include "foxmath.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/**
 * @brief   checks the abidance by the safe operating area for cell voltages
 * @req     REQ-001, REQ-002, REQ-003, REQ-004, REQ-016
 * @param[in]   pMinimumMaximumCellVoltages  pointer to database entry with
 *                                           minimum and maximum cell voltages
 */
extern void SOA_CheckVoltages(DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellVoltages) {
    /* REQ-016: 输入参数空指针保护 */
    FAS_ASSERT(pMinimumMaximumCellVoltages != NULL_PTR);
    DIAG_RETURNTYPE_e retvalUndervoltageMSL = DIAG_HANDLER_RETURN_ERR_OCCURRED;

    /* REQ-001: 遍历所有电池串执行电压SOA检查 */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int16_t voltageMax_mV = pMinimumMaximumCellVoltages->maximumCellVoltage_mV[s];
        int16_t voltageMin_mV = pMinimumMaximumCellVoltages->minimumCellVoltage_mV[s];

        /* REQ-002: 过压三级安全阈值检查 (MOL→RSL→MSL) */
        if (voltageMax_mV >= BC_VOLTAGE_MAX_MOL_mV) {
            /* Over voltage maximum operating limit violated */
            DIAG_Handler(DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            if (voltageMax_mV >= BC_VOLTAGE_MAX_RSL_mV) {
                /* Over voltage recommended safety limit violated */
                DIAG_Handler(DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                if (voltageMax_mV >= BC_VOLTAGE_MAX_MSL_mV) {
                    /* Over voltage maximum safety limit violated */
                    DIAG_Handler(DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                }
            }
        }
        /* REQ-002: 过压故障恢复检测 (MSL→RSL→MOL逐级清除) */
        if (voltageMax_mV < BC_VOLTAGE_MAX_MSL_mV) {
            /* over voltage maximum safety limit NOT violated */
            DIAG_Handler(DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
            if (voltageMax_mV < BC_VOLTAGE_MAX_RSL_mV) {
                /* over voltage recommended safety limit NOT violated */
                DIAG_Handler(DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s);
                if (voltageMax_mV < BC_VOLTAGE_MAX_MOL_mV) {
                    /* over voltage maximum operating limit NOT violated */
                    DIAG_Handler(DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
            }
        }

        /* REQ-003: 欠压三级安全阈值检查 (MOL→RSL→MSL) */
        if (voltageMin_mV <= BC_VOLTAGE_MIN_MOL_mV) {
            /* Under voltage maximum operating limit violated */
            DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            if (voltageMin_mV <= BC_VOLTAGE_MIN_RSL_mV) {
                /* Under voltage recommended safety limit violated */
                DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                if (voltageMin_mV <= BC_VOLTAGE_MIN_MSL_mV) {
                    /* Under voltage maximum safety limit violated */
                    retvalUndervoltageMSL =
                        DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);

                    /* REQ-004: 深度放电检测 — 欠压MSL确认 + 低于深度放电阈值 */
                    /* If under voltage flag is set and deep-discharge voltage is violated */
                    if ((retvalUndervoltageMSL == DIAG_HANDLER_RETURN_ERR_OCCURRED) &&
                        (voltageMin_mV <= BC_VOLTAGE_DEEP_DISCHARGE_mV)) {
                        DIAG_Handler(DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    }
                }
            }
        }
        /* REQ-003: 欠压故障恢复检测 (MSL→RSL→MOL逐级清除) */
        if (voltageMin_mV > BC_VOLTAGE_MIN_MSL_mV) {
            /* under voltage maximum safety limit NOT violated */
            DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
            if (voltageMin_mV > BC_VOLTAGE_MIN_RSL_mV) {
                /* under voltage recommended safety limit NOT violated */
                DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s);
                if (voltageMin_mV > BC_VOLTAGE_MIN_MOL_mV) {
                    /* under voltage maximum operating limit NOT violated */
                    DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
            }
        }
    }
}

/**
 * @brief   checks the abidance by the safe operating area for cell temperatures
 * @req     REQ-005, REQ-006, REQ-007, REQ-008, REQ-016
 * @param[in]   pMinimumMaximumCellTemperatures  pointer to database entry with
 *                                               min/max cell temperatures
 * @param[in]   pCurrent                         pointer to pack value database entry
 */
extern void SOA_CheckTemperatures(
    DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellTemperatures,
    DATA_BLOCK_PACK_VALUES_s *pCurrent) {
    /* REQ-016: 输入参数空指针保护 */
    FAS_ASSERT(pMinimumMaximumCellTemperatures != NULL_PTR);
    FAS_ASSERT(pCurrent != NULL_PTR);
    /* REQ-005: 遍历所有电池串执行温度SOA检查 */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int32_t i_current            = pCurrent->stringCurrent_mA[s];
        int16_t temperatureMin_ddegC = pMinimumMaximumCellTemperatures->minimumTemperature_ddegC[s];
        int16_t temperatureMax_ddegC = pMinimumMaximumCellTemperatures->maximumTemperature_ddegC[s];

        /* REQ-006: 根据电流方向选择充/放电温度阈值组 */
        if (BMS_GetCurrentFlowDirection(i_current) == BMS_DISCHARGING) {
            /* Discharge */
            /* REQ-007: 过温三级安全阈值检查 — 放电方向 (MOL→RSL→MSL) */
            if (temperatureMax_ddegC >= BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC) {
                /* Over temperature maximum operating limit violated*/
                DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                if (temperatureMax_ddegC >= BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC) {
                    /* Over temperature recommended safety limit violated*/
                    DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    if (temperatureMax_ddegC >= BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC) {
                        /* Over temperature maximum safety limit violated */
                        DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    }
                }
            }
            /* REQ-007: 过温故障恢复检测 — 放电方向 (MSL→RSL→MOL逐级清除) */
            if (temperatureMax_ddegC < BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC) {
                /* over temperature maximum safety limit NOT violated */
                DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                if (temperatureMax_ddegC < BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC) {
                    /* over temperature recommended safety limit NOT violated */
                    DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s);
                    if (temperatureMax_ddegC < BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC) {
                        /* over temperature maximum operating limit NOT violated */
                        DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s);
                    }
                }
            }
        } else {
            /* Charge */
            /* REQ-007: 过温三级安全阈值检查 — 充电方向 (MOL→RSL→MSL) */
            if (temperatureMax_ddegC >= BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC) {
                /* Over temperature maximum operating limit violated */
                DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                if (temperatureMax_ddegC >= BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC) {
                    /* Over temperature recommended safety limit violated */
                    DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    /* Over temperature maximum safety limit violated */
                    if (temperatureMax_ddegC >= BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC) {
                        DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    }
                }
            }
            /* REQ-007: 过温故障恢复检测 — 充电方向 (MSL→RSL→MOL逐级清除) */
            if (temperatureMax_ddegC < BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC) {
                /* over temperature maximum safety limit NOT violated */
                DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                if (temperatureMax_ddegC < BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC) {
                    /* over temperature recommended safety limit NOT violated */
                    DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s);
                    if (temperatureMax_ddegC < BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC) {
                        /* over temperature maximum operating limit NOT violated*/
                        DIAG_Handler(DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s);
                    }
                }
            }
        }

        /* Under temperature check */
        /* REQ-006: 根据电流方向选择充/放电温度阈值组 */
        if (BMS_GetCurrentFlowDirection(i_current) == BMS_DISCHARGING) {
            /* Discharge */
            /* REQ-008: 欠温三级安全阈值检查 — 放电方向 (MOL→RSL→MSL) */
            if (temperatureMin_ddegC <= BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC) {
                /* Under temperature maximum operating limit violated */
                DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                if (temperatureMin_ddegC <= BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC) {
                    /* Under temperature recommended safety limit violated*/
                    DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    if (temperatureMin_ddegC <= BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC) {
                        /* Under temperature maximum safety limit violated */
                        DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    }
                }
            }
            /* REQ-008: 欠温故障恢复检测 — 放电方向 (MSL→RSL→MOL逐级清除) */
            if (temperatureMin_ddegC > BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC) {
                /* under temperature maximum safety limit NOT violated */
                DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                if (temperatureMin_ddegC > BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC) {
                    /* under temperature recommended safety limit NOT violated */
                    DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s);
                    if (temperatureMin_ddegC > BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC) {
                        /* under temperature maximum operating limit NOT violated*/
                        DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s);
                    }
                }
            }
        } else {
            /* Charge */
            /* REQ-008: 欠温三级安全阈值检查 — 充电方向 (MOL→RSL→MSL) */
            if (temperatureMin_ddegC <= BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC) {
                /* Under temperature maximum operating limit violated */
                DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                if (temperatureMin_ddegC <= BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC) {
                    /* Under temperature recommended safety limit violated */
                    DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    if (temperatureMin_ddegC <= BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC) {
                        /* Under temperature maximum safety limit violated */
                        DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    }
                }
            }
            /* REQ-008: 欠温故障恢复检测 — 充电方向 (MSL→RSL→MOL逐级清除) */
            if (temperatureMin_ddegC > BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC) {
                /* under temperature maximum safety limit NOT violated */
                DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                if (temperatureMin_ddegC > BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC) {
                    /* under temperature recommended safety limit NOT violated */
                    DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_RSL, DIAG_EVENT_OK, DIAG_STRING, s);
                    if (temperatureMin_ddegC > BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC) {
                        /* under temperature maximum operating limit NOT violated*/
                        DIAG_Handler(DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL, DIAG_EVENT_OK, DIAG_STRING, s);
                    }
                }
            }
        }
    }
}

/**
 * @brief   checks the abidance by the safe operating area for currents
 * @req     REQ-009, REQ-010, REQ-011, REQ-012, REQ-013, REQ-014, REQ-015, REQ-016
 * @param[in]   pTablePackValues   pointer to pack values database entry
 */
extern void SOA_CheckCurrent(DATA_BLOCK_PACK_VALUES_s *pTablePackValues) {
    /* REQ-016: 输入参数空指针保护 */
    FAS_ASSERT(pTablePackValues != NULL_PTR);

    /* REQ-009: 遍历所有电池串执行电流SOA检查 */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* REQ-010: 仅对有效电流值执行检查，跳过无效值 */
        if (pTablePackValues->invalidStringCurrent[s] == 0u) {
            BMS_CURRENT_FLOW_STATE_e currentDirection =
                BMS_GetCurrentFlowDirection(pTablePackValues->stringCurrent_mA[s]);
            uint32_t absStringCurrent_mA = (uint32_t)abs(pTablePackValues->stringCurrent_mA[s]);
            /* REQ-011: 电池串过流检测 */
            /* REQ-012: 电芯过流检测 */
            bool stringOvercurrent = SOA_IsStringCurrentLimitViolated(absStringCurrent_mA, currentDirection);
            bool cellOvercurrent   = SOA_IsCellCurrentLimitViolated(absStringCurrent_mA, currentDirection);
            if (currentDirection == BMS_CHARGING) {
                /* REQ-011: 电池串充电过流诊断上报 */
                if (stringOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
                /* REQ-012: 电芯充电过流诊断上报 */
                if (cellOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
            } else if (currentDirection == BMS_DISCHARGING) {
                /* REQ-011: 电池串放电过流诊断上报 */
                if (stringOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
                /* REQ-012: 电芯放电过流诊断上报 */
                if (cellOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
            } else {
                /* REQ-015: 电流浮空状态 — 所有过流诊断上报为OK */
                DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                DIAG_Handler(DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                DIAG_Handler(DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
            }

            /* REQ-014: 开路异常电流检测 */
            if (SOA_IsCurrentOnOpenString(currentDirection, s) == false) {
                DIAG_Handler(DIAG_ID_CURRENT_ON_OPEN_STRING, DIAG_EVENT_OK, DIAG_STRING, s);
            } else {
                DIAG_Handler(DIAG_ID_CURRENT_ON_OPEN_STRING, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            }
        }
    }

    /* REQ-013: 电池包总电流过流检测 */
    /* REQ-010: 仅对有效电池包电流值执行检查 */
    if (pTablePackValues->invalidPackCurrent == 0u) {
        BMS_CURRENT_FLOW_STATE_e currentDirection = BMS_GetCurrentFlowDirection(pTablePackValues->packCurrent_mA);
        uint32_t absPackCurrent_mA                = (uint32_t)abs(pTablePackValues->packCurrent_mA);
        bool packOvercurrent                      = SOA_IsPackCurrentLimitViolated(absPackCurrent_mA, currentDirection);

        if (currentDirection == BMS_CHARGING) {
            /* REQ-013: 电池包充电过流诊断上报 */
            if (packOvercurrent == true) {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
            } else {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            }
        } else if (currentDirection == BMS_DISCHARGING) {
            /* REQ-013: 电池包放电过流诊断上报 */
            if (packOvercurrent == true) {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
            } else {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            }
        } else {
            /* REQ-015: 电流浮空状态 — 电池包过流诊断上报为OK */
            DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        }
    }
}

/**
 * @brief   FOR FUTURE COMPATIBILITY; DUMMY FUNCTION; DO NOT USE
 * @req     REQ-017
 */
extern void SOA_CheckSlaveTemperatures(void) { /* TODO: to be implemented */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
