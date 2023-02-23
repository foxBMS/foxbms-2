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
 * @file    database_cfg.h
 * @author  foxBMS Team
 * @date    2015-08-18 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  DATA
 *
 * @brief   Database configuration header
 *
 * @details Provides interfaces to database configuration
 *
 */

#ifndef FOXBMS__DATABASE_CFG_H_
#define FOXBMS__DATABASE_CFG_H_

/*========== Includes =======================================================*/

#include "battery_system_cfg.h"

#include "mcu.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** configuration struct of database channel (data block) */
typedef struct {
    void *pDatabaseEntry; /*!< pointer to the database entry */
    uint32_t dataLength;  /*!< length of the entry */
} DATA_BASE_s;

/** data block identification numbers */
typedef enum {
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
    DATA_BLOCK_ID_ERROR_STATE,
    DATA_BLOCK_ID_CONTACTOR_FEEDBACK,
    DATA_BLOCK_ID_INTERLOCK_FEEDBACK,
    DATA_BLOCK_ID_SOF,
    DATA_BLOCK_ID_SYSTEM_STATE,
    DATA_BLOCK_ID_MSL_FLAG,
    DATA_BLOCK_ID_RSL_FLAG,
    DATA_BLOCK_ID_MOL_FLAG,
    DATA_BLOCK_ID_SOX,
    DATA_BLOCK_ID_STATE_REQUEST,
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

FAS_STATIC_ASSERT(
    (int16_t)DATA_BLOCK_ID_MAX < UINT8_MAX,
    "Maximum number of database entries exceeds UINT8_MAX; adapted length "
    "checking in DATA_Initialize and DATA_IterateOverDatabaseEntries");

/** data block header */
typedef struct {
    DATA_BLOCK_ID_e uniqueId;   /*!< uniqueId of database entry */
    uint32_t timestamp;         /*!< timestamp of last database update */
    uint32_t previousTimestamp; /*!< timestamp of previous database update */
} DATA_BLOCK_HEADER_s;

/** data block struct of cell voltage */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                                /*!< Data block header */
    uint8_t state;                                                             /*!< for future use */
    int32_t stringVoltage_mV[BS_NR_OF_STRINGS];                                /*!< uint: mV */
    int16_t cellVoltage_mV[BS_NR_OF_STRINGS][BS_NR_OF_CELL_BLOCKS_PER_STRING]; /*!< unit: mV */
    uint64_t
        invalidCellVoltage[BS_NR_OF_STRINGS]
                          [BS_NR_OF_MODULES_PER_STRING]; /*!< bitmask if voltages are valid. 0->valid, 1->invalid */
    uint16_t nrValidCellVoltages[BS_NR_OF_STRINGS];      /*!< number of valid voltages */
    uint32_t moduleVoltage_mV[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING]; /*!< unit: mV */
    bool validModuleVoltage[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];   /*!< 0 -> if PEC okay; 1 -> PEC error */
} DATA_BLOCK_CELL_VOLTAGE_s;

/** data block struct of cell temperatures */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                                        /*!< Data block header */
    uint8_t state;                                                                     /*!< for future use */
    int16_t cellTemperature_ddegC[BS_NR_OF_STRINGS][BS_NR_OF_TEMP_SENSORS_PER_STRING]; /*!< unit: deci &deg;C */
    uint16_t invalidCellTemperature
        [BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING]; /*!< bitmask if temperatures are valid. 0->valid, 1->invalid */
    uint16_t nrValidTemperatures[BS_NR_OF_STRINGS];      /*!< number of valid temperatures in each string */
} DATA_BLOCK_CELL_TEMPERATURE_s;

/** data block struct of minimum and maximum values */
typedef struct {
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
    float_t averageTemperature_ddegC[BS_NR_OF_STRINGS];       /*!< unit: deci &deg;C */
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
typedef struct {
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
typedef struct {
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
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    uint8_t enableBalancing;    /*!< Switch for enabling/disabling balancing  */
    uint8_t threshold_mV;       /*!< balancing threshold in mV                */
    uint8_t request;            /*!< balancing request per CAN                */
    uint8_t balancingState[BS_NR_OF_STRINGS]
                          [BS_NR_OF_CELL_BLOCKS_PER_STRING]; /*!< 0: no balancing, 1: balancing active     */
    uint32_t deltaCharge_mAs[BS_NR_OF_STRINGS]
                            [BS_NR_OF_CELL_BLOCKS_PER_STRING]; /*!< Difference in Depth-of-Discharge in mAs  */
    uint16_t nrBalancedCells[BS_NR_OF_STRINGS];
} DATA_BLOCK_BALANCING_CONTROL_s;

/** data structure declaration of DATA_BLOCK_USER_IO_CONTROL */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                            /*!< Data block header */
    uint8_t state;                                         /*!< for future use */
    uint32_t eepromReadAddressToUse;                       /*!< address to read from for  slave EEPROM */
    uint32_t eepromReadAddressLastUsed;                    /*!< last address used to read from slave EEPROM */
    uint32_t eepromWriteAddressToUse;                      /*!< address to write to for slave EEPROM */
    uint32_t eepromWriteAddressLastUsed;                   /*!< last address used to write to for slave EEPROM */
    uint8_t ioValueOut[BS_NR_OF_MODULES_PER_STRING];       /*!< data to be written to the port expander */
    uint8_t ioValueIn[BS_NR_OF_MODULES_PER_STRING];        /*!< data read from to the port expander */
    uint8_t eepromValueWrite[BS_NR_OF_MODULES_PER_STRING]; /*!< data to be written to the slave EEPROM */
    uint8_t eepromValueRead[BS_NR_OF_MODULES_PER_STRING];  /*!< data read from to the slave EEPROM */
    uint8_t
        externalTemperatureSensor[BS_NR_OF_MODULES_PER_STRING]; /*!< temperature from the external sensor on slave */
} DATA_BLOCK_SLAVE_CONTROL_s;

/** data block struct of cell balancing feedback */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                    /*!< Data block header */
    uint8_t state;                                                 /*!< for future use */
    uint16_t value[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING]; /*!< unit: mV (optocoupler output) */
} DATA_BLOCK_BALANCING_FEEDBACK_s;

/** data block struct of user multiplexer values */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                              /*!< Data block header */
    uint8_t state;                                                           /*!< for future use */
    uint16_t value[BS_NR_OF_STRINGS][8u * 2u * BS_NR_OF_MODULES_PER_STRING]; /*!< unit: mV (mux voltage input) */
} DATA_BLOCK_USER_MUX_s;

/** data block struct of cell open wire */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;             /*!< Data block header */
    uint8_t state;                          /*!< for future use */
    uint16_t nrOpenWires[BS_NR_OF_STRINGS]; /*!< number of open wires */
    uint8_t openWire[BS_NR_OF_STRINGS]
                    [BS_NR_OF_MODULES_PER_STRING *
                     (BS_NR_OF_CELL_BLOCKS_PER_MODULE + 1u)]; /*!< 1 -> open wire, 0 -> everything ok */
} DATA_BLOCK_OPEN_WIRE_s;

/** data block struct of GPIO voltage */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    uint8_t state;              /*!< for future use */
    int16_t gpioVoltages_mV[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_GPIOS_PER_MODULE]; /*!< unit: mV */
    int16_t gpaVoltages_mV[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_GPAS_PER_MODULE];   /*!< unit: mV */
    uint16_t
        invalidGpioVoltages[BS_NR_OF_STRINGS]
                           [BS_NR_OF_MODULES_PER_STRING]; /*!< bitmask if voltages are valid. 0->valid, 1->invalid */
} DATA_BLOCK_ALL_GPIO_VOLTAGES_s;

/** data block struct of error flags */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                          /*!< Data block header */
    bool afeCommunicationCrcError[BS_NR_OF_STRINGS];                     /*!< false -> no error, true -> error */
    bool afeSlaveMultiplexerError[BS_NR_OF_STRINGS];                     /*!< false -> no error, true -> error */
    bool afeCommunicationSpiError[BS_NR_OF_STRINGS];                     /*!< false -> no error, true -> error */
    bool afeConfigurationError[BS_NR_OF_STRINGS];                        /*!< false -> no error, true -> error */
    bool afeCellVoltageInvalidError[BS_NR_OF_STRINGS];                   /*!< false -> no error, true -> error */
    bool afeCellTemperatureInvalidError[BS_NR_OF_STRINGS];               /*!< false -> no error, true -> error */
    bool baseCellVoltageMeasurementTimeoutError;                         /*!< false -> no error, true -> error */
    bool redundancy0CellVoltageMeasurementTimeoutError;                  /*!< false -> no error, true -> error */
    bool baseCellTemperatureMeasurementTimeoutError;                     /*!< false -> no error, true -> error */
    bool redundancy0CellTemperatureMeasurementTimeoutError;              /*!< false -> no error, true -> error */
    bool currentMeasurementTimeoutError[BS_NR_OF_STRINGS];               /*!< false -> no error, true -> error */
    bool currentMeasurementInvalidError[BS_NR_OF_STRINGS];               /*!< false -> no error, true -> error */
    bool currentSensorVoltage1TimeoutError[BS_NR_OF_STRINGS];            /*!< false -> no error, true -> error */
    bool currentSensorVoltage2TimeoutError[BS_NR_OF_STRINGS];            /*!< false -> no error, true -> error */
    bool currentSensorVoltage3TimeoutError[BS_NR_OF_STRINGS];            /*!< false -> no error, true -> error */
    bool currentSensorPowerTimeoutError[BS_NR_OF_STRINGS];               /*!< false -> no error, true -> error */
    bool currentSensorCoulombCounterTimeoutError[BS_NR_OF_STRINGS];      /*!< false -> no error, true -> error */
    bool currentSensorEnergyCounterTimeoutError[BS_NR_OF_STRINGS];       /*!< false -> no error, true -> error */
    bool powerMeasurementInvalidError[BS_NR_OF_STRINGS];                 /*!< false -> no error, true -> error */
    bool mainFuseError;                                                  /*!< false -> fuse ok,  true -> fuse tripped */
    bool stringFuseError[BS_NR_OF_STRINGS];                              /*!< false -> fuse ok,  true -> fuse tripped */
    bool openWireDetectedError[BS_NR_OF_STRINGS];                        /*!< false -> no error, true -> error */
    bool stateRequestTimingViolationError;                               /*!< false -> no error, true -> error */
    bool canRxQueueFullError;                                            /*!< false -> no error, true -> error */
    bool coinCellLowVoltageError;                                        /*!< false -> no error, true -> error */
    bool plausibilityCheckPackVoltageError[BS_NR_OF_STRINGS];            /*!< false -> no error, true -> error */
    bool plausibilityCheckCellVoltageError[BS_NR_OF_STRINGS];            /*!< false -> no error, true -> error */
    bool plausibilityCheckCellVoltageSpreadError[BS_NR_OF_STRINGS];      /*!< false -> no error, true -> error */
    bool plausibilityCheckCellTemperatureError[BS_NR_OF_STRINGS];        /*!< false -> no error, true -> error */
    bool plausibilityCheckCellTemperatureSpreadError[BS_NR_OF_STRINGS];  /*!< false -> no error, true -> error */
    bool currentSensorNotRespondingError[BS_NR_OF_STRINGS];              /*!< false -> no error, true -> error */
    bool contactorInNegativePathOfStringFeedbackError[BS_NR_OF_STRINGS]; /*!< false -> no error, true -> error */
    bool contactorInPositivePathOfStringFeedbackError[BS_NR_OF_STRINGS]; /*!< false -> no error, true -> error */
    bool prechargeContactorFeedbackError[BS_NR_OF_STRINGS];              /*!< false -> no error, true -> error */
    bool interlockOpenedError;                                           /*!< false -> no error, true -> error */
    bool insulationMeasurementInvalidError;                              /*!< false -> no error, true -> error */
    bool criticalLowInsulationResistanceError; /*!< false -> no critical resistance , true -> critical low resistance */
    bool warnableLowInsulationResistanceError; /*!< false -> no warnable resistance, true -> warnable low resistance */
    bool
        insulationGroundFaultDetectedError; /*!< false -> no insulation fault between HV and chassis detected, true -> insulation fault detected */
    bool prechargeAbortedDueToVoltage[BS_NR_OF_STRINGS];     /*!< false -> no error, true -> error */
    bool prechargeAbortedDueToCurrent[BS_NR_OF_STRINGS];     /*!< false -> no error, true -> error */
    bool deepDischargeDetectedError[BS_NR_OF_STRINGS];       /*!< false -> no error, true -> error */
    bool currentOnOpenStringDetectedError[BS_NR_OF_STRINGS]; /*!< false -> no error, true -> error */
    bool mcuDieTemperatureViolationError;                    /*!< false -> no error, true -> error */
    bool mcuSbcFinError;                    /*!< false -> no error, true -> error: short-circuit to RSTB */
    bool mcuSbcRstbError;                   /*!< false -> no error, true -> error: RSTB not working */
    bool pexI2cCommunicationError;          /*!< the I2C port expander does not work as expected */
    bool i2cRtcError;                       /*!< problem in I2C communication with RTC */
    bool framReadCrcError;                  /*!< false if read CRC matches with CRC of read data, true otherwise */
    bool rtcClockIntegrityError;            /*!< RTC time integrity not guaranteed, because oscillator has stopped */
    bool rtcBatteryLowError;                /*!< RTC battery voltage is low */
    bool taskEngineTimingViolationError;    /*!< timing violation in engine task */
    bool task1msTimingViolationError;       /*!< timing violation in 1ms task */
    bool task10msTimingViolationError;      /*!< timing violation in 10ms task */
    bool task100msTimingViolationError;     /*!< timing violation in 100ms task */
    bool task100msAlgoTimingViolationError; /*!< timing violation in 100ms algorithm task */
    bool alertFlagSetError;                 /*!< true: ALERT situation detected, false: everything okay */
} DATA_BLOCK_ERROR_STATE_s;

/** data block struct of contactor feedback */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    uint32_t contactorFeedback; /*!< feedback of all contactors, without interlock */
} DATA_BLOCK_CONTACTOR_FEEDBACK_s;

/** data block struct of interlock feedback */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                   /*!< Data block header */
    uint8_t interlockFeedback_IL_STATE;           /*!< feedback of interlock, connected to pin */
    float_t interlockVoltageFeedback_IL_HS_VS_mV; /*!< voltage feedback of interlock, connected to ADC input 2 */
    float_t interlockVoltageFeedback_IL_LS_VS_mV; /*!< voltage feedback of interlock, connected to ADC input 3 */
    float_t interlockCurrentFeedback_IL_HS_CS_mA; /*!< current feedback of interlock, connected to ADC input 4 */
    float_t interlockCurrentFeedback_IL_LS_CS_mA; /*!< current feedback of interlock, connected to ADC input 5 */
} DATA_BLOCK_INTERLOCK_FEEDBACK_s;

/** data block struct of sof limits */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                        /*!< Data block header */
    float_t recommendedContinuousPackChargeCurrent_mA; /*!< recommended continuous operating pack charge current */
    float_t
        recommendedContinuousPackDischargeCurrent_mA; /*!< recommended continuous operating pack discharge current */
    float_t recommendedPeakPackChargeCurrent_mA;      /*!< recommended peak operating pack charge current */
    float_t recommendedPeakPackDischargeCurrent_mA;   /*!< recommended peak operating pack discharge current */
    float_t recommendedContinuousChargeCurrent_mA
        [BS_NR_OF_STRINGS]; /*!< recommended continuous operating charge current    */
    float_t recommendedContinuousDischargeCurrent_mA
        [BS_NR_OF_STRINGS]; /*!< recommended continuous operating discharge current */
    float_t recommendedPeakChargeCurrent_mA[BS_NR_OF_STRINGS];    /*!< recommended peak operating charge current */
    float_t recommendedPeakDischargeCurrent_mA[BS_NR_OF_STRINGS]; /*!< recommended peak operating discharge current */
} DATA_BLOCK_SOF_s;

/** data block struct of system state */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    int32_t bmsCanState;        /*!< system state for CAN messages (e.g., standby, normal) */
} DATA_BLOCK_SYSTEM_STATE_s;

/** data block struct of the maximum safe limits */
typedef struct {
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
typedef struct {
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
typedef struct {
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
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                /*!< Data block header */
    float_t averageSoc_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= averageSoc <= 100.0 */
    float_t minimumSoc_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= minSoc <= 100.0 */
    float_t maximumSoc_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= maxSoc <= 100.0 */
    float_t averageSoe_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= averageSoe <= 100.0 */
    float_t minimumSoe_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= minimumSoe <= 100.0  */
    float_t maximumSoe_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= maximumSoe <= 100.0  */
    float_t averageSoh_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= averageSoh <= 100.0 */
    float_t minimumSoh_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= minimumSoh <= 100.0  */
    float_t maximumSoh_perc[BS_NR_OF_STRINGS]; /*!< 0.0 <= maximumSoh <= 100.0  */
    uint32_t maximumSoe_Wh[BS_NR_OF_STRINGS];  /*!< maximum string energy in Wh */
    uint32_t averageSoe_Wh[BS_NR_OF_STRINGS];  /*!< average string energy in Wh */
    uint32_t minimumSoe_Wh[BS_NR_OF_STRINGS];  /*!< minimum string energy in Wh */
} DATA_BLOCK_SOX_s;

/** data block struct of can state request */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;         /*!< Data block header */
    uint8_t stateRequestViaCan;         /*!< state request */
    uint8_t previousStateRequestViaCan; /*!< previous state request */
    uint8_t stateRequestViaCanPending;  /*!< pending state request */
    uint8_t stateCounter;               /*!< counts state updates */
} DATA_BLOCK_STATE_REQUEST_s;

/** data block struct of the moving average algorithm */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                          /*!< Data block header */
    float_t movingAverageCurrent1sInterval_mA;           /*!< current moving average over the last 1s */
    float_t movingAverageCurrent5sInterval_mA;           /*!< current moving average over the last 5s */
    float_t movingAverageCurrent10sInterval_mA;          /*!< current moving average over the last 10s */
    float_t movingAverageCurrent30sInterval_mA;          /*!< current moving average over the last 30s */
    float_t movingAverageCurrent60sInterval_mA;          /*!< current moving average over the last 60s */
    float_t movingAverageCurrentConfigurableInterval_mA; /*!< current moving average over the last configured time */
    float_t movingAveragePower1sInterval_mA;             /*!< power moving average over the last 1s */
    float_t movingAveragePower5sInterval_mA;             /*!< power moving average over the last 5s */
    float_t movingAveragePower10sInterval_mA;            /*!< power moving average over the last 10s */
    float_t movingAveragePower30sInterval_mA;            /*!< power moving average over the last 30s */
    float_t movingAveragePower60sInterval_mA;            /*!< power moving average over the last 60s */
    float_t movingAveragePowerConfigurableInterval_mA;   /*!< power moving average over the last configured time */
} DATA_BLOCK_MOVING_AVERAGE_s;

/** data block struct of insulation monitoring device measurement */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;         /*!< Data block header */
    bool isImdRunning;                  /*!< true -> Insulation resistance measurement active, false -> not active */
    bool isInsulationMeasurementValid;  /*!< true -> resistance value valid, false -> resistance unreliable */
    uint32_t insulationResistance_kOhm; /*!< insulation resistance measured in kOhm */
    bool
        areDeviceFlagsValid; /*!< true -> flags below this database entry valid, false -> flags unreliable e.g. if device error detected */
    bool
        dfIsCriticalResistanceDetected; /*!< device status flag: false -> resistance value okay, true -> resistance value too low/error */
    bool dfIsWarnableResistanceDetected; /*!< true: warning threshold violated, false: no warning active */
    bool dfIsChassisFaultDetected;       /*!< true: short between HV potential and chassis detected, false: no error */
    bool dfIsChassisShortToHvPlus;       /*!< true: bias/tendency to the location of the insulation fault to HV plus */
    bool dfIsChassisShortToHvMinus;      /*!< true: bias/tendency to the location of the insulation fault to HV minus */
    bool dfIsDeviceErrorDetected;        /*!< true: device error detected, false: no error detected */
    bool dfIsMeasurementUpToDate;        /*!< true: measurement up to-date, false: outdated */
} DATA_BLOCK_INSULATION_MONITORING_s;

/** data block struct for the I2C humidity/temperature sensor */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    int16_t temperature_ddegC;
    uint8_t humidity_perc;
} DATA_BLOCK_HTSEN_s;

/** data block struct of internal ADC voltage measurement */
typedef struct {
    /* This struct needs to be at the beginning of every database entry. During
     * the initialization of a database struct, uniqueId must be set to the
     * respective database entry representation in enum DATA_BLOCK_ID_e. */
    DATA_BLOCK_HEADER_s header;                                 /*!< Data block header */
    float_t adc1ConvertedVoltages_mV[MCU_ADC1_MAX_NR_CHANNELS]; /*!< voltages measured by the internal ADC ADC1 */
} DATA_BLOCK_ADC_VOLTAGE_s;

/** data block struct for the database built-in self-test */
typedef struct {
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
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__DATABASE_CFG_H_ */
