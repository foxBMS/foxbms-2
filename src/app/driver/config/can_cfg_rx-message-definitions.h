/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cfg_rx-message-definitions.h
 * @author  foxBMS Team
 * @date    2022-07-01 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   Header for the driver for the CAN module
 * @details TODO
 */

#ifndef FOXBMS__CAN_CFG_RX_MESSAGE_DEFINITIONS_H_
#define FOXBMS__CAN_CFG_RX_MESSAGE_DEFINITIONS_H_

/*========== Includes =======================================================*/
#include "foxbms_config.h"

#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANRX_NOT_PERIODIC (0u)

/** CAN message properties for state request message. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - Endianness
 *  - Data length @{*/
#define CANRX_BMS_STATE_REQUEST_ID         (0x210u)
#define CANRX_BMS_STATE_REQUEST_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_BMS_STATE_REQUEST_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_BMS_STATE_REQUEST_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANRX_BMS_STATE_REQUEST_DLC        (CAN_DEFAULT_DLC)
/**@} */

/** CAN message properties for debug message. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - Endianness
 *  - Data length @{*/
#define CANRX_DEBUG_ID         (0x300u)
#define CANRX_DEBUG_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_DEBUG_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_DEBUG_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANRX_DEBUG_DLC        (CAN_DEFAULT_DLC)
/**@}*/

#if (defined(FOXBMS_AFE_DRIVER_DEBUG_CAN) && (FOXBMS_AFE_DRIVER_DEBUG_CAN == 1))
/** CAN message properties for CAN AFE_CellTemperatures message. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - Endianness
 *  - Data length @{*/
#define CANRX_AFE_CELL_TEMPERATURES_ID         (0x280u)
#define CANRX_AFE_CELL_TEMPERATURES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_AFE_CELL_TEMPERATURES_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_AFE_CELL_TEMPERATURES_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANRX_AFE_CELL_TEMPERATURES_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for CAN AFE_CellVoltages message. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - Endianness
 *  - Data length @{*/
#define CANRX_AFE_CELL_VOLTAGES_ID         (0x270u)
#define CANRX_AFE_CELL_VOLTAGES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_AFE_CELL_VOLTAGES_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_AFE_CELL_VOLTAGES_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANRX_AFE_CELL_VOLTAGES_DLC        (CAN_DEFAULT_DLC)
/**@}*/
#endif /* FOXBMS_AFE_DRIVER_DEBUG_CAN */

#if (defined(FOXBMS_AS_HONEYWELL_BAS6C_X00) && (FOXBMS_AS_HONEYWELL_BAS6C_X00 == 1))
/** CAN message properties for aerosol sensor. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - Endianness
 *  - Data length @{*/
#define CANRX_AS_HONEYWELL_BAS6C_X00_ID         (0x3C4u)
#define CANRX_AS_HONEYWELL_BAS6C_X00_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_AS_HONEYWELL_BAS6C_X00_PERIOD_ms  (1000u)
#define CANRX_AS_HONEYWELL_BAS6C_X00_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANRX_AS_HONEYWELL_BAS6C_X00_DLC        (CAN_DEFAULT_DLC)
/**@}*/
#endif /* FOXBMS_AS_HONEYWELL_BAS6C_X00 */

#if (defined(FOXBMS_IMD_BENDER_ISO165C) && (FOXBMS_IMD_BENDER_ISO165C == 1))
/**
 * -------------------------CAUTION-------------------------
 * The following defines are used by the insulation monitoring device (IMD).
 * If they are changed, the IMD will not work anymore
 * -------------------------CAUTION-------------------------
 */
/** CAN message ID for info message from iso165c */
#define CANRX_IMD_BENDER_ISO165C_INFO_ID         (0x37u)
#define CANRX_IMD_BENDER_ISO165C_INFO_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_IMD_BENDER_ISO165C_INFO_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_IMD_BENDER_ISO165C_INFO_ENDIANNESS (CAN_LITTLE_ENDIAN)
#define CANRX_IMD_BENDER_ISO165C_INFO_DLC        (6u)

/** CAN message ID for response message from iso165c */
#define CANRX_IMD_BENDER_ISO165C_RESPONSE_ID         (0x23u)
#define CANRX_IMD_BENDER_ISO165C_RESPONSE_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_IMD_BENDER_ISO165C_RESPONSE_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_IMD_BENDER_ISO165C_RESPONSE_ENDIANNESS (CAN_LITTLE_ENDIAN)
#define CANRX_IMD_BENDER_ISO165C_RESPONSE_DLC        (5u)
#endif /* FOXBMS_IMD_BENDER_ISO165C */

#if (defined(FOXBMS_CS_ISABELLENHUETTE_IVT_S) && (FOXBMS_CS_ISABELLENHUETTE_IVT_S == 1))
/** CAN message properties for Isabellenhuette current sensor messages. @{*/
#define CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC        (6u)
#define CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS (CAN_BIG_ENDIAN)
/**@} */
#endif /* FOXBMS_CS_ISABELLENHUETTE_IVT_S */

#if (defined(FOXBMS_CS_LEM_CAB500) && (FOXBMS_CS_LEM_CAB500 == 1))
/** CAN message properties for LEM current sensor messages. @{*/
#define CANRX_CS_LEM_CAB500_STRING0_MESSAGES_DLC        (6u)
#define CANRX_CS_LEM_CAB500_STRING0_MESSAGES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_CS_LEM_CAB500_STRING0_MESSAGES_ENDIANNESS (CAN_BIG_ENDIAN)
/**@} */
#endif /* FOXBMS_CS_LEM_CAB500 */

#if (defined(FOXBMS_CS_LEM_CAB500) && (FOXBMS_CS_LEM_CAB500 == 1))
/** CAN message properties for LEM current sensor 'current' messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_LEM_CAB500_STRING0_ID         (0x3C2u)
#define CANRX_CS_LEM_CAB500_STRING0_ID_TYPE    (CANRX_CS_LEM_CAB500_STRING0_MESSAGES_ID_TYPE)
#define CANRX_CS_LEM_CAB500_STRING0_PERIOD_ms  (10)
#define CANRX_CS_LEM_CAB500_STRING0_ENDIANNESS (CANRX_CS_LEM_CAB500_STRING0_MESSAGES_ENDIANNESS)
#define CANRX_CS_LEM_CAB500_STRING0_DLC        (CANRX_CS_LEM_CAB500_STRING0_MESSAGES_DLC)
/**@} */
#endif /* FOXBMS_CS_LEM_CAB500 */

#if (defined(FOXBMS_CS_ISABELLENHUETTE_IVT_S) && (FOXBMS_CS_ISABELLENHUETTE_IVT_S == 1))
/** CAN message properties for string 0 current sensor 'current' messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID         (0x521u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */

/** CAN message properties for string 0 current sensor 'voltage 1' messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID         (0x522u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */

/** CAN message properties for string 0 current sensor 'voltage 2' messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID         (0x523u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */

/** CAN message properties for string 0 current sensor 'voltage 3' messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID         (0x524u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */

/** CAN message properties for string 0 current sensor 'temperature' messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID         (0x525u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */

/** CAN message properties for string 0 current sensor 'power' messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID         (0x526u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */

/** CAN message properties for string 0 current sensor 'current counter'
 *  messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID         (0x527u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */

/** CAN message properties for string 0 current sensor 'energy counter'
 *  messages.
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period in ms
 *  - data length
 *  - Endianness @{*/
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID         (0x528u)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID_TYPE    (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ID_TYPE)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ENDIANNESS (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_ENDIANNESS)
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_DLC        (CANRX_CS_MESSAGES_ISABELLENHUETTE_IVT_DLC)
/**@} */
#endif /* FOXBMS_CS_ISABELLENHUETTE_IVT_S */
/* composed Rx  messages */

/* AXIVION Disable Style Generic-NoUnsafeMacro: These macros MUST only be used
   to populate the 'can_rxMessages' array. The type of this array expects data
   exactly as it is provided here and therefore these macros are good (for this
   and only for this purpose!).*/

/*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    timing check not implemented for RX messages!
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#define CANRX_DEBUG_MESSAGE                   \
    {                                         \
        .id         = CANRX_DEBUG_ID,         \
        .idType     = CANRX_DEBUG_ID_TYPE,    \
        .dlc        = CANRX_DEBUG_DLC,        \
        .endianness = CANRX_DEBUG_ENDIANNESS, \
    },                                        \
    {                                         \
        .period = CANRX_DEBUG_PERIOD_ms       \
    }

#if (defined(FOXBMS_AS_HONEYWELL_BAS6C_X00) && (FOXBMS_AS_HONEYWELL_BAS6C_X00 == 1))
#define CANRX_AS_HONEYWELL_BAS6C_X00_MESSAGE                   \
    {                                                          \
        .id         = CANRX_AS_HONEYWELL_BAS6C_X00_ID,         \
        .idType     = CANRX_AS_HONEYWELL_BAS6C_X00_ID_TYPE,    \
        .dlc        = CANRX_AS_HONEYWELL_BAS6C_X00_DLC,        \
        .endianness = CANRX_AS_HONEYWELL_BAS6C_X00_ENDIANNESS, \
    },                                                         \
    {                                                          \
        .period = CANRX_AS_HONEYWELL_BAS6C_X00_PERIOD_ms       \
    }
#endif /* FOXBMS_AS_HONEYWELL_BAS6C_X00 */

#if (defined(FOXBMS_AFE_DRIVER_DEBUG_CAN) && (FOXBMS_AFE_DRIVER_DEBUG_CAN == 1))
#define CANRX_AFE_CELL_VOLTAGES_MESSAGE                   \
    {                                                     \
        .id         = CANRX_AFE_CELL_VOLTAGES_ID,         \
        .idType     = CANRX_AFE_CELL_VOLTAGES_ID_TYPE,    \
        .dlc        = CANRX_AFE_CELL_VOLTAGES_DLC,        \
        .endianness = CANRX_AFE_CELL_VOLTAGES_ENDIANNESS, \
    },                                                    \
    {                                                     \
        .period = CANRX_AFE_CELL_VOLTAGES_PERIOD_ms       \
    }

#define CANRX_AFE_CELL_TEMPERATURES_MESSAGE                   \
    {                                                         \
        .id         = CANRX_AFE_CELL_TEMPERATURES_ID,         \
        .idType     = CANRX_AFE_CELL_TEMPERATURES_ID_TYPE,    \
        .dlc        = CANRX_AFE_CELL_TEMPERATURES_DLC,        \
        .endianness = CANRX_AFE_CELL_TEMPERATURES_ENDIANNESS, \
    },                                                        \
    {                                                         \
        .period = CANRX_AFE_CELL_TEMPERATURES_PERIOD_ms       \
    }
#endif

#if (defined(FOXBMS_IMD_BENDER_ISO165C) && (FOXBMS_IMD_BENDER_ISO165C == 1))
#define CANRX_IMD_BENDER_ISO165C_INFO_MESSAGE                   \
    {                                                           \
        .id         = CANRX_IMD_BENDER_ISO165C_INFO_ID,         \
        .idType     = CANRX_IMD_BENDER_ISO165C_INFO_ID_TYPE,    \
        .dlc        = CANRX_IMD_BENDER_ISO165C_INFO_DLC,        \
        .endianness = CANRX_IMD_BENDER_ISO165C_INFO_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_IMD_BENDER_ISO165C_INFO_PERIOD_ms       \
    }

#define CANRX_IMD_BENDER_ISO165C_RESPONSE_MESSAGE                   \
    {                                                               \
        .id         = CANRX_IMD_BENDER_ISO165C_RESPONSE_ID,         \
        .idType     = CANRX_IMD_BENDER_ISO165C_RESPONSE_ID_TYPE,    \
        .dlc        = CANRX_IMD_BENDER_ISO165C_RESPONSE_DLC,        \
        .endianness = CANRX_IMD_BENDER_ISO165C_RESPONSE_ENDIANNESS, \
    },                                                              \
    {                                                               \
        .period = CANRX_IMD_BENDER_ISO165C_RESPONSE_PERIOD_ms       \
    }
#endif

#define CANRX_BMS_STATE_REQUEST_MESSAGE                   \
    {                                                     \
        .id         = CANRX_BMS_STATE_REQUEST_ID,         \
        .idType     = CANRX_BMS_STATE_REQUEST_ID_TYPE,    \
        .dlc        = CANRX_BMS_STATE_REQUEST_DLC,        \
        .endianness = CANRX_BMS_STATE_REQUEST_ENDIANNESS, \
    },                                                    \
    {                                                     \
        .period = CANRX_BMS_STATE_REQUEST_PERIOD_ms       \
    }

#if (defined(FOXBMS_CS_ISABELLENHUETTE_IVT_S) && (FOXBMS_CS_ISABELLENHUETTE_IVT_S == 1))
#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_MESSAGE                   \
    {                                                                       \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_ENDIANNESS, \
    },                                                                      \
    {                                                                       \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CURR_PERIOD_ms       \
    }

#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_MESSAGE                   \
    {                                                                     \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_ENDIANNESS, \
    },                                                                    \
    {                                                                     \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V1_PERIOD_ms       \
    }

#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_MESSAGE                   \
    {                                                                     \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_ENDIANNESS, \
    },                                                                    \
    {                                                                     \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V2_PERIOD_ms       \
    }

#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_MESSAGE                   \
    {                                                                     \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_ENDIANNESS, \
    },                                                                    \
    {                                                                     \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_V3_PERIOD_ms       \
    }

#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_MESSAGE                   \
    {                                                                       \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_ENDIANNESS, \
    },                                                                      \
    {                                                                       \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_TEMP_PERIOD_ms       \
    }

#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_MESSAGE                   \
    {                                                                      \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_ENDIANNESS, \
    },                                                                     \
    {                                                                      \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_PWR_PERIOD_ms       \
    }

#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_MESSAGE                   \
    {                                                                     \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_ENDIANNESS, \
    },                                                                    \
    {                                                                     \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_CC_PERIOD_ms       \
    }

#define CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_MESSAGE                   \
    {                                                                     \
        .id         = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID,         \
        .idType     = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ID_TYPE,    \
        .dlc        = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_DLC,        \
        .endianness = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_ENDIANNESS, \
    },                                                                    \
    {                                                                     \
        .period = CANRX_CS_ISABELLENHUETTE_IVT_STRING0_EC_PERIOD_ms       \
    }
#endif /* FOXBMS_CS_ISABELLENHUETTE_IVT_S */

#if (defined(FOXBMS_CS_LEM_CAB500) && (FOXBMS_CS_LEM_CAB500 == 1))
#define CANRX_CS_LEM_CAB500_STRING0_MESSAGE                   \
    {                                                         \
        .id         = CANRX_CS_LEM_CAB500_STRING0_ID,         \
        .idType     = CANRX_CS_LEM_CAB500_STRING0_ID_TYPE,    \
        .dlc        = CANRX_CS_LEM_CAB500_STRING0_DLC,        \
        .endianness = CANRX_CS_LEM_CAB500_STRING0_ENDIANNESS, \
    },                                                        \
    {                                                         \
        .period = CANRX_CS_LEM_CAB500_STRING0_PERIOD_ms       \
    }
#endif /* FOXBMS_CS_LEM_CAB500 */

/* AXIVION Enable Style Generic-NoUnsafeMacro */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_CFG_RX_MESSAGE_DEFINITIONS_H_ */
