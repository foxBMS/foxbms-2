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
 * @file    database_cfg.h
 * @author  foxBMS Team
 * @date    2015-08-18 (date of creation)
 * @updated 2021-09-30 (date of last update)
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  DATA
 *
 * @brief   Database configuration header
 *
 * Provides interfaces to database configuration
 *
 */

#ifndef FOXBMS__DATABASE_CFG_H_
#define FOXBMS__DATABASE_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

/*========== Macros and Definitions =========================================*/
/** configuration struct of database channel (data block) */
typedef struct DATA_BASE {
    void *pDatabaseEntry; /*!< pointer to the database entry */
    uint32_t datalength;  /*!< length of the entry */
} DATA_BASE_s;

/** data block identification numbers */
typedef enum DATA_BLOCK_ID {
    DATA_BLOCK_ID_CELL_VOLTAGE,
    DATA_BLOCK_ID_CELL_TEMPERATURE,
    DATA_BLOCK_ID_MIN_MAX,
    DATA_BLOCK_ID_CURRENT_SENSOR,
    DATA_BLOCK_ID_BALANCING_CONTROL,
    DATA_BLOCK_ID_SLAVE_CONTROL,
    DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE,
    DATA_BLOCK_ID_USER_MUX,
    DATA_BLOCK_ID_OPEN_WIRE_BASE,
    DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE,
    DATA_BLOCK_ID_ERRORSTATE,
    DATA_BLOCK_ID_CONTACTOR_FEEDBACK,
    DATA_BLOCK_ID_INTERLOCK_FEEDBACK,
    DATA_BLOCK_ID_SOF,
    DATA_BLOCK_ID_SYSTEMSTATE,
    DATA_BLOCK_ID_MSL_FLAG,
    DATA_BLOCK_ID_RSL_FLAG,
    DATA_BLOCK_ID_MOL_FLAG,
    DATA_BLOCK_ID_SOX,
    DATA_BLOCK_ID_STATEREQUEST,
    DATA_BLOCK_ID_MOVING_AVERAGE,
    DATA_BLOCK_ID_CELL_VOLTAGE_BASE,
    DATA_BLOCK_ID_CELL_TEMPERATURE_BASE,
    DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0,
    DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0,
    DATA_BLOCK_ID_BALANCING_FEEDBACK_REDUNDANCY0,
    DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_REDUNDANCY0,
    DATA_BLOCK_ID_OPEN_WIRE_REDUNDANCY0,
    DATA_BLOCK_ID_INSULATION_MONITORING,
    DATA_BLOCK_ID_PACK_VALUES,
    DATA_BLOCK_ID_HTSEN,
    DATA_BLOCK_ID_ADC_VOLTAGE,
    DATA_BLOCK_ID_DUMMY_FOR_SELF_TEST,
    DATA_BLOCK_ID_MAX, /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} DATA_BLOCK_ID_e;

/** data block header */
typedef struct DATA_BLOCKHEADER {
    DATA_BLOCK_ID_e uniqueId;   /*!< uniqueId of database entry */
    uint32_t timestamp;         /*!< timestamp of last database update */
    uint32_t previousTimestamp; /*!< timestamp of previous database update */
} DATA_BLOCK_HEADER_s;

/** data block struct of cell voltage */
typedef struct DATA_BLOCK_CELL_VOLTAGE {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                   /*!< Data block header */
    uint8_t state;                                                /*!< for future use */
    int32_t packVoltage_mV[BS_NR_OF_STRINGS];                     /*!< uint: mV */
    int16_t cellVoltage_mV[BS_NR_OF_STRINGS][BS_NR_OF_BAT_CELLS]; /*!< unit: mV */
    uint64_t invalidCellVoltage[BS_NR_OF_STRINGS]
                               [BS_NR_OF_MODULES];  /*!< bitmask if voltages are valid. 0->valid, 1->invalid */
    uint16_t nrValidCellVoltages[BS_NR_OF_STRINGS]; /*!< number of valid voltages */
    uint32_t moduleVoltage_mV[BS_NR_OF_STRINGS][BS_NR_OF_MODULES]; /*!< unit: mV */
    bool validModuleVoltage[BS_NR_OF_STRINGS][BS_NR_OF_MODULES];   /*!< 0 -> if PEC okay; 1 -> PEC error */
} DATA_BLOCK_CELL_VOLTAGE_s;

/** data block struct of cell temperatures */
typedef struct DATA_BLOCK_CELL_TEMPERATURE {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                                        /*!< Data block header */
    uint8_t state;                                                                     /*!< for future use */
    int16_t cellTemperature_ddegC[BS_NR_OF_STRINGS][BS_NR_OF_TEMP_SENSORS_PER_STRING]; /*!< unit: deci &deg;C */
    uint16_t invalidCellTemperature[BS_NR_OF_STRINGS]
                                   [BS_NR_OF_MODULES]; /*!< bitmask if temperatures are valid. 0->valid, 1->invalid */
    uint16_t nrValidTemperatures[BS_NR_OF_STRINGS];    /*!< number of valid temperatures in each string */
} DATA_BLOCK_CELL_TEMPERATURE_s;

/** data block struct of minimum and maximum values */
typedef struct DATA_BLOCK_MIN_MAX {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */

    int16_t averageCellVoltage_mV[BS_NR_OF_STRINGS];          /*!< average cell voltages, unit: mV */
    int16_t minimumCellVoltage_mV[BS_NR_OF_STRINGS];          /*!< minimum cell voltages, unit: mV */
    int16_t previousMinimumCellVoltage_mV[BS_NR_OF_STRINGS];  /*!< previous minimum cell voltages, unit: mV */
    int16_t maximumCellVoltage_mV[BS_NR_OF_STRINGS];          /*!< maximum cell voltages, unit: mV */
    int16_t previousMaximumCellVoltage_mV[BS_NR_OF_STRINGS];  /*!< previous maximum cell voltages, unit: mV */
    uint16_t nrModuleMinimumCellVoltage[BS_NR_OF_STRINGS];    /*!< number of the module with minimum cell voltage */
    uint16_t nrCellMinimumCellVoltage[BS_NR_OF_STRINGS];      /*!< number of the cell with minimum cell voltage */
    uint16_t nrModuleMaximumCellVoltage[BS_NR_OF_STRINGS];    /*!< number of the module with maximum cell voltage */
    uint16_t nrCellMaximumCellVoltage[BS_NR_OF_STRINGS];      /*!< number of the cell with maximum cell voltage */
    uint16_t validMeasuredCellVoltages[BS_NR_OF_STRINGS];     /*!< number of valid measured cell voltages */
    float averageTemperature_ddegC[BS_NR_OF_STRINGS];         /*!< unit: deci &deg;C */
    int16_t minimumTemperature_ddegC[BS_NR_OF_STRINGS];       /*!< unit: deci &deg;C */
    uint16_t nrModuleMinimumTemperature[BS_NR_OF_STRINGS];    /*!< number of the module with minimum temperature */
    uint16_t nrSensorMinimumTemperature[BS_NR_OF_STRINGS];    /*!< number of the sensor with minimum temperature */
    int16_t maximumTemperature_ddegC[BS_NR_OF_STRINGS];       /*!< unit: deci &deg;C */
    uint16_t nrModuleMaximumTemperature[BS_NR_OF_STRINGS];    /*!< number of the module with maximum temperature */
    uint16_t nrSensorMaximumTemperature[BS_NR_OF_STRINGS];    /*!< number of the sensor with maximum temperature */
    uint16_t validMeasuredCellTemperatures[BS_NR_OF_STRINGS]; /*!< number of valid measured cell temperatures */
    uint8_t state;                                            /*!< state of the min max module */
} DATA_BLOCK_MIN_MAX_s;

/** data block struct of pack measurement values */
typedef struct DATA_BLOCK_PACK_VALUES {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */

    int32_t packCurrent_mA;        /*!< current in the whole battery pack, unit: mA */
    uint8_t invalidPackCurrent;    /*!< bitmask if current is valid. 0->valid, 1->invalid */
    int32_t batteryVoltage_mV;     /*!< voltage between negative and positive battery pole, unit: mV */
    uint8_t invalidBatteryVoltage; /*!< bitmask if voltage is valid. 0->valid, 1->invalid */
    int32_t
        highVoltageBusVoltage_mV; /*!< voltage between negative battery pole and after positive main contactor, unit: mV */
    uint8_t invalidHvBusVoltage; /*!< bitmask if voltage is valid. 0->valid, 1->invalid */
    int32_t packPower_W;         /*!< power provided by respectively supplied to the battery pack, unit: W */
    uint8_t invalidPackPower;    /*!< bitmask if power is valid. 0->valid, 1->invalid */
    int32_t stringVoltage_mV[BS_NR_OF_STRINGS];     /*!< voltage of each string, unit: mV */
    uint8_t invalidStringVoltage[BS_NR_OF_STRINGS]; /*!< bitmask if voltages are valid. 0->valid, 1->invalid */
    int32_t stringCurrent_mA[BS_NR_OF_STRINGS];     /*!< current in each string, unit: mA */
    uint8_t invalidStringCurrent[BS_NR_OF_STRINGS]; /*!< bitmask if currents are valid. 0->valid, 1->invalid */
    int32_t stringPower_W[BS_NR_OF_STRINGS];        /*!< power of each string, unit: W */
    uint8_t invalidStringPower[BS_NR_OF_STRINGS];   /*!< bitmask if power values are valid. 0->valid, 1->invalid */
} DATA_BLOCK_PACK_VALUES_s;

/** data block struct of current measurement */
typedef struct DATA_BLOCK_CURRENT_SENSOR {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                    /*!< Data block header */
    int32_t current_mA[BS_NR_OF_STRINGS];                          /*!< unit: mA */
    uint8_t invalidCurrentMeasurement[BS_NR_OF_STRINGS];           /*!< 0: measurement valid, 1: measurement invalid */
    uint8_t newCurrent;                                            /*!< 0: measurement valid, 1: measurement invalid */
    uint32_t previousTimestampCurrent[BS_NR_OF_STRINGS];           /*!< timestamp of current measurement */
    uint32_t timestampCurrent[BS_NR_OF_STRINGS];                   /*!< timestamp of current measurement */
    int32_t sensorTemperature_ddegC[BS_NR_OF_STRINGS];             /*!< unit: 0.1&deg;C */
    uint8_t invalidSensorTemperatureMeasurement[BS_NR_OF_STRINGS]; /*!< 0: measurement valid, 1: measurement invalid */
    int32_t power_W[BS_NR_OF_STRINGS];                             /*!< unit: W */
    uint8_t invalidPowerMeasurement[BS_NR_OF_STRINGS];             /*!< 0: measurement valid, 1: measurement invalid */
    uint8_t newPower;                                            /*!< counter that indicates a new power measurement */
    uint32_t previousTimestampPower[BS_NR_OF_STRINGS];           /*!< previous timestamp of power measurement */
    uint32_t timestampPower[BS_NR_OF_STRINGS];                   /*!< timestamp of power measurement */
    int32_t currentCounter_As[BS_NR_OF_STRINGS];                 /*!< unit: A.s */
    uint8_t invalidCurrentCountingMeasurement[BS_NR_OF_STRINGS]; /*!< 0: measurement valid, 1: measurement invalid */
    uint32_t previousTimestampCurrentCounting[BS_NR_OF_STRINGS]; /*!< previous timestamp of CC measurement */
    uint32_t timestampCurrentCounting[BS_NR_OF_STRINGS];         /*!< timestamp of CC measurement */
    int32_t energyCounter_Wh[BS_NR_OF_STRINGS];                  /*!< unit: Wh */
    uint8_t invalidEnergyCountingMeasurement[BS_NR_OF_STRINGS];  /*!< 0: measurement valid, 1: measurement invalid */
    uint32_t previousTimestampEnergyCounting[BS_NR_OF_STRINGS];  /*!< previous timestamp of EC measurement */
    uint32_t timestampEnergyCounting[BS_NR_OF_STRINGS];          /*!< timestamp of EC measurement */
    uint8_t invalidHighVoltageMeasurement
        [BS_NR_OF_STRINGS][BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR]; /*!< 0: measurement valid, 1: measurement invalid */
    int32_t highVoltage_mV[BS_NR_OF_STRINGS][BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR]; /*!< unit: mV */
    uint32_t previousTimestampHighVoltage
        [BS_NR_OF_STRINGS]
        [BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR]; /*!< previous timestamp of high voltage measurement */
    uint32_t timestampHighVoltage[BS_NR_OF_STRINGS]
                                 [BS_NR_OF_VOLTAGES_FROM_CURRENT_SENSOR]; /*!< timestamp of high voltage measurement */
} DATA_BLOCK_CURRENT_SENSOR_s;

/** data structure declaration of DATA_BLOCK_BALANCING_CONTROL */
typedef struct DATA_BLOCK_BALANCING_CONTROL {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                     /*!< Data block header */
    uint8_t enableBalancing;                                        /*!< Switch for enabling/disabling balancing  */
    uint8_t threshold_mV;                                           /*!< balancing threshold in mV                */
    uint8_t request;                                                /*!< balancing request per CAN                */
    uint8_t balancingState[BS_NR_OF_STRINGS][BS_NR_OF_BAT_CELLS];   /*!< 0: no balancing, 1: balancing active     */
    uint32_t deltaCharge_mAs[BS_NR_OF_STRINGS][BS_NR_OF_BAT_CELLS]; /*!< Difference in Depth-of-Discharge in mAs  */
    uint16_t nrBalancedCells[BS_NR_OF_STRINGS];
} DATA_BLOCK_BALANCING_CONTROL_s;

/** data structure declaration of DATA_BLOCK_USER_IO_CONTROL */
typedef struct DATA_BLOCK_SLAVE_CONTROL {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                          /*!< Data block header */
    uint8_t state;                                       /*!< for future use */
    uint32_t eepromReadAddressToUse;                     /*!< address to read from for  slave EEPROM */
    uint32_t eepromReadAddressLastUsed;                  /*!< last address used to read fromfor slave EEPROM */
    uint32_t eepromWriteAddressToUse;                    /*!< address to write to for slave EEPROM */
    uint32_t eepromWriteAddressLastUsed;                 /*!< last address used to write to for slave EEPROM */
    uint8_t ioValueOut[BS_NR_OF_MODULES];                /*!< data to be written to the port expander */
    uint8_t ioValueIn[BS_NR_OF_MODULES];                 /*!< data read from to the port expander */
    uint8_t eepromValueWrite[BS_NR_OF_MODULES];          /*!< data to be written to the slave EEPROM */
    uint8_t eepromValueRead[BS_NR_OF_MODULES];           /*!< data read from to the slave EEPROM */
    uint8_t externalTemperatureSensor[BS_NR_OF_MODULES]; /*!< temperature from the external sensor on slave */
} DATA_BLOCK_SLAVE_CONTROL_s;

/** data block struct of cell balancing feedback */
typedef struct DATA_BLOCK_BALANCING_FEEDBACK {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                         /*!< Data block header */
    uint8_t state;                                      /*!< for future use */
    uint16_t value[BS_NR_OF_STRINGS][BS_NR_OF_MODULES]; /*!< unit: mV (optocoupler output) */
} DATA_BLOCK_BALANCING_FEEDBACK_s;

/** data block struct of user multiplexer values */
typedef struct DATA_BLOCK_USER_MUX {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                   /*!< Data block header */
    uint8_t state;                                                /*!< for future use */
    uint16_t value[BS_NR_OF_STRINGS][8u * 2u * BS_NR_OF_MODULES]; /*!< unit: mV (mux voltage input) */
} DATA_BLOCK_USER_MUX_s;

/** data block struct of cell open wire */
typedef struct DATA_BLOCK_OPENWIRE {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;             /*!< Data block header */
    uint8_t state;                          /*!< for future use */
    uint16_t nrOpenWires[BS_NR_OF_STRINGS]; /*!< number of open wires */
    uint8_t openwire[BS_NR_OF_STRINGS]
                    [BS_NR_OF_MODULES * (BS_NR_OF_CELLS_PER_MODULE + 1u)]; /*!< 1 -> open wire, 0 -> everything ok */
} DATA_BLOCK_OPEN_WIRE_s;

/** data block struct of GPIO voltage */
typedef struct DATA_BLOCK_ALL_GPIO_VOLTAGES {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                                               /*!< Data block header */
    uint8_t state;                                                                            /*!< for future use */
    uint16_t gpioVoltages_mV[BS_NR_OF_STRINGS][BS_NR_OF_MODULES * BS_NR_OF_GPIOS_PER_MODULE]; /*!< unit: mV */
    uint16_t invalidGpioVoltages[BS_NR_OF_STRINGS]
                                [BS_NR_OF_MODULES]; /*!< bitmask if voltages are valid. 0->valid, 1->invalid */
} DATA_BLOCK_ALL_GPIO_VOLTAGES_s;

/** data block struct of error flags */
typedef struct DATA_BLOCK_ERRORSTATE {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                       /*!< Data block header */
    uint8_t currentSensor[BS_NR_OF_STRINGS];                          /*!< 0 -> no error, 1 -> error, not responding */
    uint8_t stringContactor[BS_NR_OF_STRINGS];                        /*!< 0 -> no error, 1 -> error, not responding */
    uint8_t prechargeContactor[BS_NR_OF_STRINGS];                     /*!< 0 -> no error, 1 -> error, not responding */
    uint8_t interlock;                                                /*!< 0 -> no error, 1 -> error */
    uint8_t crcError[BS_NR_OF_STRINGS];                               /*!< 0 -> no error, 1 -> error */
    uint8_t muxError[BS_NR_OF_STRINGS];                               /*!< 0 -> no error, 1 -> error */
    uint8_t spiError[BS_NR_OF_STRINGS];                               /*!< 0 -> no error, 1 -> error */
    uint8_t afeConfigurationError[BS_NR_OF_STRINGS];                  /*!< 0 -> no error, 1 -> error */
    uint8_t afeCellvoltageError[BS_NR_OF_STRINGS];                    /*!< 0 -> no error, 1 -> error */
    uint8_t afeCellTemperatureError[BS_NR_OF_STRINGS];                /*!< 0 -> no error, 1 -> error */
    uint8_t baseCellVoltageMeasurementTimeout;                        /*!< 0 -> no error, 1 -> error */
    uint8_t redundancy0CellVoltageMeasurementTimeout;                 /*!< 0 -> no error, 1 -> error */
    uint8_t baseCellTemperatureMeasurementTimeout;                    /*!< 0 -> no error, 1 -> error */
    uint8_t redundancy0CellTemperatureMeasurementTimeout;             /*!< 0 -> no error, 1 -> error */
    uint8_t currentMeasurementTimeout[BS_NR_OF_STRINGS];              /*!< 0 -> no error, 1 -> error */
    uint8_t currentMeasurementError[BS_NR_OF_STRINGS];                /*!< 0 -> no error, 1 -> error */
    uint8_t currentSensorTimeoutV1[BS_NR_OF_STRINGS];                 /*!< 0 -> no error, 1 -> error */
    uint8_t currentSensorTimeoutV3[BS_NR_OF_STRINGS];                 /*!< 0 -> no error, 1 -> error */
    uint8_t currentSensorPowerTimeout[BS_NR_OF_STRINGS];              /*!< 0 -> no error, 1 -> error */
    uint8_t powerMeasurementError[BS_NR_OF_STRINGS];                  /*!< 0 -> no error, 1 -> error */
    uint8_t insulationError;                                          /*!< 0 -> no error, 1 -> error */
    uint8_t fuseStateNormal[BS_NR_OF_STRINGS];                        /*!< 0 -> fuse ok,  1 -> fuse tripped */
    uint8_t fuseStateCharge[BS_NR_OF_STRINGS];                        /*!< 0 -> fuse ok,  1 -> fuse tripped */
    uint8_t open_wire[BS_NR_OF_STRINGS];                              /*!< 0 -> no error, 1 -> error */
    uint8_t canTiming;                                                /*!< 0 -> no error, 1 -> error */
    uint8_t canRxQueueFull;                                           /*!< 0 -> no error, 1 -> error */
    uint8_t canTimingCc[BS_NR_OF_STRINGS];                            /*!< 0 -> no error, 1 -> error */
    uint8_t canTimingEc[BS_NR_OF_STRINGS];                            /*!< 0 -> no error, 1 -> error */
    uint8_t mcuDieTemperature;                                        /*!< 0 -> no error, 1 -> error */
    uint8_t coinCellVoltage;                                          /*!< 0 -> no error, 1 -> error */
    uint8_t plausibilityCheckPackvoltage[BS_NR_OF_STRINGS];           /*!< 0 -> no error, else: error */
    uint8_t plausibilityCheckCellvoltage[BS_NR_OF_STRINGS];           /*!< 0 -> no error, else: error */
    uint8_t plausibilityCheckCellvoltageSpread[BS_NR_OF_STRINGS];     /*!< 0 -> no error, else: error */
    uint8_t plausibilityCheckCelltemperatureSpread[BS_NR_OF_STRINGS]; /*!< 0 -> no error, 1 -> error */
    uint8_t plausibilityCheckCelltemperature[BS_NR_OF_STRINGS];       /*!< 0 -> no error, else: error */
    uint8_t deepDischargeDetected[BS_NR_OF_STRINGS];                  /*!< 0 -> no error, 1 -> error */
    uint8_t currentOnOpenString[BS_NR_OF_STRINGS];                    /*!< 0 -> no error, 1 -> error */
    uint8_t sbcFinState;  /*!< 0 -> okay, 1 -> error: short-circuit to RSTB */
    uint8_t sbcRstbState; /*!< 0 -> okay, 1 -> error: RSTB not working */
    uint8_t i2cPexError;  /*!< the I2C port expander does not work as expected */
} DATA_BLOCK_ERRORSTATE_s;

/** data block struct of contactor feedback */
typedef struct DATA_BLOCK_CONTFEEDBACK {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    uint32_t contactorFeedback; /*!< feedback of all contactors, without interlock */
} DATA_BLOCK_CONTACTOR_FEEDBACK_s;

/** data block struct of interlock feedback */
typedef struct DATA_BLOCK_INTERLOCK_FEEDBACK {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                 /*!< Data block header */
    uint8_t interlockFeedback_IL_STATE;         /*!< feedback of interlock, connected to pin */
    float interlockVoltageFeedback_IL_HS_VS_mV; /*!< voltage feedback of interlock, connected to ADC input 2 */
    float interlockVoltageFeedback_IL_LS_VS_mV; /*!< voltage feedback of interlock, connected to ADC input 3 */
    float interlockCurrentFeedback_IL_HS_CS_mA; /*!< current feedback of interlock, connected to ADC input 4 */
    float interlockCurrentFeedback_IL_LS_CS_mA; /*!< current feedback of interlock, connected to ADC input 5 */
} DATA_BLOCK_INTERLOCK_FEEDBACK_s;

/** data block struct of sof limits */
typedef struct DATA_BLOCK_SOF {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                         /*!< Data block header */
    float recommendedContinuousPackChargeCurrent_mA;    /*!< recommended continuous operating pack charge current */
    float recommendedContinuousPackDischargeCurrent_mA; /*!< recommended continuous operating pack discharge current */
    float recommendedPeakPackChargeCurrent_mA;          /*!< recommended peak operating pack charge current */
    float recommendedPeakPackDischargeCurrent_mA;       /*!< recommended peak operating pack discharge current */
    float recommendedContinuousChargeCurrent_mA
        [BS_NR_OF_STRINGS]; /*!< recommended continuous operating charge current    */
    float recommendedContinuousDischargeCurrent_mA
        [BS_NR_OF_STRINGS];                                  /*!< recommended continuous operating discharge current */
    float recommendedPeakChargeCurrent_mA[BS_NR_OF_STRINGS]; /*!< recommended peak operating charge current */
    float recommendedPeakDischargeCurrent_mA[BS_NR_OF_STRINGS]; /*!< recommended peak operating discharge current */
    float continuousMolChargeCurrent_mA[BS_NR_OF_STRINGS];      /*!< charge current maximum operating level */
    float continuousMolDischargeCurrent_mA[BS_NR_OF_STRINGS];   /*!< discharge current maximum operating level */
    float peakMolChargeCurrent_mA[BS_NR_OF_STRINGS];            /*!< charge current maximum operating level */
    float peakMolDischargeCurrent_mA[BS_NR_OF_STRINGS];         /*!< discharge current maximum operating level */
    float continuousRslChargeCurrent_mA[BS_NR_OF_STRINGS];      /*!< charge current recommended safety level */
    float continuousRslDischargeCurrent_mA[BS_NR_OF_STRINGS];   /*!< discharge current recommended safety level */
    float peakRslChargeCurrent_mA[BS_NR_OF_STRINGS];            /*!< charge current recommended safety level */
    float peakRslDischargeCurrent_mA[BS_NR_OF_STRINGS];         /*!< discharge current recommended safety level */
    float continuousMslChargeCurrent_mA[BS_NR_OF_STRINGS];      /*!< charge current maximum safety level */
    float continuousMslDischargeCurrent_mA[BS_NR_OF_STRINGS];   /*!< discharge current maximum safety level */
    float peakMslChargeCurrent_mA[BS_NR_OF_STRINGS];            /*!< charge current maximum safety level */
    float peakMslDischargeCurrent_mA[BS_NR_OF_STRINGS];         /*!< discharge current maximum safety level */
} DATA_BLOCK_SOF_s;

/** data block struct of system state */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    int32_t bmsCanState;        /*!< system state for CAN messages (e.g., standby, normal) */
} DATA_BLOCK_SYSTEMSTATE_s;

/** data block struct of the maximum safe limits */
typedef struct DATA_BLOCK_MSL_FLAG {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                           /*!< Data block header */
    uint8_t packChargeOvercurrent;                        /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t packDischargeOvercurrent;                     /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t overVoltage[BS_NR_OF_STRINGS];                /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t underVoltage[BS_NR_OF_STRINGS];               /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t overtemperatureCharge[BS_NR_OF_STRINGS];      /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t overtemperatureDischarge[BS_NR_OF_STRINGS];   /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t undertemperatureCharge[BS_NR_OF_STRINGS];     /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t undertemperatureDischarge[BS_NR_OF_STRINGS];  /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t cellChargeOvercurrent[BS_NR_OF_STRINGS];      /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t stringChargeOvercurrent[BS_NR_OF_STRINGS];    /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t cellDischargeOvercurrent[BS_NR_OF_STRINGS];   /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t stringDischargeOvercurrent[BS_NR_OF_STRINGS]; /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t pcbOvertemperature[BS_NR_OF_STRINGS];         /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
    uint8_t pcbUndertemperature[BS_NR_OF_STRINGS];        /*!< 0 -> MSL NOT violated, 1 -> MSL violated */
} DATA_BLOCK_MSL_FLAG_s;

/** data block struct of the recommended safety limit */
typedef struct DATA_BLOCK_RSL_FLAG {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                           /*!< Data block header */
    uint8_t overVoltage[BS_NR_OF_STRINGS];                /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t underVoltage[BS_NR_OF_STRINGS];               /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t overtemperatureCharge[BS_NR_OF_STRINGS];      /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t overtemperatureDischarge[BS_NR_OF_STRINGS];   /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t undertemperatureCharge[BS_NR_OF_STRINGS];     /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t undertemperatureDischarge[BS_NR_OF_STRINGS];  /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t cellChargeOvercurrent[BS_NR_OF_STRINGS];      /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t stringChargeOvercurrent[BS_NR_OF_STRINGS];    /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t cellDischargeOvercurrent[BS_NR_OF_STRINGS];   /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t stringDischargeOvercurrent[BS_NR_OF_STRINGS]; /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t pcbOvertemperature[BS_NR_OF_STRINGS];         /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
    uint8_t pcbUndertemperature[BS_NR_OF_STRINGS];        /*!< 0 -> RSL NOT violated, 1 -> RSL violated */
} DATA_BLOCK_RSL_FLAG_s;

/** data block struct of the maximum operating limit */
typedef struct DATA_BLOCK_MOL_FLAG {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                           /*!< Data block header */
    uint8_t overVoltage[BS_NR_OF_STRINGS];                /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t underVoltage[BS_NR_OF_STRINGS];               /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t overtemperatureCharge[BS_NR_OF_STRINGS];      /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t overtemperatureDischarge[BS_NR_OF_STRINGS];   /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t undertemperatureCharge[BS_NR_OF_STRINGS];     /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t undertemperatureDischarge[BS_NR_OF_STRINGS];  /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t cellChargeOvercurrent[BS_NR_OF_STRINGS];      /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t stringChargeOvercurrent[BS_NR_OF_STRINGS];    /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t cellDischargeOvercurrent[BS_NR_OF_STRINGS];   /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t stringDischargeOvercurrent[BS_NR_OF_STRINGS]; /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t pcbOvertemperature[BS_NR_OF_STRINGS];         /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
    uint8_t pcbUndertemperature[BS_NR_OF_STRINGS];        /*!< 0 -> MOL NOT violated, 1 -> MOL violated */
} DATA_BLOCK_MOL_FLAG_s;

/** data block struct of sox */
typedef struct DATA_BLOCK_SOX {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;               /*!< Data block header */
    float averageSoc_perc[BS_NR_OF_STRINGS];  /*!< 0.0 <= averageSoc <= 100.0 */
    float minimumSoc_perc[BS_NR_OF_STRINGS];  /*!< 0.0 <= minSoc <= 100.0 */
    float maximumSoc_perc[BS_NR_OF_STRINGS];  /*!< 0.0 <= maxSoc <= 100.0 */
    float averageSoe_perc[BS_NR_OF_STRINGS];  /*!< 0.0 <= averageSoe <= 100.0 */
    float minimumSoe_perc[BS_NR_OF_STRINGS];  /*!< 0.0 <= minimumSoe <= 100.0  */
    float maximumSoe_perc[BS_NR_OF_STRINGS];  /*!< 0.0 <= maximumSoe <= 100.0  */
    uint32_t maximumSoe_Wh[BS_NR_OF_STRINGS]; /*!< maximum string energy in Wh */
    uint32_t averageSoe_Wh[BS_NR_OF_STRINGS]; /*!< average string energy in Wh */
    uint32_t minimumSoe_Wh[BS_NR_OF_STRINGS]; /*!< minimum string energy in Wh */
} DATA_BLOCK_SOX_s;

/** data block struct of can state request */
typedef struct DATA_BLOCK_STATEREQUEST {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;         /*!< Data block header */
    uint8_t stateRequestViaCan;         /*!< state request */
    uint8_t previousStateRequestViaCan; /*!< previous state request */
    uint8_t stateRequestViaCanPending;  /*!< pending state request */
    uint8_t state;                      /*!< state */
} DATA_BLOCK_STATEREQUEST_s;

/** data block struct of the moving average algorithm */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                        /*!< Data block header */
    float movingAverageCurrent1sInterval_mA;           /*!< current moving average over the last 1s */
    float movingAverageCurrent5sInterval_mA;           /*!< current moving average over the last 5s */
    float movingAverageCurrent10sInterval_mA;          /*!< current moving average over the last 10s */
    float movingAverageCurrent30sInterval_mA;          /*!< current moving average over the last 30s */
    float movingAverageCurrent60sInterval_mA;          /*!< current moving average over the last 60s */
    float movingAverageCurrentConfigurableInterval_mA; /*!< current moving average over the last configured time */
    float movingAveragePower1sInterval_mA;             /*!< power moving average over the last 1s */
    float movingAveragePower5sInterval_mA;             /*!< power moving average over the last 5s */
    float movingAveragePower10sInterval_mA;            /*!< power moving average over the last 10s */
    float movingAveragePower30sInterval_mA;            /*!< power moving average over the last 30s */
    float movingAveragePower60sInterval_mA;            /*!< power moving average over the last 60s */
    float movingAveragePowerConfigurableInterval_mA;   /*!< power moving average over the last configured time */
} DATA_BLOCK_MOVING_AVERAGE_s;

/** data block struct of insulation monitoring device measurement */
typedef struct DATA_BLOCK_INSULATION_MONITORING {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                /*!< Data block header */
    uint8_t valid;                             /*!< 0 -> valid, 1 -> resistance unreliable */
    uint8_t state;                             /*!< 0 -> ok , 1 -> resistance too low/error  */
    uint32_t insulationResistance_kOhm;        /*!< insulation resistance measured in kOhm */
    uint8_t insulationFault;                   /*!< 0 = no error, 1 = error */
    uint8_t insulationWarning;                 /*!< 0 = no warning, 1 = warning */
    uint8_t chassisFault;                      /*!< 0 = no error, 1 = error */
    uint8_t systemFailure;                     /*!< 0 = no error, 1 = error */
    uint8_t calibrationRunning;                /*!< 0 = not running, 1 = running */
    uint8_t selfTestRunning;                   /*!< 0 = not running, 1 = running */
    uint8_t insulationMeasurements;            /*!< 0 = Active, 1 = Inactive */
    uint8_t aliveStatusDetection;              /*!< 0 = Ok, 1 = Failure */
    uint8_t outdatedInsulationResistanceValue; /*!< 0 = Valid, 1 = Outdated */
    uint8_t testImcOverAll;                    /*!< 0 = NotRunning, 1 = Running */
    uint8_t testImcParameterConfiguration;     /*!< 0 = NotWarning, 1 = Warning */
} DATA_BLOCK_INSULATION_MONITORING_s;

/** data block struct for the I2C humidity/temperature sensor */
typedef struct DATA_BLOCK_HTSEN {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    int16_t temperature_ddegC;
    uint8_t humidity_perc;
} DATA_BLOCK_HTSEN_s;

/** data block struct of internal ADC voltage measurement */
typedef struct DATA_BLOCK_ADC_VOLTAGE {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                               /*!< Data block header */
    float adc1ConvertedVoltages_mV[ADC_ADC1_MAX_NR_CHANNELS]; /*!< voltages measured by the internal ADC ADC1 */
} DATA_BLOCK_ADC_VOLTAGE_s;

/** data block struct for the database built-in self-test */
typedef struct DATA_BLOCK_DUMMY_FOR_SELF_TEST {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    uint8_t member1;            /*!< first member of self-test struct */
    uint8_t member2;            /*!< second member of self-test struct */
} DATA_BLOCK_DUMMY_FOR_SELF_TEST_s;

/** array for the database */
extern DATA_BASE_s data_database[DATA_BLOCK_ID_MAX];

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__DATABASE_CFG_H_ */
