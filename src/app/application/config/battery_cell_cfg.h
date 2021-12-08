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
 * @file    battery_cell_cfg.h
 * @author  foxBMS Team
 * @date    2017-03-14 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup BATTERY_CELL_CONFIGURATION
 * @prefix  BC
 *
 * @brief   Configuration of the battery cell (e.g., minimum and maximum cell
 *          voltage)
 *
 * @details This files contains basic macros of the battery cell in order to
 *          derive needed inputs in other parts of the software. These macros
 *          are all depended on the hardware.
 *
 */

#ifndef FOXBMS__BATTERY_CELL_CFG_H_
#define FOXBMS__BATTERY_CELL_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Maximum temperature limit during discharge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC (550)
#define BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC (500)
#define BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC (450)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Minimum temperature limit during discharge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC (-200)
#define BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC (-150)
#define BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC (-100)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Maximum temperature limit during charge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC (450)
#define BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC (400)
#define BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC (350)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Minimum temperature limit during discharge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC (-200)
#define BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC (-150)
#define BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC (-100)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Maximum cell voltage limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mV
 */
/**@{*/
#define BC_VOLTAGE_MAX_MSL_mV (2800)
#define BC_VOLTAGE_MAX_RSL_mV (2750)
#define BC_VOLTAGE_MAX_MOL_mV (2720)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   nominal cell voltage according to data sheet
 * @ptype   int
 * @unit    mV
 */
#define BC_VOLTAGE_NOMINAL_mV (2500)

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Minimum cell voltage limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mV
 */
/**@{*/
#define BC_VOLTAGE_MIN_MSL_mV (1700)
#define BC_VOLTAGE_MIN_RSL_mV (1750)
#define BC_VOLTAGE_MIN_MOL_mV (1780)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Deep-discharge cell voltage limit.
 * @details If this voltage limit is violated, the cell is faulty. The BMS will
 *          not allow a closing of the contactors until this cell is replaced.
 *          A replacement of the cell is confirmed by sending the respective
 *          CAN debug message
 * @ptype   int
 * @unit    mV
 */
#define BC_VOLTAGE_DEEP_DISCHARGE_mV (BC_VOLTAGE_MIN_MSL_mV)

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Maximum discharge current limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mA
 */
/**@{*/
#define BC_CURRENT_MAX_DISCHARGE_MSL_mA (180000u)
#define BC_CURRENT_MAX_DISCHARGE_RSL_mA (175000u)
#define BC_CURRENT_MAX_DISCHARGE_MOL_mA (170000u)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * @brief   Maximum charge current limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mA
 */
/**@{*/
#define BC_CURRENT_MAX_CHARGE_MSL_mA (180000u)
#define BC_CURRENT_MAX_CHARGE_RSL_mA (175000u)
#define BC_CURRENT_MAX_CHARGE_MOL_mA (170000u)
/**@}*/

/**
 * @brief   Cell capacity used for SOC calculation
 * @ptype   int
 * @unit    mAh
 */
#define BC_CAPACITY_mAh (3500u)

#define BC_ENERGY_Wh (10.0f)

#if BC_VOLTAGE_MIN_MSL_mV < BC_VOLTAGE_DEEP_DISCHARGE_mV
#error "Configuration error! - Maximum safety limit for under voltage can't be lower than deep-discharge limit"
#endif

/** structure for lookup table */
typedef struct BC_LUT {
    const int16_t voltage_mV; /*!< cell voltage in mV */
    const float value;        /*!< corresponding value, can be SOC/SOE in % or capacity/energy */
} BC_LUT_s;

/*========== Extern Constant and Variable Declarations ======================*/
extern uint16_t bc_stateOfChargeLookupTableLength;   /*!< length of the SOC lookup table */
extern const BC_LUT_s bc_stateOfChargeLookupTable[]; /*!< SOC lookup table */

extern uint16_t bc_stateOfEnergyLookupTableLength;   /*!< length of the SOE lookup table */
extern const BC_LUT_s bc_stateOfEnergyLookupTable[]; /*!< SOE lookup table */

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__BATTERY_CELL_CFG_H_ */
