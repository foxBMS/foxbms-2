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
 * @file    database_cfg.c
 * @author  foxBMS Team
 * @date    2015-08-18 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  DATA
 *
 * @brief   Database configuration
 * @details Configuration of database module
 */

/*========== Includes =======================================================*/
#include "database_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/** data block: cell voltage */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s data_blockCellVoltage     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_VOLTAGE_s data_blockCellVoltageBase = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s data_blockCellVoltageRedundancy0 = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
/**@}*/

/** data block: cell temperature */
/**@{*/
static DATA_BLOCK_CELL_TEMPERATURE_s data_blockCellTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_CELL_TEMPERATURE_s data_blockCellTemperatureBase = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s data_blockCellTemperatureRedundancy0 = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
/**@}*/

/** data block: minimum and maximum values */
static DATA_BLOCK_MIN_MAX_s data_blockMinMax = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};

/** data block: current sensor */
static DATA_BLOCK_CURRENT_SENSOR_s data_blockCurrentSensor = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};

/** data block: balancing control */
static DATA_BLOCK_BALANCING_CONTROL_s data_blockControlBalancing = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

/** data block: slave control */
static DATA_BLOCK_SLAVE_CONTROL_s data_blockSlaveControl = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};

/** data block: balancing feedback */
/**@{*/
static DATA_BLOCK_BALANCING_FEEDBACK_s data_blockFeedbackBalancingBase = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s data_blockFeedbackBalancingRedundancy0 = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_REDUNDANCY0};
/**@}*/

/** data block: open wire check */
/**@{*/
static DATA_BLOCK_OPEN_WIRE_s data_blockOpenWireBase        = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_OPEN_WIRE_s data_blockOpenWireRedundancy0 = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_REDUNDANCY0};
/**@}*/

/** data block: AFE GPIO voltages */
/**@{*/
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s data_blockAllGpioVoltagesBase = {
    .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s data_blockAllGpioVoltagesRedundancy0 = {
    .header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_REDUNDANCY0};
/**@}*/

/** data block: error flags */
static DATA_BLOCK_ERROR_STATE_s data_blockErrors = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};

/** data block: contactor feedback */
static DATA_BLOCK_CONTACTOR_FEEDBACK_s data_blockContactorFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_CONTACTOR_FEEDBACK};

/** data block: interlock feedback */
static DATA_BLOCK_INTERLOCK_FEEDBACK_s data_blockInterlockFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_INTERLOCK_FEEDBACK};

/** data block: sof */
static DATA_BLOCK_SOF_s data_blockSof = {.header.uniqueId = DATA_BLOCK_ID_SOF};

/** data block: system state */
static DATA_BLOCK_SYSTEM_STATE_s data_blockSystemState = {.header.uniqueId = DATA_BLOCK_ID_SYSTEM_STATE};

/** data block: maximum safety limit violations */
static DATA_BLOCK_MSL_FLAG_s data_blockMsl = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};

/** data block: recommended safety limit violations */
static DATA_BLOCK_RSL_FLAG_s data_blockRsl = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};

/** data block: maximum operating limit violations */
static DATA_BLOCK_MOL_FLAG_s data_blockMol = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};

/** data block: soc */
static DATA_BLOCK_SOC_s data_blockSoc = {.header.uniqueId = DATA_BLOCK_ID_SOC};

/** data block: soh */
static DATA_BLOCK_SOH_s data_blockSoh = {.header.uniqueId = DATA_BLOCK_ID_SOH};

/** data block: soe */
static DATA_BLOCK_SOE_s data_blockSoe = {.header.uniqueId = DATA_BLOCK_ID_SOE};

/** data block: can state request */
static DATA_BLOCK_STATE_REQUEST_s data_blockStateRequest = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

/** data block: moving average */
static DATA_BLOCK_MOVING_AVERAGE_s data_blockMovingAverage = {.header.uniqueId = DATA_BLOCK_ID_MOVING_AVERAGE};

/** data block: insulation monitoring */
static DATA_BLOCK_INSULATION_MONITORING_s data_blockInsulationMonitoring = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};

/** data b  lock: pack values */
static DATA_BLOCK_PACK_VALUES_s data_blockPackValues = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};

/** data block: adc temperature */
static DATA_BLOCK_ADC_VOLTAGE_s data_blockAdcVoltage = {.header.uniqueId = DATA_BLOCK_ID_ADC_VOLTAGE};

/** data block: I2C humidity/temperature sensor measurements */
static DATA_BLOCK_HTSEN_s data_blockHumidityTemperatureSensor = {.header.uniqueId = DATA_BLOCK_ID_HTSEN};

/** data block: used for self-test */
static DATA_BLOCK_DUMMY_FOR_SELF_TEST_s data_blockDummyForSelfTest = {
    .header.uniqueId = DATA_BLOCK_ID_DUMMY_FOR_SELF_TEST};

/** data block: aerosol sensor */
static DATA_BLOCK_AEROSOL_SENSOR_s data_blockAerosolSensor = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};

/** data block: aerosol sensor */
static DATA_BLOCK_PHY_s data_blockPhy = {.header.uniqueId = DATA_BLOCK_ID_PHY};

/**
 * @brief   channel configuration of database (data blocks)
 * @details all data block managed by database are listed here (address, size,
 *          consistency type)
 */
DATA_BASE_s data_database[] = {
    {(void *)(&data_blockCellVoltage), sizeof(DATA_BLOCK_CELL_VOLTAGE_s)},
    {(void *)(&data_blockCellTemperature), sizeof(DATA_BLOCK_CELL_TEMPERATURE_s)},
    {(void *)(&data_blockMinMax), sizeof(DATA_BLOCK_MIN_MAX_s)},
    {(void *)(&data_blockCurrentSensor), sizeof(DATA_BLOCK_CURRENT_SENSOR_s)},
    {(void *)(&data_blockControlBalancing), sizeof(DATA_BLOCK_BALANCING_CONTROL_s)},
    {(void *)(&data_blockSlaveControl), sizeof(DATA_BLOCK_SLAVE_CONTROL_s)},
    {(void *)(&data_blockFeedbackBalancingBase), sizeof(DATA_BLOCK_BALANCING_FEEDBACK_s)},
    {(void *)(&data_blockOpenWireBase), sizeof(DATA_BLOCK_OPEN_WIRE_s)},
    {(void *)(&data_blockAllGpioVoltagesBase), sizeof(DATA_BLOCK_ALL_GPIO_VOLTAGES_s)},
    {(void *)(&data_blockErrors), sizeof(DATA_BLOCK_ERROR_STATE_s)},
    {(void *)(&data_blockContactorFeedback), sizeof(DATA_BLOCK_CONTACTOR_FEEDBACK_s)},
    {(void *)(&data_blockInterlockFeedback), sizeof(DATA_BLOCK_INTERLOCK_FEEDBACK_s)},
    {(void *)(&data_blockSof), sizeof(DATA_BLOCK_SOF_s)},
    {(void *)(&data_blockSystemState), sizeof(DATA_BLOCK_SYSTEM_STATE_s)},
    {(void *)(&data_blockMsl), sizeof(DATA_BLOCK_MSL_FLAG_s)},
    {(void *)(&data_blockRsl), sizeof(DATA_BLOCK_RSL_FLAG_s)},
    {(void *)(&data_blockMol), sizeof(DATA_BLOCK_MOL_FLAG_s)},
    {(void *)(&data_blockSoc), sizeof(DATA_BLOCK_SOC_s)},
    {(void *)(&data_blockSoh), sizeof(DATA_BLOCK_SOH_s)},
    {(void *)(&data_blockSoe), sizeof(DATA_BLOCK_SOE_s)},
    {(void *)(&data_blockStateRequest), sizeof(DATA_BLOCK_STATE_REQUEST_s)},
    {(void *)(&data_blockMovingAverage), sizeof(DATA_BLOCK_MOVING_AVERAGE_s)},
    {(void *)(&data_blockCellVoltageBase), sizeof(DATA_BLOCK_CELL_VOLTAGE_s)},
    {(void *)(&data_blockCellTemperatureBase), sizeof(DATA_BLOCK_CELL_TEMPERATURE_s)},
    {(void *)(&data_blockCellVoltageRedundancy0), sizeof(DATA_BLOCK_CELL_VOLTAGE_s)},
    {(void *)(&data_blockCellTemperatureRedundancy0), sizeof(DATA_BLOCK_CELL_TEMPERATURE_s)},
    {(void *)(&data_blockFeedbackBalancingRedundancy0), sizeof(DATA_BLOCK_BALANCING_FEEDBACK_s)},
    {(void *)(&data_blockAllGpioVoltagesRedundancy0), sizeof(DATA_BLOCK_ALL_GPIO_VOLTAGES_s)},
    {(void *)(&data_blockOpenWireRedundancy0), sizeof(DATA_BLOCK_OPEN_WIRE_s)},
    {(void *)(&data_blockInsulationMonitoring), sizeof(DATA_BLOCK_INSULATION_MONITORING_s)},
    {(void *)(&data_blockPackValues), sizeof(DATA_BLOCK_PACK_VALUES_s)},
    {(void *)(&data_blockAdcVoltage), sizeof(DATA_BLOCK_ADC_VOLTAGE_s)},
    {(void *)(&data_blockHumidityTemperatureSensor), sizeof(DATA_BLOCK_HTSEN_s)},
    {(void *)(&data_blockDummyForSelfTest), sizeof(DATA_BLOCK_DUMMY_FOR_SELF_TEST_s)},
    {(void *)(&data_blockAerosolSensor), sizeof(DATA_BLOCK_AEROSOL_SENSOR_s)},
    {(void *)(&data_blockPhy), sizeof(DATA_BLOCK_PHY_s)},
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
