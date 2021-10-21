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
 * @file    diag_cfg.h
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2021-10-19 (date of last update)
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
#include "general.h"

#include "battery_system_cfg.h"
#include "database_cfg.h"

/*========== Macros and Definitions =========================================*/
#define DIAG_ERROR_SENSITIVITY_FIRST_EVENT (0) /*!< logging at first event */
#define DIAG_ERROR_SENSITIVITY_THIRD_EVENT (2) /*!< logging at first event */
#define DIAG_ERROR_SENSITIVITY_FIFTH_EVENT (4) /*!< logging at first event */
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

/** logging level of errors connected with the contactor feedback */
#define DIAG_ERROR_CONTACTOR_FEEDBACK_SENSITIVITY (20)

/** define if delay in #DIAG_ID_CFG_s is discarded because of severity level */
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
typedef struct DIAG_DATABASE_SHIM {
    DATA_BLOCK_ERRORSTATE_s *pTableError; /*!< database table with errorstates */
    DATA_BLOCK_MOL_FLAG_s *pTableMol;     /*!< database table with MOL flags */
    DATA_BLOCK_RSL_FLAG_s *pTableRsl;     /*!< database table with RSL flags */
    DATA_BLOCK_MSL_FLAG_s *pTableMsl;     /*!< database table with MSL flags */
} DIAG_DATABASE_SHIM_s;

/** variable for storing and passing on the local database table handles */
extern const DIAG_DATABASE_SHIM_s diag_kDatabaseShim;

/** list of diag IDs */
typedef enum DIAG_ID {
    DIAG_ID_FLASHCHECKSUM,    /**< TODO */
    DIAG_ID_SYSTEMMONITORING, /**< TODO */
    DIAG_ID_CONFIGASSERT,     /**< TODO*/
    DIAG_ID_LTC_SPI,
    DIAG_ID_LTC_PEC,
    DIAG_ID_LTC_MUX,
    DIAG_ID_LTC_CONFIG,
    DIAG_ID_CAN_TIMING,
    DIAG_ID_CAN_RX_QUEUE_FULL,
    DIAG_ID_CAN_CC_RESPONDING,
    DIAG_ID_CAN_EC_RESPONDING,
    DIAG_ID_CURRENT_SENSOR_RESPONDING,
    DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE,
    DIAG_ID_AFE_CELL_VOLTAGE_MEAS_ERROR,
    DIAG_ID_AFE_CELL_TEMPERATURE_MEAS_ERROR,
    DIAG_ID_PLAUSIBILITY_CELL_TEMP,
    DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD,
    DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD,
    DIAG_ID_CELLVOLTAGE_OVERVOLTAGE_MSL,         /* Cell voltage limits violated */
    DIAG_ID_CELLVOLTAGE_OVERVOLTAGE_RSL,         /* Cell voltage limits violated */
    DIAG_ID_CELLVOLTAGE_OVERVOLTAGE_MOL,         /* Cell voltage limits violated */
    DIAG_ID_CELLVOLTAGE_UNDERVOLTAGE_MSL,        /* Cell voltage limits violated */
    DIAG_ID_CELLVOLTAGE_UNDERVOLTAGE_RSL,        /* Cell voltage limits violated */
    DIAG_ID_CELLVOLTAGE_UNDERVOLTAGE_MOL,        /* Cell voltage limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL,     /* Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_RSL,     /* Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL,     /* Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MSL,  /* Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_RSL,  /* Temperature limits violated */
    DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MOL,  /* Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL,    /* Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_RSL,    /* Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL,    /* Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MSL, /* Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_RSL, /* Temperature limits violated */
    DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MOL, /* Temperature limits violated */
    DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL,         /* Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_CHARGE_CELL_RSL,         /* Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_CHARGE_CELL_MOL,         /* Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL,      /* Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_DISCHARGE_CELL_RSL,      /* Overcurrent on cell-level */
    DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MOL,      /* Overcurrent on cell-level */
    DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL,       /* Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_CHARGE_RSL,       /* Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_CHARGE_MOL,       /* Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL,    /* Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_DISCHARGE_RSL,    /* Overcurrent on string-level */
    DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MOL,    /* Overcurrent on string-level */
    DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL,         /* Overcurrent on string-level */
    DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL,      /* Overcurrent on pack-level */
    DIAG_ID_CURRENT_ON_OPEN_STRING,              /* Current flowing on open string */
    DIAG_ID_DEEP_DISCHARGE_DETECTED,
    DIAG_ID_OPEN_WIRE,
    DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE,
    DIAG_ID_INTERLOCK_FEEDBACK,
    DIAG_ID_STRING_CONTACTOR_FEEDBACK,
    DIAG_ID_PRECHARGE_CONTACTOR_FEEDBACK,
    DIAG_ID_SBC_FIN_STATE,
    DIAG_ID_SBC_RSTB_STATE,
    DIAG_ID_BASE_CELL_VOLTAGE_MESUREMENT_TIMEOUT,
    DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MESUREMENT_TIMEOUT,
    DIAG_ID_BASE_CELL_TEMPERATURE_MESUREMENT_TIMEOUT,
    DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MESUREMENT_TIMEOUT,
    DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT,
    DIAG_ID_CURRENT_MEASUREMENT_ERROR,
    DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT,
    DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT,
    DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT,
    DIAG_ID_POWER_MEASUREMENT_ERROR,
    DIAG_ID_INSULATION_MEASUREMENT_INVALID,
    DIAG_ID_INSULATION_ERROR,
    DIAG_ID_INSULATION_GROUND_ERROR,
    DIAG_ID_I2C_PEX_ERROR, /**< general error with the port expanders */
    DIAG_ID_MAX,           /**< MAX indicator - do not change */
} DIAG_ID_e;

/** diagnosis check result (event) */
typedef enum DIAG_EVENT {
    DIAG_EVENT_OK,     /**< diag channel event OK                */
    DIAG_EVENT_NOT_OK, /**< diag channel event NOK               */
    DIAG_EVENT_RESET,  /**< reset diag channel eventcounter to 0 */
} DIAG_EVENT_e;

/** enable or disable the diagnosis handling for an event */
typedef enum DIAG_EVALUATE {
    DIAG_EVALUATION_ENABLED,  /**< enables evaluation of diagnosis handling */
    DIAG_EVALUATION_DISABLED, /**< disables evaluation of diagnosis handling */
} DIAG_EVALUATE_e;

/** impact level of diagnosis event, e.g., influences the event the whole system or only a string */
typedef enum DIAG_IMPACT_LEVEL {
    DIAG_SYSTEM, /**< diag event impact is system related e.g., can timing */
    DIAG_STRING, /**< diag event impact is string related e.g., overvoltage in string x */
} DIAG_IMPACT_LEVEL_e;

/**
 * @def     DIAG_CAN_TIMING
 * @brief   Value that is written into the field that describes whether CAN
 *          timing diag entries should be generated
 */
#if CHECK_CAN_TIMING == true
#define DIAG_CAN_TIMING (DIAG_EVALUATION_ENABLED)
#else
#define DIAG_CAN_TIMING (DIAG_EVALUATION_DISABLED)
#endif

/**
 * @def     DIAG_CAN_SENSOR_PRESENT
 * @brief   Value that is written into the field that describes whether current
 *          sensor diag entries should be generated if it is not present
 */
#if CURRENT_SENSOR_PRESENT == true
#define DIAG_CAN_SENSOR_PRESENT (DIAG_EVALUATION_ENABLED)
#else /* CURRENT_SENSOR_PRESENT == true */
#define DIAG_CAN_SENSOR_PRESENT (DIAG_EVALUATION_DISABLED)
#endif /* CURRENT_SENSOR_PRESENT */

/** diagnosis severity level */
typedef enum DIAG_SEVERITY_LEVEL {
    DIAG_FATAL_ERROR, /*!< severity level fatal error */
    DIAG_WARNING,     /*!< severity level warning */
    DIAG_INFO,        /*!< severity level info */
} DIAG_SEVERITY_LEVEL_e;

/** diagnosis recording activation */
typedef enum DIAG_RECORDING {
    DIAG_RECORDING_ENABLED,  /**< enable diagnosis event recording   */
    DIAG_RECORDING_DISABLED, /**< disable diagnosis event recording  */
} DIAG_RECORDING_e;

/**
 * @brief   function type for diag callbacks
 * @param[in] ch_id         ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          data
 */
typedef void DIAG_CALLBACK_FUNCTION_f(
    DIAG_ID_e ch_id,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/** Channel configuration of one diag channel */
typedef struct DIAG_CH_CFG {
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
typedef struct DIAG_DEV {
    uint8_t nr_of_ch;             /*!< number of entries in DIAG_ID_CFG_s */
    DIAG_ID_CFG_s *ch_cfg;        /*!< pointer to diag channel config struct */
    uint16_t numberOfFatalErrors; /*!< number of configured diagnosis entries with severity #DIAG_FATAL_ERROR */
    DIAG_ID_CFG_s *pFatalErrorLinkTable
        [DIAG_ID_MAX]; /*!< list with pointers to all diagnosis entries with severity #DIAG_FATAL_ERROR */
} DIAG_DEV_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** diag device configuration struct */
extern DIAG_DEV_s diag_device;
extern DIAG_ID_CFG_s DIAG_ID_cfg[DIAG_ID_MAX];

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   update function for diagnosis flags
 * @details TODO
 */
extern void DIAG_UpdateFlags(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__DIAG_CFG_H_ */
