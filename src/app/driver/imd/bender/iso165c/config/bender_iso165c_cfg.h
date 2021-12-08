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
 * @file    bender_iso165c_cfg.h
 * @author  foxBMS Team
 * @date    2021-03-17 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  I165C
 *
 * @brief   Headers for the configuration for the insulation monitoring
 *
 *
 */

#ifndef FOXBMS__BENDER_ISO165C_CFG_H_
#define FOXBMS__BENDER_ISO165C_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

/* clang-format off */
#include "imd.h"
/* clang-format on */

#include "battery_system_cfg.h"

#include "can.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/

/** Measured resistance threshold under which a warning is issued by i165c */
#define I165C_WARNING_THRESHOLD_KOHM (400u)

/** Measured resistance threshold under which an error is issued by i165c */
#define I165C_ERROR_THRESHOLD_KOHM (250u)

/** Number of attempts to get acknowledgement of message reception by i165c */
#define I165C_TRANSMISSION_TRIES (3u)

/** Number of attempts to get IMD_Info message from iso165c */
#define I165C_IMD_INFO_RECEIVE_TRIES (50u)

/** Number of attempts to wait for iso165c initialization */
#define I165C_INITIALIZATION_TRIES (5000u)

/** Max number of attempts to read CAN queue */
#define I165C_MAX_QUEUE_READS (5u)

/** Maximum queue timeout time in milliseconds */
#define I165C_QUEUE_TIMEOUT_MS ((TickType_t)0u)

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
#define I165C_MESSAGETYPE_IMD_INFO (CAN_ID_IMD_INFO)
/** message for requests (self test, reset, set values...) */
#define I165C_MESSAGETYPE_IMD_REQUEST (CAN_ID_IMD_REQUEST)
/** answer message, always send by I165C when it received a request*/
#define I165C_MESSAGETYPE_IMD_RESPONSE (CAN_ID_IMD_RESPONSE)

/** control commands (CTL) */

/** trigger self test */
#define I165C_CMD_S_IMC_CTL_SELFTEST            (0x21u)
#define I165C_D_IMC_SELFTEST_SCR_CTL_REQUEST    I165C_DW1
#define I165C_D_IMC_SELFTEST_SCR_CTL_RESPONSE   I165C_DW1
#define I165C_SELFTEST_SCENARIO_NO_ACTION       (0u)
#define I165C_SELFTEST_SCENARIO_OVERALL         (1u)
#define I165C_SELFTEST_SCENARIO_PARAMETERCONFIG (2u)
/** reset I165C */
#define I165C_CMD_S_VIFC_CTL_IMC_RESET (0xC8u)
/** lock and unlock possibility to make changes to I165C */
#define I165C_CMD_S_VIFC_CTL_LOCK           (0xCAu)
#define I165C_D_VIFC_LOCK_MODE_CTL_REQUEST  I165C_DW1
#define I165C_D_VIFC_LOCK_MODE_CTL_RESPONSE I165C_DW1
#define I165C_D_VIFC_LOCK_PWD_CTL_REQUEST   I165C_DW2
#define I165C_LOCKMODE_UNLOCKED             (0u)
#define I165C_LOCKMODE_LOCKED               (1u)
#define I165C_LOCKMODE_UNKNOWN              (100u)
#define I165C_UNLOCK_PASSWD                 (0u)
#define I165C_LOCK_PASSWD                   (0xFFFFu)
/** change measurement mode */
#define I165C_CMD_S_VIFC_CTL_MEASUREMENT (0xCBu)

/**
 * set commands (SET)
 * for each command, 0,1 or 2 data words are defined
 * they are defined for the request and the response
 * a response is always sent when the I165C receives
 * a command
 */

/** set error threshold */
#define I165C_CMD_S_IMC_SET_R_ISO_ERR_THR      (0x28u)
#define I165C_D_IMC_R_ISO_ERR_THR_SET_REQUEST  I165C_DW1
#define I165C_D_IMC_R_ISO_ERR_THR_SET_RESPONSE I165C_DW1
/** set warning threshold */
#define I165C_CMD_S_IMC_SET_R_ISO_ERR_WRN      (0x29u)
#define I165C_D_IMC_R_ISO_ERR_WRN_SET_REQUEST  I165C_DW1
#define I165C_D_IMC_R_ISO_ERR_WRN_SET_RESPONSE I165C_DW1
/** set the mean factor of the insulation resistance averaging algorithm */
#define I165C_CMD_S_IMC_SET_MEAN_FACTOR      (0x2Bu)
#define I165C_D_IMC_MEAN_FACTOR_SET_REQUEST  I165C_DW1
#define I165C_D_IMC_MEAN_FACTOR_SET_RESPONSE I165C_DW1
/** change the state of the HV relays in the HV coupling network */
#define I165C_CMD_S_VIFC_SET_HV_RELAIS            (0xD2u)
#define I165C_D_VIFC_HV_RELAIS_SET_REQUEST        I165C_DW1
#define I165C_D_VIFC_HV_RELAIS_STATE_SET_REQUEST  I165C_DW2
#define I165C_D_VIFC_HV_RELAIS_SET_RESPONSE       I165C_DW1
#define I165C_D_VIFC_HV_RELAIS_STATE_SET_RESPONSE I165C_DW2

/**
 * get commands (GET)
 * for each command, 0,1 or 2 data words are defined
 * they are defined for the request and the response
 * a response is always sent when the I165C receives
 * a command
 */

/** dummy command (ping the I165C) */
#define I165C_CMD_S_VIFC_DUMMY (0x00u)
/** get internal status of the I165C */
#define I165C_CMD_S_IMC_GET_STATUS              (0x37u)
#define I165C_D_IMC_STATUS_GET_RESPONSE         I165C_DW1
#define I165C_INSULATION_FAULT                  (0u)
#define I165C_CHASSIS_FAULT                     (1u)
#define I165C_SYSTEM_FAILURE                    (2u)
#define I165C_CALIBRATION_RUNNING               (3u)
#define I165C_SELFTEST_RUNNING                  (4u)
#define I165C_INSULATION_WARNING                (5u)
#define I165C_D_IMC_STATUS_EXT_GET_RESPONSE     I165C_DW2
#define I165C_CALIBRATION_PARAMETER             (0u)
#define I165C_HARDWARE_FAILURE                  (1u)
#define I165C_EEPROM_PARAMETER                  (2u)
#define I165C_FLASH_PARAMETER                   (3u)
#define I165C_RAM_PARAMETER                     (4u)
#define I165C_STACK_OVERFLOW                    (5u)
#define I165C_PARAMETER_VALUE                   (7u)
#define I165C_TEST_PULSEVOLTAGE_AREF            (8u)
#define I165C_VOLTAGE_LEVEL_PLUS12V             (9u)
#define I165C_VOLTAGE_LEVEL_MINUS12V            (10u)
#define I165C_FUSE_BIT_VALID_INVALID            (11u)
#define I165C_HV1_VOLTAGE                       (12u)
#define I165C_HV2_VOLTAGE                       (13u)
#define I165C_MANUFACTURER_STRING_VALID_INVALID (14u)
/** get the insulation resistance */
#define I165C_CMD_S_IMC_GET_R_ISO           (0x35u)
#define I165C_D_IMC_R_ISO_GET_RESPONSE      I165C_DW1
#define I165C_D_IMC_R_ISO_BIAS_GET_RESPONSE I165C_DB3
#define I165C_D_IMC_R_ISO_CNT_GET_RESPONSE  I165C_DB4
/** get error threshold */
#define I165C_CMD_S_IMC_GET_R_ISO_ERR_THR      (0x32u)
#define I165C_D_IMC_R_ISO_ERR_THR_GET_RESPONSE I165C_DW1
/** get warning threshold */
#define I165C_CMD_S_IMC_GET_R_ISO_WRN_THR      (0x39u)
#define I165C_D_IMC_R_ISO_WRN_THR_GET_RESPONSE I165C_DW1
/** get mean factor */
#define I165C_CMD_S_IMC_GET_MEAN_FACTOR      (0x3Cu)
#define I165C_D_IMC_MEAN_FACTOR_GET_RESPONSE I165C_DW1
/** get HV1, HV value between HV1_POS and HV1_NEG */
#define I165C_CMD_S_IMC_GET_HV_1      (0x36u)
#define I165C_D_IMC_HV_1_GET_RESPONSE I165C_DW1
/** get HV2, HV value between HV2_POS and HV2_NEG */
#define I165C_CMD_S_IMC_GET_HV_2      (0x3Au)
#define I165C_D_IMC_HV_2_GET_RESPONSE I165C_DW1
/** get software version of the IMC */
#define I165C_CMD_S_IMC_GET_VERSION            (0x33u)
#define I165C_D_IMC_VERSION_INDEX_GET_REQUEST  I165C_DW1
#define I165C_IMC_BOOTLOADER                   (0u)
#define I165C_IMC_FIRMWARE                     (1u)
#define I165C_IMC_FIRMWARE_ID                  (2u)
#define I165C_IMC_FIRMWARE_HASH                (3u)
#define I165C_D_IMC_VERSION_INDEX_GET_RESPONSE I165C_DW1
#define I165C_D_IMC_VERSION_GET_RESPONSE       I165C_DW2
/** get insulation monitoring counter value */
#define I165C_CMD_S_IMC_GET_TEST_CNT      (0x5Au)
#define I165C_D_IMC_TEST_CNT_GET_RESPONSE I165C_DW1
/** get manufacturer information */
#define I165C_CMD_S_IMC_GET_MANUFACTURER        (0x3Du)
#define I165C_D_IMC_MANUFACT_INDEX_GET_REQUEST  I165C_DW1
#define I165C_D_IMC_MANUFACT_INDEX_GET_RESPONSE I165C_DW1
#define I165C_D_IMC_MANUFACT_DATA_GET_RESPONSE  I165C_DW2
/** get internal status of the VIFC */
#define I165C_CMD_S_VIFC_GET_STATUS                 (0xDCu)
#define I165C_D_VIFC_STATUS_GET_RESPONSE            I165C_DW1
#define I165C_INSULATION_MEASUREMENT                (0u)
#define I165C_IMC_CONNECTIVITY                      (1u)
#define I165C_IMC_ALIVE_STATUS_DETECTION            (2u)
#define I165C_VIFC_COMMAND                          (4u)
#define I165C_INSULATION_RESISTANCE_VALUE           (8u)
#define I165C_IMC_SELFTEST_OVERALL_SCENARIO         (12u)
#define I165C_IMC_SELFTEST_PARAMETERCONFIG_SCENARIO (13u)
/** get state of the HV relays in the HV coupling network */
#define I165C_CMD_S_VIFC_GET_HV_RELAIS            (0xDDu)
#define I165C_D_VIFC_HV_RELAIS_GET_REQUEST        I165C_DW1
#define I165C_D_VIFC_HV_RELAIS_GET_RESPONSE       I165C_DW1
#define I165C_D_VIFC_HV_RELAIS_STATE_GET_RESPONSE I165C_DW2
/** get "Alive" state of the IMC */
#define I165C_CMD_S_VIFC_GET_IMC_ALIVE      (0xE2u)
#define I165C_D_VIFC_IMC_ALIVE_GET_RESPONSE I165C_DW1
/** get software version of the VIFC */
#define I165C_CMD_S_VIFC_GET_VERSION            (0xDEu)
#define I165C_D_IMC_VERSION_INDEX_GET_REQUEST   I165C_DW1
#define I165C_D_VIFC_VERSION_INDEX_GET_RESPONSE I165C_DW1
#define I165C_D_VIFC_VERSION_GET_RESPONSE       I165C_DW2
/** get locking state of the iso165C */
#define I165C_CMD_S_VIFC_GET_LOCK           (0xE0u)
#define I165C_D_VIFC_LOCK_MODE_GET_RESPONSE I165C_DW1

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__BENDER_ISO165C_CFG_H_ */
