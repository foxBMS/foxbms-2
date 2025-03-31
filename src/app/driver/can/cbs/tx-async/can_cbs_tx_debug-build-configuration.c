/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cbs_tx_debug-build-configuration.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for sending the build configuration
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx_debug-build-configuration.h"

#include "app_build_cfg.h"
#include "battery_cell_cfg.h"
#include "battery_system_cfg.h"
#include "bms-slave_cfg.h"

#include "can.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** @{
 *  current sensor mode enum
 */
typedef enum {
    CURRENT_SENSOR_CYCLIC,
    CURRENT_SENSOR_TRIGGERED,
} CANTX_CURRENT_SENSOR_MODE_e;
/** @} */

/** @{
 * configuration of the multiplexer
 */
#define CANTX_MUX_START_BIT                              (7u)
#define CANTX_MUX_LENGTH                                 (8u)
#define CANTX_MUX_SLAVE                                  (0x00u)
#define CANTX_MUX_APPLICATION                            (0x10u)
#define CANTX_MUX_BATTERY_CELL                           (0x20u)
#define CANTX_MUX_BATTERY_CELL_MAX_CHARGE_CURRENT        (0x21u)
#define CANTX_MUX_BATTERY_CELL_MAX_CHARGE_TEMPERATURE    (0x22u)
#define CANTX_MUX_BATTERY_CELL_MAX_DISCHARGE_CURRENT     (0x23u)
#define CANTX_MUX_BATTERY_CELL_MAX_DISCHARGE_TEMPERATURE (0x24u)
#define CANTX_MUX_BATTERY_CELL_MAX_VOLTAGE               (0x25u)
#define CANTX_MUX_BATTERY_CELL_MIN_CHARGE_TEMPERATURE    (0x26u)
#define CANTX_MUX_BATTERY_CELL_MIN_DISCHARGE_TEMPERATURE (0x27u)
#define CANTX_MUX_BATTERY_CELL_MIN_VOLTAGE               (0x28u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1               (0x30u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2               (0x31u)
#define CANTX_MUX_BATTERY_SYSTEM_CONTACTORS              (0x32u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR          (0x33u)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE                    (0x34u)
#define CANTX_MUX_BATTERY_SYSTEM_MAX_CURRENT             (0x35u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK         (0x36u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS           (0x37u)
/** @} */

/** @{
 * configuration of the slave multiplexer message
 */
#define CANTX_MUX_SLAVE_AFE_START_BIT                       (15u)
#define CANTX_MUX_SLAVE_AFE_LENGTH                          (8u)
#define CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_START_BIT        (23u)
#define CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_LENGTH           (8u)
#define CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_METHOD_START_BIT (31u)
#define CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_METHOD_LENGTH    (8u)
/** @} */

/** @{
 * configuration of the application multiplexer message
 */
#define CANTX_MUX_APPLICATION_SOC_ALGORITHM_START_BIT      (15u)
#define CANTX_MUX_APPLICATION_SOC_ALGORITHM_LENGTH         (4u)
#define CANTX_MUX_APPLICATION_SOE_ALGORITHM_START_BIT      (11u)
#define CANTX_MUX_APPLICATION_SOE_ALGORITHM_LENGTH         (4u)
#define CANTX_MUX_APPLICATION_SOF_ALGORITHM_START_BIT      (23u)
#define CANTX_MUX_APPLICATION_SOF_ALGORITHM_LENGTH         (4u)
#define CANTX_MUX_APPLICATION_SOH_ALGORITHM_START_BIT      (19u)
#define CANTX_MUX_APPLICATION_SOH_ALGORITHM_LENGTH         (4u)
#define CANTX_MUX_APPLICATION_BALANCING_STRATEGY_START_BIT (31u)
#define CANTX_MUX_APPLICATION_BALANCING_STRATEGY_LENGTH    (4u)
#define CANTX_MUX_APPLICATION_IMD_START_BIT                (27u)
#define CANTX_MUX_APPLICATION_IMD_LENGTH                   (4u)
#define CANTX_MUX_APPLICATION_RTOS_START_BIT               (39u)
#define CANTX_MUX_APPLICATION_RTOS_LENGTH                  (4u)
/** @} */

/** @{
 * configuration of the battery cell multiplexer messages
 */
/* battery cell */
#define CANTX_MUX_BATTERY_CELL_NOMINAL_CELL_VOLTAGE_START_BIT        (15u)
#define CANTX_MUX_BATTERY_CELL_NOMINAL_CELL_VOLTAGE_LENGTH           (16u)
#define CANTX_MUX_BATTERY_CELL_DEEP_DISCHARGE_CELL_VOLTAGE_START_BIT (31u)
#define CANTX_MUX_BATTERY_CELL_DEEP_DISCHARGE_CELL_VOLTAGE_LENGTH    (16u)
#define CANTX_MUX_BATTERY_CELL_CELL_CAPACITY_START_BIT               (47u)
#define CANTX_MUX_BATTERY_CELL_CELL_CAPACITY_LENGTH                  (16u)
#define CANTX_MUX_BATTERY_CELL_CELL_ENERGY_START_BIT                 (63u)
#define CANTX_MUX_BATTERY_CELL_CELL_ENERGY_LENGTH                    (8u)

/* battery cell msl/rsl/mol values */
#define CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT (15u)
#define CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH    (18u)
#define CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT (29u)
#define CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH    (18u)
#define CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT (43u)
#define CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH    (18u)
/** @} */

/** @{
 * defines for the signal type for temperature value signals
 */
#define CANTX_MUX_BATTERY_CELL_TEMPERATURES_MAX_VALUE (131071.0f)
#define CANTX_MUX_BATTERY_CELL_TEMPERATURES_MIN_VALUE (-131072.0f)
#define CANTX_MUX_BATTERY_CELL_TEMPERATURES_FACTOR    (1.0f)
#define CANTX_MUX_BATTERY_CELL_TEMPERATURES_OFFSET    (0.0f)
/** @} */

/** @{
 * signal type for temperature limit values
 */
static const CAN_SIGNAL_TYPE_s cantx_signalBatteryCellTemperatures = {
    0u,
    0u,
    CANTX_MUX_BATTERY_CELL_TEMPERATURES_FACTOR,
    CANTX_MUX_BATTERY_CELL_TEMPERATURES_OFFSET,
    CANTX_MUX_BATTERY_CELL_TEMPERATURES_MIN_VALUE,
    CANTX_MUX_BATTERY_CELL_TEMPERATURES_MAX_VALUE};
/** @} */

/** @{
 * configuration of the battery system multiplexer messages
 */
/* battery system general 1*/
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_STRINGS_START_BIT                        (15u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_STRINGS_LENGTH                           (8u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_MODULES_PER_STRING_START_BIT             (23u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_MODULES_PER_STRING_LENGTH                (8u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_CELL_BLOCKS_PER_MODULE_START_BIT         (31u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_CELL_BLOCKS_PER_MODULE_LENGTH            (8u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK_START_BIT  (39u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK_LENGTH     (8u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPIOS_PER_MODULE_START_BIT               (47u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPIOS_PER_MODULE_LENGTH                  (8u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPAS_PER_MODULE_START_BIT                (55u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPAS_PER_MODULE_LENGTH                   (8u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_TEMPERATURE_SENSORS_PER_MODULE_START_BIT (63u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_TEMPERATURE_SENSORS_PER_MODULE_LENGTH    (8u)

/* battery system general 2*/
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_POSITIVE_DISCHARGE_CURRENT_START_BIT (15u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_POSITIVE_DISCHARGE_CURRENT_LENGTH    (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_IGNORE_INTERLOCK_FEEDBACK_START_BIT  (14u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_IGNORE_INTERLOCK_FEEDBACK_LENGTH     (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_CHECK_CAN_TIMING_START_BIT           (13u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_CHECK_CAN_TIMING_LENGTH              (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_DEFAULT_INACTIVE_START_BIT (12u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_DEFAULT_INACTIVE_LENGTH    (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_RESISTANCE_START_BIT       (11u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_RESISTANCE_LENGTH          (12u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_REST_CURRENT_START_BIT               (31u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_REST_CURRENT_LENGTH                  (12u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_RELAXATION_PERIOD_START_BIT          (35u)
#define CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_RELAXATION_PERIOD_LENGTH             (16u)

/* battery system contactors*/
#define CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_OUTSIDE_STRINGS_START_BIT (15u)
#define CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_OUTSIDE_STRINGS_LENGTH    (8u)
#define CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_START_BIT                 (23u)
#define CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_LENGTH                    (8u)
#define CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_MAIN_CONTACTOR_MAX_BREAK_CURRENT_START_BIT (31u)
#define CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_MAIN_CONTACTOR_MAX_BREAK_CURRENT_LENGTH    (32u)

/* battery system current sensor*/
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_PRESENT_START_BIT               (15u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_PRESENT_LENGTH                  (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_CYCLIC_START_BIT                (14u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_CYCLIC_LENGTH                   (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR_START_BIT   (13u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR_LENGTH      (2u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_MEASUREMENT_TIMEOUT_START_BIT          (11u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_MEASUREMENT_TIMEOUT_LENGTH             (12u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_COULOMB_COUNTING_MEASUREMENT_TIMEOUT_START_BIT (31u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_COULOMB_COUNTING_MEASUREMENT_TIMEOUT_LENGTH    (12u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_ENERGY_COUNTING_MEASUREMENT_TIMEOUT_START_BIT  (35u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_ENERGY_COUNTING_MEASUREMENT_TIMEOUT_LENGTH     (12u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_THRESHOLD_NO_CURRENT_START_BIT                 (55u)
#define CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_THRESHOLD_NO_CURRENT_LENGTH                    (12u)

/* battery system fuse*/
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_CHARGE_PATH_START_BIT  (15u)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_CHARGE_PATH_LENGTH     (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_NORMAL_PATH_START_BIT  (14u)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_NORMAL_PATH_LENGTH     (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_MAX_VOLTAGE_DROP_OVER_FUSE_START_BIT         (11u)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_MAX_VOLTAGE_DROP_OVER_FUSE_LENGTH            (12u)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_START_BIT (31u)
#define CANTX_MUX_BATTERY_SYSTEM_FUSE_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_LENGTH    (16u)

/* battery system maximum current*/
#define CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_STRING_CURRENT_START_BIT (15u)
#define CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_STRING_CURRENT_LENGTH    (28u)
#define CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_PACK_CURRENT_START_BIT   (35u)
#define CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_PACK_CURRENT_LENGTH      (28u)

/* battery system open wire check*/
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_PERIODIC_OPEN_WIRE_CHECK_START_BIT (15u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_PERIODIC_OPEN_WIRE_CHECK_LENGTH    (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_OPEN_WIRE_PERIOD_START_BIT         (14u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_OPEN_WIRE_PERIOD_LENGTH            (12u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_PERIODIC_OPEN_WIRE_CHECK_START_BIT  (18u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_PERIODIC_OPEN_WIRE_CHECK_LENGTH     (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_OPEN_WIRE_PERIOD_START_BIT          (17u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_OPEN_WIRE_PERIOD_LENGTH             (12u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_PERIODIC_OPEN_WIRE_CHECK_START_BIT  (37u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_PERIODIC_OPEN_WIRE_CHECK_LENGTH     (CAN_BIT)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_OPEN_WIRE_PERIOD_START_BIT          (36u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_OPEN_WIRE_PERIOD_LENGTH             (12u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_ERROR_OPEN_WIRE_PERIOD_START_BIT           (40u)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_ERROR_OPEN_WIRE_PERIOD_LENGTH              (12u)

/* battery system total cell and temperature sensor numbers*/
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_PER_STRING_START_BIT  (15u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_PER_STRING_LENGTH     (14u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_START_BIT             (17u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_LENGTH                (14u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_PER_STRING_START_BIT (35u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_PER_STRING_LENGTH    (14u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_START_BIT            (53u)
#define CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_LENGTH               (14u)
/** @} */

/** @{
 * defines for signal type for open wire period values
 */
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_MAX_VALUE (4095000.0f)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_MIN_VALUE (0.0f)
#define CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_OFFSET    (0.0f)
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Sends the application configuration message
 * @param   pMessageData message data of the CAN message
 * @param   muxValue multiplexer value to be set
 */
static void CANTX_SetDebugBuildConfigurationMux(uint64_t *pMessageData, uint8_t muxValue);

/**
 * @brief   Sends the application configuration message
 */
static STD_RETURN_TYPE_e CANTX_SendApplicationConfiguration(void);

/**
 * @brief   Sends the slave configuration message
 */
static STD_RETURN_TYPE_e CANTX_SendSlaveConfiguration(void);

/**
 * @brief   Sends the battery cell configuration message
 */
static STD_RETURN_TYPE_e CANTX_SendBatteryCellConfiguration(void);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMaxVoltageMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatteryCellMuxMinVoltageMessageData(uint64_t *pMessageData);

/**
 * @brief   Sends the battery system configuration message
 */
static STD_RETURN_TYPE_e CANTX_SendBatterySystemConfiguration(void);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxGeneral1MessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxGeneral2MessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxContactorsMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxCurrentSensorMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxFuseMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxMaxCurrentMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxOpenWireCheckMessageData(uint64_t *pMessageData);

/**
 * @brief   Adds the data to the message
 * @param   pMessageData message data of the CAN message
 */
static void CANTX_SetBatterySystemMuxTotalNumbersMessageData(uint64_t *pMessageData);

/*========== Static Function Implementations ================================*/
static void CANTX_SetDebugBuildConfigurationMux(uint64_t *pMessageData, uint8_t muxValue) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT(
        (muxValue == CANTX_MUX_SLAVE) || (muxValue == CANTX_MUX_APPLICATION) ||
        ((muxValue >= CANTX_MUX_BATTERY_CELL) && (muxValue <= CANTX_MUX_BATTERY_CELL_MIN_VOLTAGE)) ||
        ((muxValue >= CANTX_MUX_BATTERY_SYSTEM_GENERAL_1) && (muxValue <= CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS)));

    CAN_TxSetMessageDataWithSignalData(
        pMessageData, CANTX_MUX_START_BIT, CANTX_MUX_LENGTH, muxValue, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static STD_RETURN_TYPE_e CANTX_SendApplicationConfiguration(void) {
    uint64_t messageData         = 0u;
    uint8_t canData[CAN_MAX_DLC] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(&messageData, CANTX_MUX_APPLICATION);

    /* Set soc algorithm */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_APPLICATION_SOC_ALGORITHM_START_BIT,
        CANTX_MUX_APPLICATION_SOC_ALGORITHM_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.socAlgorithm,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set soe algorithm */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_APPLICATION_SOE_ALGORITHM_START_BIT,
        CANTX_MUX_APPLICATION_SOE_ALGORITHM_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.soeAlgorithm,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set sof algorithm */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_APPLICATION_SOF_ALGORITHM_START_BIT,
        CANTX_MUX_APPLICATION_SOF_ALGORITHM_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.sofAlgorithm,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set soh algorithm */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_APPLICATION_SOH_ALGORITHM_START_BIT,
        CANTX_MUX_APPLICATION_SOH_ALGORITHM_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.sohAlgorithm,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set balancing strategy */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_APPLICATION_BALANCING_STRATEGY_START_BIT,
        CANTX_MUX_APPLICATION_BALANCING_STRATEGY_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.balancingStrategy,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set IMD */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_APPLICATION_IMD_START_BIT,
        CANTX_MUX_APPLICATION_IMD_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.imdName,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set rtos */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_APPLICATION_RTOS_START_BIT,
        CANTX_MUX_APPLICATION_RTOS_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.rtos,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set the can data */
    CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Send the message */
    STD_RETURN_TYPE_e queuedSuccessfully = CAN_DataSend(
        CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_BUILD_CONFIGURATION_ID, CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE, canData);

    return queuedSuccessfully;
}

static STD_RETURN_TYPE_e CANTX_SendSlaveConfiguration(void) {
    uint64_t messageData         = 0u;
    uint8_t canData[CAN_MAX_DLC] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(&messageData, CANTX_MUX_SLAVE);

    /* Set afe name */
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_SLAVE_AFE_START_BIT,
        CANTX_MUX_SLAVE_AFE_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.afeName,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set temperature sensor name*/
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_START_BIT,
        CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.temperatureSensorName,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set temperature sensor method*/
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_METHOD_START_BIT,
        CANTX_MUX_SLAVE_TEMPERATURE_SENSOR_METHOD_LENGTH,
        (uint64_t)ver_foxbmsBuildConfiguration.temperatureSensorMethod,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set the can data */
    CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Send the message */
    STD_RETURN_TYPE_e queuedSuccessfully = CAN_DataSend(
        CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_BUILD_CONFIGURATION_ID, CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE, canData);

    return queuedSuccessfully;
}

static STD_RETURN_TYPE_e CANTX_SendBatteryCellConfiguration(void) {
    uint64_t messageData         = 0u;
    uint8_t canData[CAN_MAX_DLC] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set battery cell message data */
    CANTX_SetBatteryCellMuxMessageData(&messageData);
    /* Send battery cell message*/
    CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
    STD_RETURN_TYPE_e queuedSuccessfully = CAN_DataSend(
        CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_BUILD_CONFIGURATION_ID, CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE, canData);
    messageData = 0u;

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell max charge current message data*/
        CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(&messageData);
        /* Send battery cell max charge current message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell max charge temperature message data*/
        CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(&messageData);
        /* Send battery cell max charge temperature message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell max discharge current message data*/
        CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(&messageData);
        /* Send battery cell max discharge current message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell max discharge temperature message data*/
        CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(&messageData);
        /* Send battery cell max discharge temperature message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell max voltage message data*/
        CANTX_SetBatteryCellMuxMaxVoltageMessageData(&messageData);
        /* Send battery cell max voltage message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell min charge temperature message data*/
        CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(&messageData);
        /* Send battery cell min charge temperature message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell min discharge temperature message data*/
        CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(&messageData);
        /* Send battery cell min discharge temperature message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery cell min voltage message data*/
        CANTX_SetBatteryCellMuxMinVoltageMessageData(&messageData);
        /* Send battery cell min voltage message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
    }

    return queuedSuccessfully;
}

static void CANTX_SetBatteryCellMuxMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL);

    /* Set nominal cell voltage value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_NOMINAL_CELL_VOLTAGE_START_BIT,
        CANTX_MUX_BATTERY_CELL_NOMINAL_CELL_VOLTAGE_LENGTH,
        BC_VOLTAGE_NOMINAL_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set deep discharge cell voltage value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_DEEP_DISCHARGE_CELL_VOLTAGE_START_BIT,
        CANTX_MUX_BATTERY_CELL_DEEP_DISCHARGE_CELL_VOLTAGE_LENGTH,
        BC_VOLTAGE_DEEP_DISCHARGE_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set cell capacity value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_CELL_CAPACITY_START_BIT,
        CANTX_MUX_BATTERY_CELL_CELL_CAPACITY_LENGTH,
        BC_CAPACITY_mAh,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set cell energy value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_CELL_ENERGY_START_BIT,
        CANTX_MUX_BATTERY_CELL_CELL_ENERGY_LENGTH,
        (uint64_t)BC_ENERGY_Wh,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MAX_CHARGE_CURRENT);

    /* Set maximal charge current MSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        BC_CURRENT_MAX_CHARGE_MSL_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal charge current RSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        BC_CURRENT_MAX_CHARGE_RSL_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal charge current MOL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        BC_CURRENT_MAX_CHARGE_MOL_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MAX_CHARGE_TEMPERATURE);

    /* Set maximal charge temperature MSL value*/
    float_t signalData = (float_t)BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal charge temperature RSL value*/
    signalData = (float_t)BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal charge temperature MOL value*/
    signalData = (float_t)BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MAX_DISCHARGE_CURRENT);

    /* Set maximal discharge current MSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        BC_CURRENT_MAX_DISCHARGE_MSL_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal discharge current RSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        BC_CURRENT_MAX_DISCHARGE_RSL_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal discharge current MOL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        BC_CURRENT_MAX_DISCHARGE_MOL_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MAX_DISCHARGE_TEMPERATURE);

    /* Set maximal discharge temperature MSL value*/
    float_t signalData = (float_t)BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal discharge temperature RSL value*/
    signalData = (float_t)BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal discharge temperature MOL value*/
    signalData = (float_t)BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMaxVoltageMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MAX_VOLTAGE);

    /* Set maximal voltage MSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        BC_VOLTAGE_MAX_MSL_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal voltage RSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        BC_VOLTAGE_MAX_RSL_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set maximal voltage MOL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        BC_VOLTAGE_MAX_MOL_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MIN_CHARGE_TEMPERATURE);

    /* Set minimal charge temperature MSL value*/
    float_t signalData = (float_t)BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set minimal charge temperature RSL value*/
    signalData = (float_t)BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set minimal charge temperature MOL value*/
    signalData = (float_t)BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MIN_DISCHARGE_TEMPERATURE);

    /* Set minimal discharge temperature MSL value*/
    float_t signalData = (float_t)BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set minimal discharge temperature RSL value*/
    signalData = (float_t)BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set minimal discharge temperature MOL value*/
    signalData = (float_t)BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC;
    CAN_TxPrepareSignalData(&signalData, cantx_signalBatteryCellTemperatures);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        (int64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatteryCellMuxMinVoltageMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_CELL_MIN_VOLTAGE);

    /* Set minimal voltage MSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MSL_LENGTH,
        BC_VOLTAGE_MIN_MSL_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set minimal voltage RSL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_RSL_LENGTH,
        BC_VOLTAGE_MIN_RSL_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set minimal voltage MOL value*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_START_BIT,
        CANTX_MUX_BATTERY_CELL_VALUES_MOL_LENGTH,
        BC_VOLTAGE_MIN_MOL_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static STD_RETURN_TYPE_e CANTX_SendBatterySystemConfiguration(void) {
    uint64_t messageData         = 0u;
    uint8_t canData[CAN_MAX_DLC] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set battery system general 1 message data*/
    CANTX_SetBatterySystemMuxGeneral1MessageData(&messageData);
    /* Send battery system general 1 message*/
    CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
    STD_RETURN_TYPE_e queuedSuccessfully = CAN_DataSend(
        CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_BUILD_CONFIGURATION_ID, CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE, canData);
    messageData = 0u;

    if (queuedSuccessfully == STD_OK) {
        /* Set battery system general 2 message data*/
        CANTX_SetBatterySystemMuxGeneral2MessageData(&messageData);
        /* Send battery system general 2 message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery system contactors message data*/
        CANTX_SetBatterySystemMuxContactorsMessageData(&messageData);
        /* Send battery system contactors message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery system current sensor message data*/
        CANTX_SetBatterySystemMuxCurrentSensorMessageData(&messageData);
        /* Send battery system current sensor message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery system fuse message data*/
        CANTX_SetBatterySystemMuxFuseMessageData(&messageData);
        /* Send battery system fuse message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery system max current message data*/
        CANTX_SetBatterySystemMuxMaxCurrentMessageData(&messageData);
        /* Send battery system max current message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery system open wire check message data*/
        CANTX_SetBatterySystemMuxOpenWireCheckMessageData(&messageData);
        /* Send battery system open wire check message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
        messageData = 0u;
    }

    if (queuedSuccessfully == STD_OK) {
        /* Set battery system total numbers message data*/
        CANTX_SetBatterySystemMuxTotalNumbersMessageData(&messageData);
        /* Send battery system total numbers message*/
        CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
        queuedSuccessfully = CAN_DataSend(
            CAN_NODE_DEBUG_MESSAGE,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID,
            CANTX_DEBUG_BUILD_CONFIGURATION_ID_TYPE,
            canData);
    }

    return queuedSuccessfully;
}

static void CANTX_SetBatterySystemMuxGeneral1MessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_GENERAL_1);

    /* Set number of strings value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_STRINGS_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_STRINGS_LENGTH,
        BS_NR_OF_STRINGS,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of modules per string value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_MODULES_PER_STRING_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_MODULES_PER_STRING_LENGTH,
        BS_NR_OF_MODULES_PER_STRING,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number fo cell blocks per module value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_CELL_BLOCKS_PER_MODULE_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_CELL_BLOCKS_PER_MODULE_LENGTH,
        BS_NR_OF_CELL_BLOCKS_PER_MODULE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of parallel cells per cell block value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK_LENGTH,
        BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of gpios per module value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPIOS_PER_MODULE_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPIOS_PER_MODULE_LENGTH,
        SLV_NR_OF_GPIOS_PER_MODULE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of gpas per module value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPAS_PER_MODULE_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_GPAS_PER_MODULE_LENGTH,
        SLV_NR_OF_GPAS_PER_MODULE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of temperature sensors per module value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_TEMPERATURE_SENSORS_PER_MODULE_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_1_NR_OF_TEMPERATURE_SENSORS_PER_MODULE_LENGTH,
        BS_NR_OF_TEMP_SENSORS_PER_MODULE,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatterySystemMuxGeneral2MessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_GENERAL_2);

    /* Set positive discharge current flag */
    uint64_t signalData = CAN_ConvertBooleanToInteger(BS_POSITIVE_DISCHARGE_CURRENT);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_POSITIVE_DISCHARGE_CURRENT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_POSITIVE_DISCHARGE_CURRENT_LENGTH,
        signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set ignore interlock feedback flag */
    signalData = CAN_ConvertBooleanToInteger(BS_IGNORE_INTERLOCK_FEEDBACK);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_IGNORE_INTERLOCK_FEEDBACK_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_IGNORE_INTERLOCK_FEEDBACK_LENGTH,
        signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set check can timing flag */
    signalData = CAN_ConvertBooleanToInteger(BS_CHECK_CAN_TIMING);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_CHECK_CAN_TIMING_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_CHECK_CAN_TIMING_LENGTH,
        signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set balancing default inactive flag */
    signalData = CAN_ConvertBooleanToInteger(BS_BALANCING_DEFAULT_INACTIVE);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_DEFAULT_INACTIVE_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_DEFAULT_INACTIVE_LENGTH,
        signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set balancing resistance value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_RESISTANCE_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_BALANCING_RESISTANCE_LENGTH,
        (uint64_t)SLV_BALANCING_RESISTANCE_ohm,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set rest current value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_REST_CURRENT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_REST_CURRENT_LENGTH,
        BS_REST_CURRENT_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set relaxation period value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_RELAXATION_PERIOD_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_GENERAL_2_RELAXATION_PERIOD_LENGTH,
        BS_RELAXATION_PERIOD_10ms,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatterySystemMuxContactorsMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_CONTACTORS);

    /* Set number of contactors outside string value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_OUTSIDE_STRINGS_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_OUTSIDE_STRINGS_LENGTH,
        BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of contactors value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_NR_OF_CONTACTORS_LENGTH,
        BS_NR_OF_CONTACTORS,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set main contactors maximum break current value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_MAIN_CONTACTOR_MAX_BREAK_CURRENT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CONTACTORS_MAIN_CONTACTOR_MAX_BREAK_CURRENT_LENGTH,
        BS_MAIN_CONTACTORS_MAXIMUM_BREAK_CURRENT_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatterySystemMuxCurrentSensorMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR);

    /* Set current sensor present flag */
    uint64_t signalData = CAN_ConvertBooleanToInteger(BS_CURRENT_SENSOR_PRESENT);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_PRESENT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_PRESENT_LENGTH,
        signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of voltages from current sensor value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR_LENGTH,
        BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

#if BS_CURRENT_SENSOR_PRESENT == true
#ifdef CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC
    /* Set current sensor cyclic flag */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_CYCLIC_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_CYCLIC_LENGTH,
        (uint64_t)CURRENT_SENSOR_CYCLIC,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
#elif CURRENT_SENSOR_ISABELLENHUETTE_TRIGGERED
    /* Set current sensor cyclic flag */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_CYCLIC_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_SENSOR_CYCLIC_LENGTH,
        (uint64_t)CURRENT_SENSOR_TRIGGERED,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
#endif /* CURRENT_SENSOR_ISABELLENHUETTE_CYCLIC*/

    /* Set current measurement timeout value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_MEASUREMENT_TIMEOUT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_CURRENT_MEASUREMENT_TIMEOUT_LENGTH,
        BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set coulomb counting measurement timeout value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_COULOMB_COUNTING_MEASUREMENT_TIMEOUT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_COULOMB_COUNTING_MEASUREMENT_TIMEOUT_LENGTH,
        BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set energy counting measurement timeout value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_ENERGY_COUNTING_MEASUREMENT_TIMEOUT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_ENERGY_COUNTING_MEASUREMENT_TIMEOUT_LENGTH,
        BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */

    /* Set threshold no current value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_THRESHOLD_NO_CURRENT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_CURRENT_SENSOR_THRESHOLD_NO_CURRENT_LENGTH,
        BS_CS_THRESHOLD_NO_CURRENT_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatterySystemMuxFuseMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_FUSE);

    /* Set check fuse placed in charge path flag */
    uint64_t signalData = CAN_ConvertBooleanToInteger(BS_CHECK_FUSE_PLACED_IN_CHARGE_PATH);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_CHARGE_PATH_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_CHARGE_PATH_LENGTH,
        signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set check fuse placed in normal path flag */
    signalData = CAN_ConvertBooleanToInteger(BS_CHECK_FUSE_PLACED_IN_NORMAL_PATH);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_NORMAL_PATH_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_CHECK_FUSED_PLACED_IN_NORMAL_PATH_LENGTH,
        signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set max voltage drop over fuse value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_MAX_VOLTAGE_DROP_OVER_FUSE_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_MAX_VOLTAGE_DROP_OVER_FUSE_LENGTH,
        BS_MAX_VOLTAGE_DROP_OVER_FUSE_mV,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set main fuse maximum trigger duration value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_FUSE_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_LENGTH,
        BS_MAIN_FUSE_MAXIMUM_TRIGGER_DURATION_ms,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatterySystemMuxMaxCurrentMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_MAX_CURRENT);

    /* Set max string current value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_STRING_CURRENT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_STRING_CURRENT_LENGTH,
        BS_MAXIMUM_STRING_CURRENT_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set max pack current value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_PACK_CURRENT_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_MAXIMUM_CURRENT_MAX_PACK_CURRENT_LENGTH,
        BS_MAXIMUM_PACK_CURRENT_mA,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatterySystemMuxOpenWireCheckMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    /* Signal type of the open wire period values */
    static const CAN_SIGNAL_TYPE_s cantx_signalOpenWirePeriod = {
        0u,
        0u,
        UNIT_CONVERSION_FACTOR_1000_FLOAT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_OFFSET,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_MIN_VALUE,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_MAX_VALUE};

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK);

    /* Set standby periodic open wire check flag */
    uint64_t convertedSignalData = CAN_ConvertBooleanToInteger(BS_STANDBY_PERIODIC_OPEN_WIRE_CHECK);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_PERIODIC_OPEN_WIRE_CHECK_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_PERIODIC_OPEN_WIRE_CHECK_LENGTH,
        convertedSignalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set standby open wire period value */
    float_t signalData = (float_t)BS_STANDBY_OPEN_WIRE_PERIOD_ms;
    CAN_TxPrepareSignalData(&signalData, cantx_signalOpenWirePeriod);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_OPEN_WIRE_PERIOD_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_STANDBY_OPEN_WIRE_PERIOD_LENGTH,
        (uint64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set normal periodic open wire check flag */
    convertedSignalData = CAN_ConvertBooleanToInteger(BS_NORMAL_PERIODIC_OPEN_WIRE_CHECK);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_PERIODIC_OPEN_WIRE_CHECK_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_PERIODIC_OPEN_WIRE_CHECK_LENGTH,
        convertedSignalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set normal open wire period value */
    signalData = (float_t)BS_NORMAL_OPEN_WIRE_PERIOD_ms;
    CAN_TxPrepareSignalData(&signalData, cantx_signalOpenWirePeriod);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_OPEN_WIRE_PERIOD_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_NORMAL_OPEN_WIRE_PERIOD_LENGTH,
        (uint64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set charge periodic open wire check flag */
    convertedSignalData = CAN_ConvertBooleanToInteger(BS_CHARGE_PERIODIC_OPEN_WIRE_CHECK);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_PERIODIC_OPEN_WIRE_CHECK_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_PERIODIC_OPEN_WIRE_CHECK_LENGTH,
        convertedSignalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set charge open wire period value */
    signalData = (float_t)BS_CHARGE_OPEN_WIRE_PERIOD_ms;
    CAN_TxPrepareSignalData(&signalData, cantx_signalOpenWirePeriod);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_OPEN_WIRE_PERIOD_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_CHARGE_OPEN_WIRE_PERIOD_LENGTH,
        (uint64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set error open wire period value */
    signalData = (float_t)BS_ERROR_OPEN_WIRE_PERIOD_ms;
    CAN_TxPrepareSignalData(&signalData, cantx_signalOpenWirePeriod);
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_ERROR_OPEN_WIRE_PERIOD_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_OPEN_WIRE_CHECK_ERROR_OPEN_WIRE_PERIOD_LENGTH,
        (uint64_t)signalData,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static void CANTX_SetBatterySystemMuxTotalNumbersMessageData(uint64_t *pMessageData) {
    FAS_ASSERT(pMessageData != NULL_PTR);

    /* Set multiplexer value */
    CANTX_SetDebugBuildConfigurationMux(pMessageData, CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS);

    /* Set number of cell blocks per string value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_PER_STRING_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_PER_STRING_LENGTH,
        BS_NR_OF_CELL_BLOCKS_PER_STRING,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of cell blocks value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_CELL_BLOCKS_LENGTH,
        BS_NR_OF_CELL_BLOCKS,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of temperature sensors per string value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_PER_STRING_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_PER_STRING_LENGTH,
        BS_NR_OF_TEMP_SENSORS_PER_STRING,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);

    /* Set number of temperature sensors value */
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_START_BIT,
        CANTX_MUX_BATTERY_SYSTEM_TOTAL_NUMBERS_NR_OF_TEMP_SENSORS_LENGTH,
        BS_NR_OF_TEMP_SENSORS,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e CANTX_DebugBuildConfiguration(void) {
    /* Trigger the application configuration message */
    STD_RETURN_TYPE_e queuedSuccessfully = CANTX_SendApplicationConfiguration();
    if (queuedSuccessfully == STD_OK) {
        /* Trigger the slave configuration message */
        queuedSuccessfully = CANTX_SendSlaveConfiguration();
    }

    if (queuedSuccessfully == STD_OK) {
        /* Trigger battery cell configuration messages */
        queuedSuccessfully = CANTX_SendBatteryCellConfiguration();
    }

    if (queuedSuccessfully == STD_OK) {
        /* Trigger battery system configuration messages */
        queuedSuccessfully = CANTX_SendBatterySystemConfiguration();
    }
    return queuedSuccessfully;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANTX_SetDebugBuildConfigurationMux(uint64_t *pMessageData, uint8_t muxValue) {
    CANTX_SetDebugBuildConfigurationMux(pMessageData, muxValue);
}

extern STD_RETURN_TYPE_e TEST_CANTX_SendApplicationConfiguration(void) {
    return CANTX_SendApplicationConfiguration();
}

extern STD_RETURN_TYPE_e TEST_CANTX_SendSlaveConfiguration(void) {
    return CANTX_SendSlaveConfiguration();
}

extern STD_RETURN_TYPE_e TEST_CANTX_SendBatteryCellConfiguration(void) {
    return CANTX_SendBatteryCellConfiguration();
}

extern void TEST_CANTX_SetBatteryCellMuxMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMaxChargeCurrentMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMaxChargeTemperatureMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMaxDischargeCurrentMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMaxDischargeTemperatureMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMaxVoltageMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMaxVoltageMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMinChargeTemperatureMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMinDischargeTemperatureMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatteryCellMuxMinVoltageMessageData(uint64_t *pMessageData) {
    CANTX_SetBatteryCellMuxMinVoltageMessageData(pMessageData);
}

extern STD_RETURN_TYPE_e TEST_CANTX_SendBatterySystemConfiguration(void) {
    return CANTX_SendBatterySystemConfiguration();
}

extern void TEST_CANTX_SetBatterySystemMuxGeneral1MessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxGeneral1MessageData(pMessageData);
}

extern void TEST_CANTX_SetBatterySystemMuxGeneral2MessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxGeneral2MessageData(pMessageData);
}

extern void TEST_CANTX_SetBatterySystemMuxContactorsMessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxContactorsMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatterySystemMuxCurrentSensorMessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxCurrentSensorMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatterySystemMuxFuseMessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxFuseMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatterySystemMuxMaxCurrentMessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxMaxCurrentMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatterySystemMuxOpenWireCheckMessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxOpenWireCheckMessageData(pMessageData);
}

extern void TEST_CANTX_SetBatterySystemMuxTotalNumbersMessageData(uint64_t *pMessageData) {
    CANTX_SetBatterySystemMuxTotalNumbersMessageData(pMessageData);
}
#endif
