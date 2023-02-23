/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    soa.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup APPLICATION
 * @prefix  SOA
 *
 * @brief   SOA module responsible for checking battery parameters against
 *          safety limits
 *
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

extern void SOA_CheckVoltages(DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellVoltages) {
    FAS_ASSERT(pMinimumMaximumCellVoltages != NULL_PTR);
    DIAG_RETURNTYPE_e retvalUndervoltMSL = DIAG_HANDLER_RETURN_ERR_OCCURRED;

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int16_t voltageMax_mV = pMinimumMaximumCellVoltages->maximumCellVoltage_mV[s];
        int16_t voltageMin_mV = pMinimumMaximumCellVoltages->minimumCellVoltage_mV[s];

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

        if (voltageMin_mV <= BC_VOLTAGE_MIN_MOL_mV) {
            /* Under voltage maximum operating limit violated */
            DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            if (voltageMin_mV <= BC_VOLTAGE_MIN_RSL_mV) {
                /* Under voltage recommended safety limit violated */
                DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                if (voltageMin_mV <= BC_VOLTAGE_MIN_MSL_mV) {
                    /* Under voltage maximum safety limit violated */
                    retvalUndervoltMSL =
                        DIAG_Handler(DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);

                    /* If under voltage flag is set and deep-discharge voltage is violated */
                    if ((retvalUndervoltMSL == DIAG_HANDLER_RETURN_ERR_OCCURRED) &&
                        (voltageMin_mV <= BC_VOLTAGE_DEEP_DISCHARGE_mV)) {
                        DIAG_Handler(DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                    }
                }
            }
        }
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

extern void SOA_CheckTemperatures(
    DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellTemperatures,
    DATA_BLOCK_PACK_VALUES_s *pCurrent) {
    FAS_ASSERT(pMinimumMaximumCellTemperatures != NULL_PTR);
    FAS_ASSERT(pCurrent != NULL_PTR);
    /* Iterate over each string and check temperatures */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        int32_t i_current            = pCurrent->stringCurrent_mA[s];
        int16_t temperatureMin_ddegC = pMinimumMaximumCellTemperatures->minimumTemperature_ddegC[s];
        int16_t temperatureMax_ddegC = pMinimumMaximumCellTemperatures->maximumTemperature_ddegC[s];

        /* Over temperature check */
        if (BMS_GetCurrentFlowDirection(i_current) == BMS_DISCHARGING) {
            /* Discharge */
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
        if (BMS_GetCurrentFlowDirection(i_current) == BMS_DISCHARGING) {
            /* Discharge */
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

extern void SOA_CheckCurrent(DATA_BLOCK_PACK_VALUES_s *pTablePackValues) {
    FAS_ASSERT(pTablePackValues != NULL_PTR);

    /* Iterate over each string and check current */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Only perform check if current value is valid */
        if (pTablePackValues->invalidStringCurrent[s] == 0u) {
            BMS_CURRENT_FLOW_STATE_e currentDirection =
                BMS_GetCurrentFlowDirection(pTablePackValues->stringCurrent_mA[s]);
            uint32_t absStringCurrent_mA = (uint32_t)abs(pTablePackValues->stringCurrent_mA[s]);
            /* Check various current limits depending on current direction */
            bool stringOvercurrent = SOA_IsStringCurrentLimitViolated(absStringCurrent_mA, currentDirection);
            bool cellOvercurrent   = SOA_IsCellCurrentLimitViolated(absStringCurrent_mA, currentDirection);
            if (currentDirection == BMS_CHARGING) {
                /* Check string current limit */
                if (stringOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
                /* Check battery cell limit */
                if (cellOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
            } else if (currentDirection == BMS_DISCHARGING) {
                /* Check string current limit */
                if (stringOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
                /* Check battery cell limit */
                if (cellOvercurrent == true) {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
                } else {
                    DIAG_Handler(DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                }
            } else {
                /* No current floating -> everything okay */
                DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                DIAG_Handler(DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                DIAG_Handler(DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
                DIAG_Handler(DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL, DIAG_EVENT_OK, DIAG_STRING, s);
            }

            /* Check if current is floating while contactors are open */
            if (SOA_IsCurrentOnOpenString(currentDirection, s) == false) {
                DIAG_Handler(DIAG_ID_CURRENT_ON_OPEN_STRING, DIAG_EVENT_OK, DIAG_STRING, s);
            } else {
                DIAG_Handler(DIAG_ID_CURRENT_ON_OPEN_STRING, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            }
        }
    }

    /* Check pack current */
    if (pTablePackValues->invalidPackCurrent == 0u) {
        BMS_CURRENT_FLOW_STATE_e currentDirection = BMS_GetCurrentFlowDirection(pTablePackValues->packCurrent_mA);
        uint32_t absPackCurrent_mA                = (uint32_t)abs(pTablePackValues->packCurrent_mA);
        bool packOvercurrent                      = SOA_IsPackCurrentLimitViolated(absPackCurrent_mA, currentDirection);

        if (currentDirection == BMS_CHARGING) {
            if (packOvercurrent == true) {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
            } else {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            }
        } else if (currentDirection == BMS_DISCHARGING) {
            if (packOvercurrent == true) {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
            } else {
                DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            }
        } else {
            /* No current floating -> everything okay */
            DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
            DIAG_Handler(DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        }
    }
}

extern void SOA_CheckSlaveTemperatures(void) { /* TODO: to be implemented */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
