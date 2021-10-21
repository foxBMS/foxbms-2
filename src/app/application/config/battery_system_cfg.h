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
 * @file    battery_system_cfg.h
 * @author  foxBMS Team
 * @date    2019-12-10 (date of creation)
 * @updated 2021-06-09 (date of last update)
 * @ingroup BATTERY_SYSTEM_CONFIGURATION
 * @prefix  BS
 *
 * @brief   Configuration of the battery system (e.g., number of battery
 *          modules, battery cells, temperature sensors)
 *
 * @details This files contains basic macros of the battery system in order to
 *          derive needed inputs in other parts of the software. These macros
 *          are all depended on the hardware.
 *
 */

#ifndef FOXBMS__BATTERY_SYSTEM_CFG_H_
#define FOXBMS__BATTERY_SYSTEM_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/** Symbolic identifiers for strings with precharge */
typedef enum BS_STRING_PRECHARGE_PRESENT {
    BS_STRING_WITH_PRECHARGE,
    BS_STRING_WITHOUT_PRECHARGE,
} BS_STRING_PRECHARGE_PRESENT_e;

/**
 * Symbolic identifiers for strings.
 * Currently unused.
 * Added for future compatibility.
 */
typedef enum BS_STRING_ID {
    BS_STRING0    = 0u,
    BS_STRING1    = 1u,
    BS_STRING2    = 2u,
    BS_STRING_MAX = 3u,
} BS_STRING_ID_e;

/** Define if discharge current are positive negative, default is positive */
#define POSITIVE_DISCHARGE_CURRENT (true)

/** Number of strings in system */
#define BS_NR_OF_STRINGS (3u)

/* safety check: due to implementation BS_NR_OF_STRINGS may not be larger than REPEAT_MAXIMUM_REPETITIONS */
#if (BS_NR_OF_STRINGS > REPEAT_MAXIMUM_REPETITIONS)
#error "Too large number of strings, please check implementation of REPEAT_U()."
#endif

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   number of modules in battery pack
 * @ptype   uint
 */
#define BS_NR_OF_MODULES (1u)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   number of battery cells per battery module (parallel cells are
 *          counted as one)
 * @ptype   uint
 */
#define BS_NR_OF_CELLS_PER_MODULE (18u)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   number of battery cells in a parallel cell connection per battery
 *          module
 * @ptype   uint
 */
#define BS_NR_OF_PARALLEL_CELLS_PER_MODULE (1u)

#if BS_NR_OF_CELLS_PER_MODULE <= 12u
#define BS_MAX_SUPPORTED_CELLS (12u)
#elif BS_NR_OF_CELLS_PER_MODULE <= 15u
#define BS_MAX_SUPPORTED_CELLS (15u)
#elif BS_NR_OF_CELLS_PER_MODULE <= 16u
#define BS_MAX_SUPPORTED_CELLS (16u)
#elif BS_NR_OF_CELLS_PER_MODULE <= 18u
#define BS_MAX_SUPPORTED_CELLS (18u)
#elif BS_NR_OF_CELLS_PER_MODULE == 36u
#define BS_MAX_SUPPORTED_CELLS (36u)
#else
#error "Unsupported number of cells per module, higher than 18 and not 36"
#endif

/**
 * @def     BS_MAX_SUPPORTED_CELLS
 * @brief   Defines the maximal number of supported cells per module
 */

/** Value of the balancing resistors on the slave-board */
#define BS_BALANCING_RESISTANCE_ohm (100.0)

/**
 * @def     BS_NR_OF_GPIOS_PER_MODULE
 * @brief   Number of GPIOs on the LTC IC
 * @details - 5 for 12 cell version
 *          - 9 for 18 cell version
 */
#if BS_MAX_SUPPORTED_CELLS == 12u
#define BS_NR_OF_GPIOS_PER_MODULE (5u)
#else
#define BS_NR_OF_GPIOS_PER_MODULE (9u)
#endif

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   number of temperature sensors per battery module
 * @ptype   int
 */
#define BS_NR_OF_TEMP_SENSORS_PER_MODULE (8u)

/** number of (not parallel) battery cells in the system */
#define BS_NR_OF_BAT_CELLS (BS_NR_OF_MODULES * BS_NR_OF_CELLS_PER_MODULE)
/** number of temperature sensors in a string */
#define BS_NR_OF_TEMP_SENSORS_PER_STRING (BS_NR_OF_MODULES * BS_NR_OF_TEMP_SENSORS_PER_MODULE)
/** number of temperature sensors in the battery system */
#define BS_NR_OF_TEMP_SENSORS (BS_NR_OF_TEMP_SENSORS_PER_STRING * BS_NR_OF_STRINGS)

/** number of temperature sensors on each ADC0 channel of the slave-board */
#define BS_NR_OF_TEMP_SENSORS_ON_SADC0 (3u)
/** number of temperature sensors on each ADC1 channel of the slave-board */
#define BS_NR_OF_TEMP_SENSORS_ON_SADC1 (3u)

/**
 * @details - If set to false, foxBMS does not check for the presence of a
 *            current sensor.
 *          - If set to true, foxBMS checks for the presence of a current
 *            sensor. If sensor stops responding during runtime, an error is
 *            raised.
 */
#define CURRENT_SENSOR_PRESENT (false)

#if CURRENT_SENSOR_PRESENT == true
/**
 * defines if the Isabellenhuette current sensor is used in cyclic or triggered mode
 */
#define CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
/* #define CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED */

/** Delay in ms after which it is considered the current measurement is not responding anymore. */
#define BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_MS (200u)

/** Delay in ms after which it is considered the coulomb counting is not responding anymore. */
#define BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_MS (2000u)

/** Delay in ms after which it is considered the energy counting is not responding anymore. */
#define BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_MS (2000u)

#endif /* CURRENT_SENSOR_PRESENT == true */

/**
 * @brief   Maximum string current limit in mA that is used in the SOA module
 *          to check for string overcurrent
* @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open.
 */
#define BS_MAXIMUM_STRING_CURRENT_mA (10000u)

/**
 * @brief   Maximum pack current limit in mA that is used in the SOA module
 *          to check for pack overcurrent
 * @details When maximum safety limit (MSL) is violated, error state is
 *          requested and contactors will open.
 */
#define BS_MAXIMUM_PACK_CURRENT_mA (10000u * BS_NR_OF_STRINGS)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   Define if interlock feedback should be discarded or not
 * @details True: interlock feedback will be discarded
 *          False: interlock feedback will evaluated
 */
#define BS_IGNORE_INTERLOCK_FEEDBACK (false)

/**
 * @details - If set to false, foxBMS does not check CAN timing.
 *          - If set to true, foxBMS checks CAN timing. A valid request must
 *            come every 100ms, within the 95-150ms window.
 */
#define CHECK_CAN_TIMING (true)

/**
 * @details - If set to true, balancing is deactivated completely.
 *          - If set to false, foxBMS checks when balancing must be done and
 *            activates it accordingly.
 */
#define BALANCING_DEFAULT_INACTIVE (true)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   Checking if current is in SOF limits of cells.
 * @details If set to true the current is checked against the SOF limits.
 *          If set to false the current is checked against the constant values
 *          for charging and discharging:
 *          - #BC_CURRENT_MAX_DISCHARGE_MSL_mA
 *          - #BC_CURRENT_MAX_DISCHARGE_RSL_mA
 *          - #BC_CURRENT_MAX_DISCHARGE_MOL_mA
 *
 *          - #BC_CURRENT_MAX_CHARGE_MSL_mA
 *          - #BC_CURRENT_MAX_CHARGE_RSL_mA
 *          - #BC_CURRENT_MAX_CHARGE_MOL_mA
 * @ptype   bool
 */
#define BMS_CHECK_SOF_CURRENT_LIMITS (true)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   Defines behaviour if an insulation error is detected
 * @details - If set to true: contactors will be opened
 *          - If set to false: contactors will NOT be opened
 */
#define BMS_OPEN_CONTACTORS_ON_INSULATION_ERROR (false)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   number of high voltage inputs measured by current sensors (like
 *          IVT-MOD)
 * @ptype   int
 */
#define BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR (3u)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   number of voltages measured by MCU internal ADC
 * @ptype   int
 */
#define BS_NR_OF_VOLTAGES_FROM_MCU_ADC (2)

/** Number of contactors in addition to string contactors (e.g., PRECHARGE).*/
#define BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS (1U)

/** Number of contactors. One per string + main and precharge */
#define BS_NR_OF_CONTACTORS ((2U * BS_NR_OF_STRINGS) + BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS)

/**
 * @ingroup CONFIG_BATTERYSYSTEM
 * @brief   separation of charge and discharge power line
 * @ptype   select(2)
 */
#define BS_SEPARATE_POWER_PATHS (1)

#if (BS_NR_OF_CONTACTORS > 3) && (BS_SEPARATE_POWER_PATHS == 0)
#error "Configuration mismatch: Can't use only one power path with more than 3 contactors"
#endif /*  */

#if (BS_NR_OF_CONTACTORS < 4) && (BS_SEPARATE_POWER_PATHS == 1)
#error "Configuration mismatch: Can't use separate power path with less than 4 contactors"
#endif /*  */

/**
 * @brief   current threshold for determing rest state of battery. If absolute
 *          current is below this limit value the battery is resting.
 */
#define BS_REST_CURRENT_mA (200)

/**
 * @brief   Wait time in 10ms before battery system is at rest. Balancing for
 *          example only starts if battery system is at rest.
 */
#define BS_RELAXATION_PERIOD_10ms (60000u)

/**
 * @brief   current sensor threshold for 0 current in mA as the sensor has a
 *          jitter.
 */
#define BS_CS_THRESHOLD_NO_CURRENT_mA (200u)

/**
 * @brief   Maximum voltage drop over fuse.
 * @details If the measured voltage difference between battery voltage
 *          voltage after fuse is larger than this value. It can be concluded,
 *          that the fuse has tripped. The voltage difference can be estimated
 *          by the maximum current and the resistance.
 *          For a Cooper Bussmann 1000A fuse the voltage drop can be estimated
 *          to: I_max =  1000A and P_loss = 206W
 *          -> voltage drop at 1000A roughly 206mV
 *          -> select 500mV because of measurement inaccuracies
 */
#define BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV (500)

/**
 * @brief   TODO
 * @details Set to true if fuse in NORMAL path should be checked. This can only
 *          be done if one dedicated HV measurement is used to monitor the
 *          voltage directly after the fuse. Then a voltage difference between
 *          V_bat and V_fuse indicates a tripped fuse.
 *
 *            V_bat  +------+     V_fuse       Precharge/Main+ contactor
 *          -----+---| FUSE |-----+------------/   -----------------
 *                   +------+
 */
#define BS_CHECK_FUSE_PLACED_IN_NORMAL_PATH (true)

/**
 * @brief   TODO
 * @details Set to true if fuse in CHARGE path should be checked. This can only
 *          be done if one dedicated HV measurement is used to monitor
 *          voltage directly after the fuse. Then a voltage difference between
 *          V_bat and V_fuse indicates a tripped fuse.
 *
 *            V_bat  +------+     V_fuse       Precharge/Charge+ contactor
 *          -----+---| FUSE |-----+------------/   -----------------
 *                   +------+
 */
#define BS_CHECK_FUSE_PLACED_IN_CHARGE_PATH (false)

/*========== Extern Constant and Variable Declarations ======================*/
/** Precharge presence of not for each string */
extern BS_STRING_PRECHARGE_PRESENT_e bs_stringsWithPrecharge[BS_NR_OF_STRINGS];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__BATTERY_SYSTEM_CFG_H_ */
