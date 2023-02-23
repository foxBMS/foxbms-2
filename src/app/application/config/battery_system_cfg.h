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
 * @file    battery_system_cfg.h
 * @author  foxBMS Team
 * @date    2019-12-10 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
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

#include "fassert.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Symbolic identifiers for strings with precharge */
typedef enum {
    BS_STRING_WITH_PRECHARGE,
    BS_STRING_WITHOUT_PRECHARGE,
} BS_STRING_PRECHARGE_PRESENT_e;

/** Symbolic identifiers for strings. */
typedef enum {
    BS_STRING0    = 0u,
    BS_STRING1    = 1u,
    BS_STRING2    = 2u,
    BS_STRING_MAX = 3u,
} BS_STRING_ID_e;

/** Define if discharge current is seen as positive or negative */
#define BS_POSITIVE_DISCHARGE_CURRENT (true)

/**
 * @brief   Number of parallel strings in the battery pack
 * @details For details see
 *          <a href="../../../../introduction/naming-conventions.html" target="_blank">Naming Conventions</a>.
 *          Implementation detail: The number of parallel strings cannot exceed
 *          #GEN_REPEAT_MAXIMUM_REPETITIONS unless the implementation of the
 *          repetition macro is adapted.
 * @ptype   uint
 */
#define BS_NR_OF_STRINGS (1u)

/* safety check: due to implementation BS_NR_OF_STRINGS may not be larger than GEN_REPEAT_MAXIMUM_REPETITIONS */
#if (BS_NR_OF_STRINGS > GEN_REPEAT_MAXIMUM_REPETITIONS)
#error "Too large number of strings, please check implementation of GEN_REPEAT_U()."
#endif

/**
 * @brief   number of modules in a string
 * @details For details see
 *          <a href="../../../../introduction/naming-conventions.html" target="_blank">Naming Conventions</a>.
 * @ptype   uint
 */
#define BS_NR_OF_MODULES_PER_STRING (1u)

/**
 * @brief   number of cells per module
 * @details number of cells per module, where parallel cells are
 *          counted as one cell block.
 *          For details see
 *          <a href="../../../../introduction/naming-conventions.html" target="_blank">Naming Conventions</a>.
 * @ptype   uint
 */
#define BS_NR_OF_CELL_BLOCKS_PER_MODULE (18u)

/**
 * @brief   number of battery cells in a parallel cell connection per battery
 *          module
 * @details For details see
 *          <a href="../../../../introduction/naming-conventions.html" target="_blank">Naming Conventions</a>.
 * @ptype   uint
 */
#define BS_NR_OF_PARALLEL_CELLS_PER_MODULE (1u)

/** Value of the balancing resistors on the slave-board */
#define BS_BALANCING_RESISTANCE_ohm (100.0)

/**
 * @def     BS_NR_OF_GPIOS_PER_MODULE
 * @brief   Defines the number of GPIOs
 */
#define BS_NR_OF_GPIOS_PER_MODULE (10u)

/**
 * @def     BS_NR_OF_GPAS_PER_MODULE
 * @brief   Defines the number of GPA inputs
 */
#define BS_NR_OF_GPAS_PER_MODULE (2u)

/**
 * @brief   number of temperature sensors per battery module
 * @ptype   int
 */
#define BS_NR_OF_TEMP_SENSORS_PER_MODULE (8u)

#if BS_NR_OF_TEMP_SENSORS_PER_MODULE > BS_NR_OF_GPIOS_PER_MODULE
#error "Number of temperature inputs cannot be higher than number of GPIOs"
#endif

/** number of battery cells in the system */
#define BS_NR_OF_CELL_BLOCKS_PER_STRING (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE)
/** number of temperature sensors in a string */
#define BS_NR_OF_TEMP_SENSORS_PER_STRING (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE)
/** number of temperature sensors in the battery system */
#define BS_NR_OF_TEMP_SENSORS (BS_NR_OF_TEMP_SENSORS_PER_STRING * BS_NR_OF_STRINGS)

/**
 * @details - If set to false, foxBMS does not check for the presence of a
 *            current sensor.
 *          - If set to true, foxBMS checks for the presence of a current
 *            sensor. If sensor stops responding during runtime, an error is
 *            raised.
 */
#define BS_CURRENT_SENSOR_PRESENT (false)

#if BS_CURRENT_SENSOR_PRESENT == true
/**
 * defines if the Isabellenhuette current sensor is used in cyclic or triggered mode
 */
#define CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
/* #define CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED */

/** Delay in ms after which it is considered the current measurement is not responding anymore. */
#define BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms (200u)

/** Delay in ms after which it is considered the coulomb counting is not responding anymore. */
#define BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms (2000u)

/** Delay in ms after which it is considered the energy counting is not responding anymore. */
#define BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms (2000u)

#endif /* BS_CURRENT_SENSOR_PRESENT == true */

/**
 * @brief   Maximum break current of main contactors.
 * @details The contacts of the main contactors can be welded, when attempting
 *          to interrupt the current flow while a current higher than the
 *          maximum break current of the contactor is flowing.
 *
 *          Thus, the contactors will not be opened if the floating current is
 *          above this value. The fuse should trigger to interrupt current
 *          flows above this value.
 */
#define BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA (500000)

/**
 * @brief   Maximum fuse trigger duration
 * @details If the current is above #BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA,
 *          the BMS state machine will wait this time until the fuse triggers so
 *          that the current will be interrupted by the fuse and not the
 *          contactors. After this time, the BMS will nevertheless try to open
 *          the contactors.
 */
#define BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms (30000u)

/**
 * @brief   Maximum string current limit in mA that is used in the SOA module
 *          to check for string overcurrent
* @details  When maximum safety limit (MSL) is violated, error state is
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
 * @brief   Define if interlock feedback should be discarded or not
 * @details True: interlock feedback will be discarded
 *          False: interlock feedback will evaluated
 */
#define BS_IGNORE_INTERLOCK_FEEDBACK (false)

/**
 * @brief   Defines whether CAN timing shall be evaluated or not
 * @details - If set to false, foxBMS does not check CAN timing.
 *          - If set to true, foxBMS checks CAN timing. A valid request must
 *            come every 100ms, within the 95-150ms window.
 */
#define BS_CHECK_CAN_TIMING (true)

/**
 * @brief   Defines whether balancing shall be available or not
 * @details - If set to true, balancing is deactivated completely.
 *          - If set to false, foxBMS checks when balancing must be done and
 *            activates it accordingly.
 */
#define BS_BALANCING_DEFAULT_INACTIVE (true)

/**
 * @brief   number of high voltage inputs measured by current sensors (like
 *          IVT-MOD)
 * @ptype   int
 */
#define BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR (3u)

/** Number of contactors in addition to string contactors (e.g., PRECHARGE).*/
#define BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS (1u)

/** Total number of contactors in system:
 *  - Two contactors per string (string+ and string-)
 *  - One optional precharge contactor for each string */
#define BS_NR_OF_CONTACTORS ((2u * BS_NR_OF_STRINGS) + BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS)

/**
 * @brief   current threshold for determining rest state of battery. If absolute
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

/**
 * \defgroup    open wire check configuration
 *  @details    If open-wire check is performed, depending on the AFE
 *              implementation, cell voltages and temperatures are not updated
 *              and thus old values can be transmitted on the CAN bus. Check
 *              time is dependent on module configuration and external
 *              capacitance. Activate open-wire check with care! See the AFE
 *              implementation for details.
 * @{
 */
/** enable open-wire checks during standby */
#define BS_STANDBY_PERIODIC_OPEN_WIRE_CHECK (false)

/** Periodic open-wire check time in STANDBY state in ms */
#define BS_STANDBY_OPEN_WIRE_PERIOD_ms (600000)

/** open-wire check in normal mode (set to true or false) */
#define BS_NORMAL_PERIODIC_OPEN_WIRE_CHECK (false)

/** Periodic open-wire check time in NORMAL state in ms */
#define BS_NORMAL_OPEN_WIRE_PERIOD_ms (600000)

/** open-wire check in charge mode (set to true or false) */
#define BS_CHARGE_PERIODIC_OPEN_WIRE_CHECK (false)

/** Periodic open-wire check time in CHARGE state in ms */
#define BS_CHARGE_OPEN_WIRE_PERIOD_ms (600000)

/** Periodic open-wire check time in ERROR state in ms */
#define BS_ERROR_OPEN_WIRE_PERIOD_ms (30000)
/**@}*/

FAS_STATIC_ASSERT((BS_NR_OF_STRINGS <= (uint8_t)UINT8_MAX), "This code assumes BS_NR_OF_STRINGS fits into uint8_t");

/*========== Extern Constant and Variable Declarations ======================*/
/** Defines whether a string can be used to precharge or not */
extern BS_STRING_PRECHARGE_PRESENT_e bs_stringsWithPrecharge[BS_NR_OF_STRINGS];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BATTERY_SYSTEM_CFG_H_ */
