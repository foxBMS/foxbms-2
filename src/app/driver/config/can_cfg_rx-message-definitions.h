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
 * @file    can_cfg_rx-message-definitions.h
 * @author  foxBMS Team
 * @date    2022-07-01 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   Header for the driver for the CAN module
 *
 */

#ifndef FOXBMS__CAN_CFG_RX_MESSAGE_DEFINITIONS_H_
#define FOXBMS__CAN_CFG_RX_MESSAGE_DEFINITIONS_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define CANRX_NOT_PERIODIC (0u)

/* Message IDs */

/** CAN message properties for state request message. Required properties are:
 * - Message ID
 * - Identifier type (standard or extended)
 * - Expected message period in ms or if asynchronous message
 * - Endianness of message data @{*/
#define CANRX_BMS_STATE_REQUEST_ID         (0x230u)
#define CANRX_BMS_STATE_REQUEST_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_BMS_STATE_REQUEST_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_BMS_STATE_REQUEST_ENDIANNESS (CAN_BIG_ENDIAN)
/**@} */

/** CAN message properties for debug message. Required properties are:
 * - Message ID
 * - Identifier type (standard or extended)
 * - Expected message period in ms or if asynchronous message
 * - Endianness of message data @{*/
#define CANRX_DEBUG_ID         (0x200u)
#define CANRX_DEBUG_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_DEBUG_PERIOD_ms  (CANRX_NOT_PERIODIC)
#define CANRX_DEBUG_ENDIANNESS (CAN_BIG_ENDIAN)
/**@}*/

/**
 * -------------------------CAUTION-------------------------
 * The following defines are used by the insulation monitoring device (IMD).
 * If they are changed, the IMD will not work anymore
 * -------------------------CAUTION-------------------------
 */
/** CAN message ID for info message from iso165c */
#define CANRX_IMD_INFO_ID (0x37u)
/** CAN message ID for response message from iso165c */
#define CANRX_IMD_RESPONSE_ID (0x23u)
/** CAN message identifier type */
#define CANRX_IMD_ID_TYPE (CAN_STANDARD_IDENTIFIER_11_BIT)

/** CAN message properties for Isabellenhuette current sensor messages. Properties are:
 * - Data length of current sensor CAN messages
 * - Identifier type (standard or extended)
 * - Endianness of message data @{*/
#define CANRX_CURRENT_SENSOR_MESSAGES_DLC        (6u)
#define CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS (CAN_BIG_ENDIAN)
/**@} */

/** CAN message properties for string 0 current sensor messages. Properties for each message are:
 * - Message ID
 * - Expected message period in ms or if asynchronous message @{*/
#define CANRX_STRING0_CURRENT_ID                (0x521u)
#define CANRX_STRING0_CURRENT_PERIOD_ms         (CANRX_NOT_PERIODIC)
#define CANRX_STRING0_VOLTAGE1_ID               (0x522u)
#define CANRX_STRING0_VOLTAGE1_PERIOD_ms        (CANRX_NOT_PERIODIC)
#define CANRX_STRING0_VOLTAGE2_ID               (0x523u)
#define CANRX_STRING0_VOLTAGE2_PERIOD_ms        (CANRX_NOT_PERIODIC)
#define CANRX_STRING0_VOLTAGE3_ID               (0x524u)
#define CANRX_STRING0_VOLTAGE3_PERIOD_ms        (CANRX_NOT_PERIODIC)
#define CANRX_STRING0_TEMPERATURE_ID            (0x525u)
#define CANRX_STRING0_TEMPERATURE_PERIOD_ms     (CANRX_NOT_PERIODIC)
#define CANRX_STRING0_POWER_ID                  (0x526u)
#define CANRX_STRING0_POWER_PERIOD_ms           (CANRX_NOT_PERIODIC)
#define CANRX_STRING0_CURRENT_COUNTER_ID        (0x527u)
#define CANRX_STRING0_CURRENT_COUNTER_PERIOD_ms (CANRX_NOT_PERIODIC)
#define CANRX_STRING0_ENERGY_COUNTER_ID         (0x528u)
#define CANRX_STRING0_ENERGY_COUNTER_PERIOD_ms  (CANRX_NOT_PERIODIC)
/**@} */

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
        .dlc        = CAN_DEFAULT_DLC,        \
        .endianness = CANRX_DEBUG_ENDIANNESS, \
    },                                        \
    {                                         \
        .period = CANRX_DEBUG_PERIOD_ms       \
    }

#define CANRX_IMD_INFO_MESSAGE           \
    {                                    \
        .id         = CANRX_IMD_INFO_ID, \
        .idType     = CANRX_IMD_ID_TYPE, \
        .dlc        = CAN_DEFAULT_DLC,   \
        .endianness = CAN_LITTLE_ENDIAN, \
    },                                   \
    {                                    \
        .period = CANRX_NOT_PERIODIC     \
    }

#define CANRX_IMD_RESPONSE_MESSAGE           \
    {                                        \
        .id         = CANRX_IMD_RESPONSE_ID, \
        .idType     = CANRX_IMD_ID_TYPE,     \
        .dlc        = CAN_DEFAULT_DLC,       \
        .endianness = CAN_LITTLE_ENDIAN,     \
    },                                       \
    {                                        \
        .period = CANRX_NOT_PERIODIC         \
    }

#define CANRX_BMS_STATE_REQUEST_MESSAGE                   \
    {                                                     \
        .id         = CANRX_BMS_STATE_REQUEST_ID,         \
        .idType     = CANRX_BMS_STATE_REQUEST_ID_TYPE,    \
        .dlc        = CAN_DEFAULT_DLC,                    \
        .endianness = CANRX_BMS_STATE_REQUEST_ENDIANNESS, \
    },                                                    \
    {                                                     \
        .period = CANRX_BMS_STATE_REQUEST_PERIOD_ms       \
    }

#define CANRX_STRING0_CURRENT_MESSAGE                           \
    {                                                           \
        .id         = CANRX_STRING0_CURRENT_ID,                 \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_CURRENT_PERIOD_ms               \
    }

#define CANRX_STRING0_VOLTAGE1_MESSAGE                          \
    {                                                           \
        .id         = CANRX_STRING0_VOLTAGE1_ID,                \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_VOLTAGE1_PERIOD_ms              \
    }

#define CANRX_STRING0_VOLTAGE2_MESSAGE                          \
    {                                                           \
        .id         = CANRX_STRING0_VOLTAGE2_ID,                \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_VOLTAGE2_PERIOD_ms              \
    }

#define CANRX_STRING0_VOLTAGE3_MESSAGE                          \
    {                                                           \
        .id         = CANRX_STRING0_VOLTAGE3_ID,                \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_VOLTAGE3_PERIOD_ms              \
    }

#define CANRX_STRING0_TEMPERATURE_MESSAGE                       \
    {                                                           \
        .id         = CANRX_STRING0_TEMPERATURE_ID,             \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_TEMPERATURE_PERIOD_ms           \
    }

#define CANRX_STRING0_POWER_MESSAGE                             \
    {                                                           \
        .id         = CANRX_STRING0_POWER_ID,                   \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_POWER_PERIOD_ms                 \
    }

#define CANRX_STRING0_CURRENT_COUNTER_MESSAGE                   \
    {                                                           \
        .id         = CANRX_STRING0_CURRENT_COUNTER_ID,         \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_CURRENT_COUNTER_PERIOD_ms       \
    }

#define CANRX_STRING0_ENERGY_COUNTER_MESSAGE                    \
    {                                                           \
        .id         = CANRX_STRING0_ENERGY_COUNTER_ID,          \
        .idType     = CANRX_CURRENT_SENSOR_MESSAGES_ID_TYPE,    \
        .dlc        = CANRX_CURRENT_SENSOR_MESSAGES_DLC,        \
        .endianness = CANRX_CURRENT_SENSOR_MESSAGES_ENDIANNESS, \
    },                                                          \
    {                                                           \
        .period = CANRX_STRING0_ENERGY_COUNTER_PERIOD_ms        \
    }
/* AXIVION Enable Style Generic-NoUnsafeMacro */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_CFG_RX_MESSAGE_DEFINITIONS_H_ */
