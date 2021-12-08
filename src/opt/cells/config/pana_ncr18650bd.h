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
 * @file    pana_ncr18650bd.h
 * @author  foxBMS Team
 * @date    2018-10-02 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup BATTERY_CELL_CONF
 * @prefix  BC
 *
 * @brief   Configuration of the battery cell (e.g., minimum and maximum cell voltage)
 *
 * This files contains basic macros of the battery cell in order to derive needed inputs
 * in other parts of the software. These macros are all depended on the hardware.
 *
 */

#ifndef FOXBMS__PANA_NCR18650BD_H_
#define FOXBMS__PANA_NCR18650BD_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/**
 * @ingroup CONFIG_BATTERY_CELL
 * Maximum temperature limit during discharge.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 * @{
 */
#define BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC (600)
#define BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC (550)
#define BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC (500)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * Minimum temperature limit during discharge.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 * @{
 */
#define BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC (-200)
#define BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC (-180)
#define BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC (-150)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * Maximum temperature limit during charge.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 * @{
 */
#define BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC (450)
#define BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC (400)
#define BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC (350)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * Minimum temperature limit during discharge.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    deci &deg;C
 * @{
 */
#define BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC (100)
#define BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC (120)
#define BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC (150)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * Maximum cell voltage limit.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    mV
 * @{
 */
#define BC_VOLTAGE_MAX_MSL_mV (4200u)
#define BC_VOLTAGE_MAX_RSL_mV (4150u)
#define BC_VOLTAGE_MAX_MOL_mV (4100u)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * nominal cell voltage according to data sheet
 * @ptype   int
 * @unit    mV
 */
#define BC_VOLTAGE_NOMINAL_mV (3600u)
/**
 * @ingroup CONFIG_BATTERY_CELL
 * Minimum cell voltage limit.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    mV
 * @{
 */
#define BC_VOLTAGE_MIN_MSL_mV (2500u)
#define BC_VOLTAGE_MIN_RSL_mV (2550u)
#define BC_VOLTAGE_MIN_MOL_mV (2600u)
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * Deep-discharge cell voltage limit.
 * If this voltage limit is violated, the cell is faulty. The BMS won't allow
 * a closing of the contactors until this cell is replaced. a replacement of
 * the cell is confirmed by sending the respective CAN debug message
 * @ptype   int
 * @unit    mV
 */
#define BC_VOLTAGE_DEEP_DISCHARGE_mV (BC_VOLTAGE_MIN_MSL_mV - 100u)

/**
 * @ingroup CONFIG_BATTERY_CELL
 * Maximum discharge current limit.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    mA
 * @{
 */
#define BC_CURRENT_MAX_DISCHARGE_MSL_mA (550000u) /*!< 6 modules (7s20p) in series, 3 strings in parallel */
#define BC_CURRENT_MAX_DISCHARGE_RSL_mA (525000u) /*!< current reduced by 50 A because of imbalances between */
#define BC_CURRENT_MAX_DISCHARGE_MOL_mA (500000u) /*!< the different strings */
/**@}*/

/**
 * @ingroup CONFIG_BATTERY_CELL
 * Maximum charge current limit.
 * When maximum safety limit (MSL) is violated, error state is requested and
 * contactors will open. When recommended safety limit (RSL) or maximum
 * operating limit (MOL) is violated, the respective flag will be set.
 * @ptype   int
 * @unit    mA
 * @{
 */
#define BC_CURRENT_MAX_CHARGE_MSL_mA (50000u) /*!< 6 modules (7s20p) in series, 3 strings in parallel */
#define BC_CURRENT_MAX_CHARGE_RSL_mA (47500u) /*!< current reduced by 4,54 A because of imbalances between */
#define BC_CURRENT_MAX_CHARGE_MOL_mA (45000u) /*!< the different strings */
/**@}*/

/**
 * @brief the cell capacity used for SOC calculation, in this case Ah counting
 * @ptype   int
 * @unit    mAh
 */
#define BC_CAPACITY_mAh (3180u)

#if BC_VOLTAGE_MIN_MSL_mV < BC_VOLTAGE_DEEP_DISCHARGE_mV
#error "Configuration error! - Maximum safety limit for under voltage can't be lower than deep-discharge limit"
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__PANA_NCR18650BD_H_ */
