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
 * @file    ${file_name}
 * @author  foxBMS Team
 * @date    ${date}
 * @updated ${updated}
 * @version ${version}
 * @ingroup BATTERY_CELL_CONFIGURATION
 * @prefix  BC
 *
 * @brief   Configuration of the battery cell (e.g., minimum and maximum cell
 *          voltage)
 * @details This files contains basic macros of the battery cell in order to
 *          derive needed inputs in other parts of the software. These macros
 *          are all depended on the hardware.
 *
 */

#ifndef FOXBMS__BATTERY_CELL_CFG_H_
#define FOXBMS__BATTERY_CELL_CFG_H_

/*========== Includes =======================================================*/

#include "battery_cell_cfg_types.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * @brief   Maximum temperature limit during discharge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC (${BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC})
#define BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC (${BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC})
#define BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC (${BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC})
/**@}*/

/**
 * @brief   Minimum temperature limit during discharge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC (${BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC})
#define BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC (${BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC})
#define BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC (${BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC})
/**@}*/

/**
 * @brief   Maximum temperature limit during charge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC (${BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC})
#define BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC (${BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC})
#define BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC (${BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC})
/**@}*/

/**
 * @brief   Minimum temperature limit during charge.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 */
/**@{*/
#define BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC (${BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC})
#define BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC (${BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC})
#define BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC (${BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC})
/**@}*/

/**
 * @brief   Maximum cell voltage limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mV
 */
/**@{*/
#define BC_VOLTAGE_MAX_MSL_mV (${BC_VOLTAGE_MAX_MSL_mV})
#define BC_VOLTAGE_MAX_RSL_mV (${BC_VOLTAGE_MAX_RSL_mV})
#define BC_VOLTAGE_MAX_MOL_mV (${BC_VOLTAGE_MAX_MOL_mV})
/**@}*/

/**
 * @brief   nominal cell voltage according to datasheet
 * @ptype   int
 * @unit    mV
 */
#define BC_VOLTAGE_NOMINAL_mV (${BC_VOLTAGE_NOMINAL_mV})

/**
 * @brief   Minimum cell voltage limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mV
 */
/**@{*/
#define BC_VOLTAGE_MIN_MSL_mV (${BC_VOLTAGE_MIN_MSL_mV})
#define BC_VOLTAGE_MIN_RSL_mV (${BC_VOLTAGE_MIN_RSL_mV})
#define BC_VOLTAGE_MIN_MOL_mV (${BC_VOLTAGE_MIN_MOL_mV})
/**@}*/

/**
 * @brief   Deep-discharge cell voltage limit.
 * @details If this voltage limit is violated, the cell is faulty. The BMS will
 *          not allow a closing of the contactors until this cell is replaced.
 *          A replacement of the cell is confirmed by resetting persistent
 *          flags in BMS state request CAN message.
 * @ptype   int
 * @unit    mV
 */
#define BC_VOLTAGE_DEEP_DISCHARGE_mV (BC_VOLTAGE_MIN_MSL_mV)

/**
 * @brief   Maximum discharge current limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mA
 */
/**@{*/
#define BC_CURRENT_MAX_DISCHARGE_MSL_mA (${BC_CURRENT_MAX_DISCHARGE_MSL_mA})
#define BC_CURRENT_MAX_DISCHARGE_RSL_mA (${BC_CURRENT_MAX_DISCHARGE_RSL_mA})
#define BC_CURRENT_MAX_DISCHARGE_MOL_mA (${BC_CURRENT_MAX_DISCHARGE_MOL_mA})
/**@}*/

/**
 * @brief   Maximum charge current limit.
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open. When recommended safety limit
 *          (RSL) or maximum operating limit (MOL) is violated, the respective
 *          flag will be set.
 * @ptype   int
 * @unit    mA
 */
/**@{*/
#define BC_CURRENT_MAX_CHARGE_MSL_mA (${BC_CURRENT_MAX_CHARGE_MSL_mA})
#define BC_CURRENT_MAX_CHARGE_RSL_mA (${BC_CURRENT_MAX_CHARGE_RSL_mA})
#define BC_CURRENT_MAX_CHARGE_MOL_mA (${BC_CURRENT_MAX_CHARGE_MOL_mA})
/**@}*/

/**
 * @brief   Cell capacity used for SOC calculation
 * @ptype   int
 * @unit    mAh
 */
#define BC_CAPACITY_mAh (${BC_CAPACITY_mAh})

/**
 * @brief   Cell energy
 * @ptype   float
 * @unit    Wh
 */
#define BC_ENERGY_Wh (${BC_ENERGY_Wh})

#if BC_VOLTAGE_MIN_MSL_mV < BC_VOLTAGE_DEEP_DISCHARGE_mV
#error "Configuration error! - Maximum safety limit for under voltage can't be lower than deep-discharge limit"
#endif

/*========== Extern Constant and Variable Declarations ======================*/
extern uint16_t bc_stateOfChargeLookupTableLength;   /*!< length of the SOC lookup table */
extern const BC_LUT_s bc_stateOfChargeLookupTable[]; /*!< SOC lookup table */

extern uint16_t bc_stateOfEnergyLookupTableLength;   /*!< length of the SOE lookup table */
extern const BC_LUT_s bc_stateOfEnergyLookupTable[]; /*!< SOE lookup table */

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BATTERY_CELL_CFG_H_ */
