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
 * @file    bender_iso165c_cfg.h
 * @author  foxBMS Team
 * @date    2021-03-17 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  I165C
 *
 * @brief   Headers for the configuration for the insulation monitoring
 *
 * @details Please see the manual and data sheet for detailed information about
 *          the specifications and defines used within this file.
 *
 *          docref: iso165C_C1_D00154_06_M_XXEN - 03/2021 - Manual
 *          docref: iso165Cx_D00154_04_D_XXEN - 03/2021 - Data sheet
 */

#ifndef FOXBMS__BENDER_ISO165C_CFG_H_
#define FOXBMS__BENDER_ISO165C_CFG_H_

/*========== Includes =======================================================*/

/* clang-format off */
#include "imd.h"
/* clang-format on */

#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** CAN Node the IMD device is connected to */
#define I165C_CAN_NODE (CAN_NODE_1)

/** Switch if bender iso165C or iso165C-1 is used
 *  true:  iso165C
 *  false: iso165C-1
 */
#define I165C_IS_165C_USED (false)

/** Select if self-test scenario overall (I165C_SELF_TEST_LONG) or ParameterConfig
 *  self-test scenario (I165C_SELF_TEST_SHORT) shall be executed */
#define I165C_SELF_TEST_LONG

/** Measured resistance threshold under which a warning is issued by i165c */
#define I165C_WARNING_THRESHOLD_kOhm IMD_WARNING_THRESHOLD_INSULATION_RESISTANCE_kOhm

/** Measured resistance threshold under which an error is issued by i165c */
#define I165C_ERROR_THRESHOLD_kOhm IMD_ERROR_THRESHOLD_INSULATION_RESISTANCE_kOhm

#if ((I165C_WARNING_THRESHOLD_kOhm > 2000u) || (I165C_WARNING_THRESHOLD_kOhm < 40u))
#error "Warning threshold outside of possible measurement 40-2000 kOhm"
#endif
#if ((I165C_ERROR_THRESHOLD_kOhm > 1000u) || (I165C_ERROR_THRESHOLD_kOhm < 30u))
#error "Warning threshold outside of possible measurement 40-2000 kOhm"
#endif

/** Measurement averaging factor */
#define I165C_MEASUREMENT_AVERAGING_FACTOR (10u) /* Recommended value for EVs according to data sheet: 10 */

/** Number of attempts to get acknowledgement of message reception by i165c */
#define I165C_TRANSMISSION_ATTEMPTS (3u)

/** Number of attempts to receive the IMD_Info message from iso165c */
#define I165C_IMD_INFO_RECEIVE_ATTEMPTS (50u)

/** Max number of attempts to read CAN queue */
#define I165C_MAX_QUEUE_READS (5u)

/** Maximum queue timeout time in milliseconds */
#define I165C_QUEUE_TIMEOUT_ms ((TickType_t)0u)

/** datsheet version: iso165C_D00154_03_M_XXEN/01.2019 */

/**
 * frames have up to two data words
 * (1 data word length = 16 bits)
 * or up to 4 data bytes
 * (1 data byte length = 8 bits)
 *
 * CAUTION: data word order is reversed !!!!!!!!!
 * (see data sheet page 15)
 */

/** data word 1 is in first position */
#define I165C_DW1 (0u)
/** data word 2 is in second position */
#define I165C_DW2 (1u)
/** data word 3 is in third position */
#define I165C_DW3 (2u)
/** data byte 1 is in first position */
#define I165C_DB1 (3u)
/** data byte 2 is in second position */
#define I165C_DB2 (4u)
/** data byte 3 is in third position */
#define I165C_DB3 (5u)
/** data byte 4 is in fourth position */
#define I165C_DB4 (6u)

/** type of messages to communicate with I165C */

/** cyclic message, transmitted every second */
#define I165C_MESSAGETYPE_IMD_INFO (CANRX_IMD_INFO_ID)
/** message for requests (self test, reset, set values...) */
#define I165C_MESSAGETYPE_IMD_REQUEST (CANTX_IMD_REQUEST_ID)
/** answer message, always send by I165C when it received a request*/
#define I165C_MESSAGETYPE_IMD_RESPONSE (CANRX_IMD_RESPONSE_ID)
/** RX message identifier type */
#define I165C_RX_MESSAGE_IDENTIFIER_TYPE (CANRX_IMD_ID_TYPE)
/** TX message identifier type */
#define I165C_TX_MESSAGE_IDENTIFIER_TYPE (CANTX_IMD_ID_TYPE)

/** control commands (CTL) */

/** trigger self test */
#define I165C_CMD_S_IMC_CTL_SELFTEST            (0x21u)
#define I165C_D_IMC_SELFTEST_SCR_CTL_REQUEST    (I165C_DW1)
#define I165C_D_IMC_SELFTEST_SCR_CTL_RESPONSE   (I165C_DW1)
#define I165C_SELFTEST_SCENARIO_NO_ACTION       (0u)
#define I165C_SELFTEST_SCENARIO_OVERALL         (1u)
#define I165C_SELFTEST_SCENARIO_PARAMETERCONFIG (2u)
/** reset I165C */
#define I165C_CMD_S_VIFC_CTL_IMC_RESET (0xC8u)

/** lock and unlock possibility to make changes to I165C */
#define I165C_CMD_S_VIFC_CTL_LOCK           (0xCAu)
#define I165C_D_VIFC_LOCK_MODE_CTL_REQUEST  (I165C_DW1)
#define I165C_D_VIFC_LOCK_MODE_CTL_RESPONSE (I165C_DW1)
#define I165C_D_VIFC_LOCK_PWD_CTL_REQUEST   (I165C_DW2)
#define I165C_LOCKMODE_UNLOCKED             (0u)
#define I165C_LOCKMODE_LOCKED               (1u)
#define I165C_LOCKMODE_UNKNOWN              (100u)
#define I165C_UNLOCK_PASSWORD               (0u)
#define I165C_LOCK_PASSWORD                 (0xFFFFu)

/** change measurement mode */
#define I165C_CMD_S_VIFC_CTL_MEASUREMENT      (0xCBu)
#define I165C_DW_VIFC_CTL_MEASUREMENT_REQUEST (I165C_DW1)
#define I165C_DISABLE_MEASUREMENT             (0u)
#define I165C_ENABLE_MEASUREMENT              (1u)
#define I165C_MEASUREMENT_MODE_UNKNOWN        (100u)

/**
 * set commands (SET)
 * for each command, 0,1 or 2 data words are defined
 * they are defined for the request and the response
 * a response is always sent when the I165C receives
 * a command
 */

/** set error threshold */
#define I165C_CMD_S_IMC_SET_R_ISO_ERR_THR      (0x28u)
#define I165C_D_IMC_R_ISO_ERR_THR_SET_REQUEST  (I165C_DW1)
#define I165C_D_IMC_R_ISO_ERR_THR_SET_RESPONSE (I165C_DW1)
/** set warning threshold */
#define I165C_CMD_S_IMC_SET_R_ISO_ERR_WRN      (0x29u)
#define I165C_D_IMC_R_ISO_ERR_WRN_SET_REQUEST  (I165C_DW1)
#define I165C_D_IMC_R_ISO_ERR_WRN_SET_RESPONSE (I165C_DW1)
/** set the mean factor of the insulation resistance averaging algorithm */
#define I165C_CMD_S_IMC_SET_MEAN_FACTOR      (0x2Bu)
#define I165C_D_IMC_MEAN_FACTOR_SET_REQUEST  (I165C_DW1)
#define I165C_D_IMC_MEAN_FACTOR_SET_RESPONSE (I165C_DW1)
/** change the state of the HV relays in the HV coupling network */
#define I165C_CMD_S_VIFC_SET_HV_RELAIS            (0xD2u)
#define I165C_D_VIFC_HV_RELAIS_SET_REQUEST        (I165C_DW1)
#define I165C_D_VIFC_HV_RELAIS_STATE_SET_REQUEST  (I165C_DW2)
#define I165C_D_VIFC_HV_RELAIS_NEGATIVE           (0u)
#define I165C_D_VIFC_HV_RELAIS_POSITIVE           (1u)
#define I165C_D_VIFC_HV_RELAIS_SET_RESPONSE       (I165C_DW1)
#define I165C_D_VIFC_HV_RELAIS_STATE_SET_RESPONSE (I165C_DW2)
#define I165C_RELAY_STATE_OPEN                    (0u)
#define I165C_RELAY_STATE_CLOSED                  (1u)

/**
 * get commands (GET)
 * for each command, 0,1 or 2 data words are defined
 * they are defined for the request and the response
 * a response is always sent when the I165C receives
 * a command
 */

/** dummy command (ping the I165C) */
#define I165C_CMD_S_VIFC_DUMMY (0x00u)

/** get the D_IMC_STATUS internal status of the I165C */
#define I165C_CMD_S_IMC_GET_STATUS      (0x37u)
#define I165C_D_IMC_STATUS_GET_RESPONSE (I165C_DW1)
#define I165C_INSULATION_FAULT_SHIFT    (0u)
#define I165C_CHASSIS_FAULT_SHIFT       (1u)
#define I165C_SYSTEM_FAILURE_SHIFT      (2u)
#define I165C_CALIBRATION_RUNNING_SHIFT (3u)
#define I165C_SELFTEST_RUNNING_SHIFT    (4u)
#define I165C_INSULATION_WARNING_SHIFT  (5u)

/** get the D_IMC_STATUS_EXT device-internal IMC self-test results */
#define I165C_D_IMC_STATUS_EXT_GET_RESPONSE           (I165C_DW2)
#define I165C_CALIBRATION_PARAMETER_SHIFT             (0u)
#define I165C_HARDWARE_FAILURE_SHIFT                  (1u)
#define I165C_EEPROM_PARAMETER_SHIFT                  (2u)
#define I165C_FLASH_PARAMETER_SHIFT                   (3u)
#define I165C_RAM_PARAMETER_SHIFT                     (4u)
#define I165C_STACK_OVERFLOW_SHIFT                    (5u)
#define I165C_PARAMETER_VALUE_SHIFT                   (7u)
#define I165C_TEST_PULSE_VOLTAGE_A_REF_SHIFT          (8u)
#define I165C_VOLTAGE_LEVEL_PLUS12V_SHIFT             (9u)
#define I165C_VOLTAGE_LEVEL_MINUS12V_SHIFT            (10u)
#define I165C_FUSE_BIT_VALID_INVALID_SHIFT            (11u)
#define I165C_HV1_VOLTAGE_SHIFT                       (12u)
#define I165C_HV2_VOLTAGE_SHIFT                       (13u)
#define I165C_MANUFACTURER_STRING_VALID_INVALID_SHIFT (14u)

/** get the insulation resistance */
#define I165C_CMD_S_IMC_GET_R_ISO           (0x35u)
#define I165C_D_IMC_R_ISO_GET_RESPONSE      (I165C_DW1)
#define I165C_D_IMC_R_ISO_BIAS_GET_RESPONSE (I165C_DB1)
#define I165C_BIAS_TO_HV_MINUS              (1u)
#define I165C_BIAS_TO_HV_PLUS               (2u)
#define I165C_D_IMC_R_ISO_CNT_GET_RESPONSE  (I165C_DB2)

/** get error threshold */
#define I165C_CMD_S_IMC_GET_R_ISO_ERR_THR      (0x32u)
#define I165C_D_IMC_R_ISO_ERR_THR_GET_RESPONSE (I165C_DW1)

/** get warning threshold */
#define I165C_CMD_S_IMC_GET_R_ISO_WRN_THR      (0x39u)
#define I165C_D_IMC_R_ISO_WRN_THR_GET_RESPONSE (I165C_DW1)

/** get mean factor */
#define I165C_CMD_S_IMC_GET_MEAN_FACTOR      (0x3Cu)
#define I165C_D_IMC_MEAN_FACTOR_GET_RESPONSE (I165C_DW1)

/** get HV1, HV value between HV1_POS and HV1_NEG */
#define I165C_CMD_S_IMC_GET_HV_1      (0x36u)
#define I165C_D_IMC_HV_1_GET_RESPONSE (I165C_DW1)

/** get HV2, HV value between HV2_POS and HV2_NEG */
#define I165C_CMD_S_IMC_GET_HV_2      (0x3Au)
#define I165C_D_IMC_HV_2_GET_RESPONSE (I165C_DW1)

/** get software version of the IMC */
#define I165C_CMD_S_IMC_GET_VERSION            (0x33u)
#define I165C_D_IMC_VERSION_INDEX_GET_REQUEST  (I165C_DW1)
#define I165C_IMC_BOOTLOADER                   (0u)
#define I165C_IMC_FIRMWARE                     (1u)
#define I165C_IMC_FIRMWARE_ID                  (2u)
#define I165C_IMC_FIRMWARE_HASH                (3u)
#define I165C_D_IMC_VERSION_INDEX_GET_RESPONSE (I165C_DW1)
#define I165C_D_IMC_VERSION_GET_RESPONSE       (I165C_DW2)

/** get insulation monitoring counter value */
#define I165C_CMD_S_IMC_GET_TEST_CNT      (0x5Au)
#define I165C_D_IMC_TEST_CNT_GET_RESPONSE (I165C_DW1)

/** get manufacturer information */
#define I165C_CMD_S_IMC_GET_MANUFACTURER        (0x3Du)
#define I165C_D_IMC_MANUFACT_INDEX_GET_REQUEST  (I165C_DW1)
#define I165C_D_IMC_MANUFACT_INDEX_GET_RESPONSE (I165C_DW1)
#define I165C_D_IMC_MANUFACT_DATA_GET_RESPONSE  (I165C_DW2)

/** get internal status of the D_VIFC_STATUS */
#define I165C_CMD_S_VIFC_GET_STATUS                       (0xDCu)
#define I165C_D_VIFC_STATUS_GET_RESPONSE                  (I165C_DW1)
#define I165C_INSULATION_MEASUREMENT_STATUS_SHIFT         (0u)
#define I165C_IMC_CONNECTIVITY_SHIFT                      (1u)
#define I165C_IMC_ALIVE_STATUS_DETECTION_SHIFT            (2u)
#define I165C_VIFC_COMMAND_SHIFT                          (4u)
#define I165C_RESISTANCE_VALUE_OUTDATED_SHIFT             (8u)
#define I165C_IMC_SELFTEST_OVERALL_SCENARIO_SHIFT         (12u)
#define I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO_SHIFT (13u)

/** get state of the HV relays in the HV coupling network */
#define I165C_CMD_S_VIFC_GET_HV_RELAIS            (0xDDu)
#define I165C_D_VIFC_HV_RELAIS_GET_REQUEST        (I165C_DW1)
#define I165C_D_VIFC_HV_RELAIS_GET_RESPONSE       (I165C_DW1)
#define I165C_D_VIFC_HV_RELAIS_STATE_GET_RESPONSE (I165C_DW2)

/** get "Alive" state of the IMC */
#define I165C_CMD_S_VIFC_GET_IMC_ALIVE      (0xE2u)
#define I165C_D_VIFC_IMC_ALIVE_GET_RESPONSE (I165C_DW1)

/** get software version of the VIFC */
#define I165C_CMD_S_VIFC_GET_VERSION            (0xDEu)
#define I165C_D_IMC_VERSION_INDEX_GET_REQUEST   (I165C_DW1)
#define I165C_D_VIFC_VERSION_INDEX_GET_RESPONSE (I165C_DW1)
#define I165C_D_VIFC_VERSION_GET_RESPONSE       (I165C_DW2)

/** get locking state of the iso165C */
#define I165C_CMD_S_VIFC_GET_LOCK           (0xE0u)
#define I165C_D_VIFC_LOCK_MODE_GET_RESPONSE (I165C_DW1)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BENDER_ISO165C_CFG_H_ */
