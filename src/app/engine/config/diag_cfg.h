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
 * @file    diag_cfg.h
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  DIAG
 *
 * @brief   Diagnostic module configuration header
 * @details In this header filer are the different diagnosis channel defines
 *          assigned to different diagnosis IDs. Furthermore are the diagnosis
 *          error log settings be configured here.
 */

#ifndef FOXBMS__DIAG_CFG_H_
#define FOXBMS__DIAG_CFG_H_

/*========== Includes =======================================================*/

#include "battery_system_cfg.h"
#include "database_cfg.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define DIAG_ERROR_SENSITIVITY_FIRST_EVENT (0) /*!< logging at first event */
#define DIAG_ERROR_SENSITIVITY_THIRD_EVENT (2) /*!< logging at third event */
#define DIAG_ERROR_SENSITIVITY_FIFTH_EVENT (4) /*!< logging at fifth event */
#define DIAG_ERROR_SENSITIVITY_TENTH_EVENT (9) /*!< logging at tenth event */

#define DIAG_ERROR_SENSITIVITY_HIGH (0)  /*!< logging at first event */
#define DIAG_ERROR_SENSITIVITY_MID  (5)  /*!< logging at fifth event */
#define DIAG_ERROR_SENSITIVITY_LOW  (10) /*!< logging at tenth event */

#define DIAG_ERROR_INTERLOCK_SENSITIVITY (10) /*!< logging level of interlock */

#define DIAG_ERROR_VOLTAGE_SENSITIVITY_MSL (500) /*!< MSL level for event occurrence if over/under voltage event */
#define DIAG_ERROR_VOLTAGE_SENSITIVITY_RSL (500) /*!< RSL level for event occurrence if over/under voltage event */
#define DIAG_ERROR_VOLTAGE_SENSITIVITY_MOL (500) /*!< MOL level for event occurrence if over/under voltage event */

/** MSL level for event occurrence if over/under temperature event */
#define DIAG_ERROR_TEMPERATURE_SENSITIVITY_MSL (500)
/** RSL level for event occurrence if over/under temperature event */
#define DIAG_ERROR_TEMPERATURE_SENSITIVITY_RSL (500)
/** MOL level for event occurrence if over/under temperature event */
#define DIAG_ERROR_TEMPERATURE_SENSITIVITY_MOL (500)

/** MSL level for event occurrence if over/under current event */
#define DIAG_ERROR_CURRENT_SENSITIVITY_MSL (500)
/** RSL level for event occurrence if over/under current event */
#define DIAG_ERROR_CURRENT_SENSITIVITY_RSL (500)
/** MOL level for event occurrence if over/under current event */
#define DIAG_ERROR_CURRENT_SENSITIVITY_MOL (500)

#define DIAG_ERROR_LTC_PEC_SENSITIVITY (5) /*!< logging level of LTC PEC errors */
#define DIAG_ERROR_LTC_MUX_SENSITIVITY (5) /*!< logging level of LTC MUX errors */
#define DIAG_ERROR_LTC_SPI_SENSITIVITY (5) /*!< logging level of LTC SPI errors */

#define DIAG_ERROR_CAN_TIMING_SENSITIVITY    (100) /*!< logging level of CAN timing errors */
#define DIAG_ERROR_CAN_TIMING_CC_SENSITIVITY (100) /*!< logging level of CAN timing errors on the current sensor */
#define DIAG_ERROR_CAN_SENSOR_SENSITIVITY    (100) /*!< logging level of CAN errors on the current sensor */

/**
 * Logging after 20th event for errors connected related to the contactor
 * feedback. This value is chosen to be so large because of the time delay
 * between the request for a state and the actual physical response. It is
 * caused by the SPI transaction to the SPS module, the rise time of the
 * control signal and the actual opening/closing of the contactor. Only then
 * can the feedback be read correctly, which also take some additional delay
 * depending on the feedback source. */
#define DIAG_ERROR_CONTACTOR_FEEDBACK_SENSITIVITY (20)

/** ---------------- DEFINES FOR ERROR STATE TRANSITION DELAY----------------
 * These defines configure the delay before the transition to the error state
 * occurs if a fault is detected. During this time the BMS can alert a
 * superordinate control unit that the contactors will be opened soon. The
 * superior control unit can take action and e.g., reduce the current until
 * the transition to error state takes place.
 *
 * The delay is not taken into account if severity level #DIAG_FATAL_ERROR of
 * type #DIAG_SEVERITY_LEVEL_e is configured in config array #diag_diagnosisIdConfiguration.
 * For any other severity, #DIAG_DELAY_DISCARDED can be used as a dummy value.
 */
#define DIAG_DELAY_DISCARDED (UINT32_MAX)
/** no delay after error is detected, open contactors instantaneous */
#define DIAG_NO_DELAY (0u)
/** delay for interlock error */
#define DIAG_DELAY_INTERLOCK_ms (100u)
/** delay for overvoltage errors */
#define DIAG_DELAY_OVERVOLTAGE_ms (200u)
/** delay for undervoltage errors */
#define DIAG_DELAY_UNDERVOLTAGE_ms (200u)
/** delay for temperature errors */
#define DIAG_DELAY_TEMPERATURE_ms (1000u)
/** delay for overcurrent errors */
#define DIAG_DELAY_OVERCURRENT_ms (100u)
/** delay for AFE related errors */
#define DIAG_DELAY_AFE_ms (100u)
/** delay for can timing error */
#define DIAG_DELAY_CAN_TIMING_ms (200u)
/** delay for energy counting/coulomb counting timing error */
#define DIAG_DELAY_EC_CC_TIMING_ms (2000u)
/** delay for current sensor response error */
#define DIAG_DELAY_CURRENT_SENSOR_ms (200u)
/** delay for SBC related errors */
#define DIAG_DELAY_SBC_ms (100u)
/** delay for pack voltage plausibility error */
#define DIAG_DELAY_PL_PACK_VOLTAGE_ms (100u)
/** delay for contactor feedback errors */
#define DIAG_DELAY_CONTACTOR_FEEDBACK_ms (100u)
/** delay for deep-discharge error */
#define DIAG_DELAY_DEEP_DISCHARGE_ms (100u)
/** delay redundancy measurement timeout errors */
#define DIAG_DELAY_REDUNDANCY_MEAS_TIMEOUT_ms (100u)
/** delay redundancy measurement errors */
#define DIAG_DELAY_REDUNDANCY_MEAS_ERROR_ms (100u)

/** Maximum number of the same errors that are logged */
#define DIAG_MAX_ENTRIES_OF_ERROR (5)

/** composite type for storing and passing on the local database table handles */
typedef struct {
    DATA_BLOCK_ERROR_STATE_s *pTableError; /*!< database table with error states */
    DATA_BLOCK_MOL_FLAG_s *pTableMol;      /*!< database table with MOL flags */
    DATA_BLOCK_RSL_FLAG_s *pTableRsl;      /*!< database table with RSL flags */
    DATA_BLOCK_MSL_FLAG_s *pTableMsl;      /*!< database table with MSL flags */
} DIAG_DATABASE_SHIM_s;

/** variable for storing and passing on the local database table handles */
extern const DIAG_DATABASE_SHIM_s diag_kDatabaseShim;

/** list of diag IDs */
typedef enum {
    DIAG_ID_FLASHCHECKSUM,     /*!< the checksum of the flashed software could not be validated */
    DIAG_ID_SYSTEM_MONITORING, /*!< the system monitoring module has detected a deviation from task timing limits */
    DIAG_ID_CONFIGASSERT,      /*!< TODO */
    DIAG_ID_AFE_SPI,           /*!< issues with the SPI communication of the AFE */
    DIAG_ID_AFE_COM_INTEGRITY, /*!< error on the communication integrity of the AFE, e.g. PEC error for LTC */
    DIAG_ID_AFE_MUX,           /*!< the multiplexer that is connected to the AFE does not react in an expected way */
    DIAG_ID_AFE_CONFIG,        /*!< the AFE driver has recognized a configuration error */
    DIAG_ID_CAN_TIMING, /*!< the BMS does not receive CAN messages or they are not inside the expected timing constraints */
    DIAG_ID_CAN_RX_QUEUE_FULL, /*!< the reception queue of the driver is full; no new messages can be received */
    DIAG_ID_CAN_CC_RESPONDING, /*!< current counter measurements on the CAN bus are missing or not inside expected timing constraints */
    DIAG_ID_CAN_EC_RESPONDING, /*!< energy counter measurements on the CAN bus are missing or not inside expected timing constraints */
    DIAG_ID_CURRENT_SENSOR_RESPONDING, /*!< current sensor measurements on the CAN bus are missing or not inside expected timing constraints */
    DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE, /*!< redundant measurement of the cell voltages has returned implausible values */
    DIAG_ID_AFE_CELL_VOLTAGE_MEAS_ERROR, /*!< the AFE driver has determined a cell voltage measurement to be implausible */
    DIAG_ID_AFE_CELL_TEMPERATURE_MEAS_ERROR, /*!< the AFE driver has determined a cell temperature measurement to be implausible */
    DIAG_ID_PLAUSIBILITY_CELL_TEMP, /*!< redundant measurement of the cell temperatures has returned implausible values */
    DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD, /*!< the spread (difference between min and max values) of the cell voltages is implausibly high */
    DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD, /*!< the spread (difference between min and max values) of the cell temperatures is implausibly high */
    DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL,        /*!< Cell voltage limits violated */
    DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL,        /*!< Cell voltage limits violated */
    DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL,        /*!< Cell voltage limits violated */
    DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL,       /*!< Cell voltage limits violated */
    DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL,       /*!< Cell voltage limits violated */
    DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL,       /*!< Cell voltage limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL,     /*!< Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_RSL,     /*!< Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL,     /*!< Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MSL,  /*!< Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_RSL,  /*!< Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MOL,  /*!< Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL,    /*!< Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_RSL,    /*!< Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL,    /*!< Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MSL, /*!< Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_RSL, /*!< Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MOL, /*!< Temperature limits violated */
    DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL,         /*!< Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_CHARGE_CELL_RSL,         /*!< Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_CHARGE_CELL_MOL,         /*!< Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL,      /*!< Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_DISCHARGE_CELL_RSL,      /*!< Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MOL,      /*!< Overcurrent on cell-level */
    DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL,       /*!< Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_CHARGE_RSL,       /*!< Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_CHARGE_MOL,       /*!< Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL,    /*!< Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_DISCHARGE_RSL,    /*!< Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MOL,    /*!< Overcurrent on string-level */
    DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL,         /*!< Overcurrent on string-level */
    DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL,      /*!< Overcurrent on pack-level */
    DIAG_ID_CURRENT_ON_OPEN_STRING,              /*!< Current flowing on open string */
    DIAG_ID_DEEP_DISCHARGE_DETECTED,             /*!< the deep discharge flag has been set in persistent memory */
    DIAG_ID_AFE_OPEN_WIRE, /*!< an open (broken) sense wire has been detected on the battery cell measurement */
    DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE, /*!< the plausibility module has decided that the pack voltage is implausible */
    DIAG_ID_INTERLOCK_FEEDBACK, /*!< the interlock feedback indicates it to be open (but it is expected to be closed) */
    DIAG_ID_STRING_MINUS_CONTACTOR_FEEDBACK, /*!< the feedback on a string minus contactor does not match the expected value */
    DIAG_ID_STRING_PLUS_CONTACTOR_FEEDBACK, /*!< the feedback on a string plus contactor does not match the expected value */
    DIAG_ID_PRECHARGE_CONTACTOR_FEEDBACK, /*!< the feedback on a precharge contactor does not match the expected value */
    DIAG_ID_SBC_FIN_STATE,                /*!< the state of the FIN signal in the SBC is not ok */
    DIAG_ID_SBC_RSTB_STATE,               /*!< an activation of the RSTB pin of the SBC has been detected */
    DIAG_ID_BASE_CELL_VOLTAGE_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the base cell voltage measurements are missing */
    DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the redundancy0 cell voltage measurements are missing */
    DIAG_ID_BASE_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the base cell temperature measurements are missing */
    DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the redundancy0 temperature measurements are missing */
    DIAG_ID_PRECHARGE_ABORT_REASON_VOLTAGE, /*!< precharging aborted due to a too high voltage difference */
    DIAG_ID_PRECHARGE_ABORT_REASON_CURRENT, /*!< precharging aborted because measured current was too high */
    DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the current measurement on a string is not updated */
    DIAG_ID_CURRENT_MEASUREMENT_ERROR, /*!< the redundancy module has detected a current measurement to be invalid */
    DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the voltage 1 measurement of a current sensor is not updated */
    DIAG_ID_CURRENT_SENSOR_V2_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the voltage 2 measurement of a current sensor is not updated */
    DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the voltage 3 measurement of a current sensor is not updated */
    DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT, /*!< the redundancy module has detected that the power measurement of a current sensor is not updated */
    DIAG_ID_POWER_MEASUREMENT_ERROR,      /*!< the redundancy module has detected a power measurement to be invalid */
    DIAG_ID_INSULATION_MEASUREMENT_VALID, /*!< the insulation measurement is valid or invalid */
    DIAG_ID_LOW_INSULATION_RESISTANCE_ERROR,   /*!< a critical low insulation resistance has been measured */
    DIAG_ID_LOW_INSULATION_RESISTANCE_WARNING, /*!< a warnable low insulation resistance has been measured */
    DIAG_ID_INSULATION_GROUND_ERROR,           /*!< insulation monitoring has detected a ground error */
    DIAG_ID_I2C_PEX_ERROR,                     /*!< general error with the port expanders */
    DIAG_ID_I2C_RTC_ERROR,                     /*!< i2c communication error with the RTC */
    DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR,         /*!< clock integrity not garanteed error in RTC IC */
    DIAG_ID_RTC_BATTERY_LOW_ERROR,             /*!< RTC IC battery low flag set */
    DIAG_ID_FRAM_READ_CRC_ERROR,               /*!< CRC does not match when reading from the FRAM */
    DIAG_ID_ALERT_MODE, /*!< Critical error while opening the contactors. Fuse has not triggered */
    DIAG_ID_MAX,        /*!< MAX indicator - do not change */
} DIAG_ID_e;

/** diagnosis check result (event) */
typedef enum {
    DIAG_EVENT_OK,     /**< diag channel event OK                */
    DIAG_EVENT_NOT_OK, /**< diag channel event NOK               */
    DIAG_EVENT_RESET,  /**< reset diag channel event counter to 0 */
} DIAG_EVENT_e;

/** enable or disable the diagnosis handling for an event */
typedef enum {
    DIAG_EVALUATION_ENABLED,  /**< enables evaluation of diagnosis handling */
    DIAG_EVALUATION_DISABLED, /**< disables evaluation of diagnosis handling */
} DIAG_EVALUATE_e;

/** impact level of diagnosis event, e.g., influences the event the whole system or only a string */
typedef enum {
    DIAG_SYSTEM, /**< diag event impact is system related e.g., can timing */
    DIAG_STRING, /**< diag event impact is string related e.g., overvoltage in string x */
} DIAG_IMPACT_LEVEL_e;

/**
 * @def     DIAG_CAN_TIMING
 * @brief   Value that is written into the field that describes whether CAN
 *          timing diag entries should be generated
 */
#if BS_CHECK_CAN_TIMING == true
#define DIAG_CAN_TIMING (DIAG_EVALUATION_ENABLED)
#else
#define DIAG_CAN_TIMING (DIAG_EVALUATION_DISABLED)
#endif

/**
 * @def     DIAG_CAN_SENSOR_PRESENT
 * @brief   Value that is written into the field that describes whether current
 *          sensor diag entries should be generated if it is not present
 */
#if BS_CURRENT_SENSOR_PRESENT == true
#define DIAG_CAN_SENSOR_PRESENT (DIAG_EVALUATION_ENABLED)
#else /* BS_CURRENT_SENSOR_PRESENT == true */
#define DIAG_CAN_SENSOR_PRESENT (DIAG_EVALUATION_DISABLED)
#endif /* BS_CURRENT_SENSOR_PRESENT */

/** diagnosis severity level */
typedef enum {
    DIAG_FATAL_ERROR, /*!< severity level fatal error */
    DIAG_WARNING,     /*!< severity level warning */
    DIAG_INFO,        /*!< severity level info */
} DIAG_SEVERITY_LEVEL_e;

/** diagnosis recording activation */
typedef enum {
    DIAG_RECORDING_ENABLED,  /**< enable diagnosis event recording   */
    DIAG_RECORDING_DISABLED, /**< disable diagnosis event recording  */
} DIAG_RECORDING_e;

/**
 * @brief   function type for diag callbacks
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          data
 */
typedef void DIAG_CALLBACK_FUNCTION_f(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/** Channel configuration of one diag channel */
typedef struct {
    DIAG_ID_e id;       /**< diagnosis event id diag_id */
    uint16_t threshold; /**< threshold for number of events which will be
        * tolerated before generating a notification in both directions:
        * threshold = 0: reports the value at first occurrence,
        * threshold = 1: reports the value at second occurrence */
    DIAG_SEVERITY_LEVEL_e
        severity; /**< severity of diag entry, #DIAG_FATAL_ERROR will lead to an opening of the contactors */
    uint32_t
        delay_ms; /**< delay in ms after error detection if severity is #DIAG_FATAL_ERROR until an opening the contactors */
    DIAG_RECORDING_e enable_recording;    /**< if enabled recording in diag_memory * will be activated */
    DIAG_EVALUATE_e enable_evaluate;      /**< if enabled diagnosis event will be evaluated */
    DIAG_CALLBACK_FUNCTION_f *fpCallback; /**< will be called if
        * number of events exceeds threshold in both
        * directions with parameter DIAG_EVENT_e
        * string id or system related data */
} DIAG_ID_CFG_s;

/** struct for device Configuration of diag module */
typedef struct {
    uint8_t nrOfConfiguredDiagnosisEntries;          /*!< number of entries in DIAG_ID_CFG_s */
    DIAG_ID_CFG_s *pConfigurationOfDiagnosisEntries; /*!< pointer to configuration array for all diagnosis entries */
    uint16_t numberOfFatalErrors; /*!< number of configured diagnosis entries with severity #DIAG_FATAL_ERROR */
    DIAG_ID_CFG_s *pFatalErrorLinkTable
        [DIAG_ID_MAX]; /*!< list with pointers to all diagnosis entries with severity #DIAG_FATAL_ERROR */
} DIAG_DEV_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** diag device configuration struct */
extern DIAG_DEV_s diag_device;
extern DIAG_ID_CFG_s diag_diagnosisIdConfiguration[DIAG_ID_MAX];

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   update function for diagnosis flags
 * @details TODO
 */
extern void DIAG_UpdateFlags(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__DIAG_CFG_H_ */
