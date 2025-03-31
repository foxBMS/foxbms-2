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
 * @file    can_cfg_tx-cyclic-message-definitions.h
 * @author  foxBMS Team
 * @date    2022-07-01 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Header for the driver for the CAN module
 * @details TODO
 */

#ifndef FOXBMS__CAN_CFG_TX_CYCLIC_MESSAGE_DEFINITIONS_H_
#define FOXBMS__CAN_CFG_TX_CYCLIC_MESSAGE_DEFINITIONS_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** CAN message properties for BMS state message. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_BMS_STATE_ID         (0x220u)
#define CANTX_BMS_STATE_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_BMS_STATE_PERIOD_ms  (100u)
#define CANTX_BMS_STATE_PHASE_ms   (0u)
#define CANTX_BMS_STATE_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_BMS_STATE_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for BMS detail state. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_BMS_STATE_DETAILS_ID         (0x221u)
#define CANTX_BMS_STATE_DETAILS_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_BMS_STATE_DETAILS_PERIOD_ms  (1000u)
#define CANTX_BMS_STATE_DETAILS_PHASE_ms   (100u)
#define CANTX_BMS_STATE_DETAILS_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_BMS_STATE_DETAILS_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for BMS cell voltages. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_CELL_VOLTAGES_ID         (0x250u)
#define CANTX_CELL_VOLTAGES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_CELL_VOLTAGES_PERIOD_ms  (100u)
#define CANTX_CELL_VOLTAGES_PHASE_ms   (10u)
#define CANTX_CELL_VOLTAGES_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_CELL_VOLTAGES_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for BMS cell temperatures. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_CELL_TEMPERATURES_ID         (0x260u)
#define CANTX_CELL_TEMPERATURES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_CELL_TEMPERATURES_PERIOD_ms  (200u)
#define CANTX_CELL_TEMPERATURES_PHASE_ms   (20u)
#define CANTX_CELL_TEMPERATURES_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_CELL_TEMPERATURES_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for BMS limit values. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_PACK_LIMITS_ID         (0x232u)
#define CANTX_PACK_LIMITS_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_PACK_LIMITS_PERIOD_ms  (100u)
#define CANTX_PACK_LIMITS_PHASE_ms   (30u)
#define CANTX_PACK_LIMITS_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_PACK_LIMITS_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for minimum and maximum values. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ID         (0x231u)
#define CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_PACK_MINIMUM_MAXIMUM_VALUES_PERIOD_ms  (100u)
#define CANTX_PACK_MINIMUM_MAXIMUM_VALUES_PHASE_ms   (40u)
#define CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_PACK_MINIMUM_MAXIMUM_VALUES_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for pack state estimation values. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_PACK_STATE_ESTIMATION_ID         (0x235u)
#define CANTX_PACK_STATE_ESTIMATION_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_PACK_STATE_ESTIMATION_PERIOD_ms  (1000u)
#define CANTX_PACK_STATE_ESTIMATION_PHASE_ms   (50u)
#define CANTX_PACK_STATE_ESTIMATION_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_PACK_STATE_ESTIMATION_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for pack values. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_PACK_VALUES_P0_ID         (0x233u)
#define CANTX_PACK_VALUES_P0_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_PACK_VALUES_P0_PERIOD_ms  (100u)
#define CANTX_PACK_VALUES_P0_PHASE_ms   (60u)
#define CANTX_PACK_VALUES_P0_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_PACK_VALUES_P0_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for pack values. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_PACK_VALUES_P1_ID         (0x234u)
#define CANTX_PACK_VALUES_P1_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_PACK_VALUES_P1_PERIOD_ms  (100u)
#define CANTX_PACK_VALUES_P1_PHASE_ms   (60u)
#define CANTX_PACK_VALUES_P1_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_PACK_VALUES_P1_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** TX messages - string related */

/** CAN message properties for string state. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_STRING_STATE_ID         (0x240u)
#define CANTX_STRING_STATE_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_STRING_STATE_PERIOD_ms  (100u)
#define CANTX_STRING_STATE_PHASE_ms   (70u)
#define CANTX_STRING_STATE_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_STRING_STATE_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for string values part 0. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_STRING_VALUES_P0_ID         (0x243u)
#define CANTX_STRING_VALUES_P0_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_STRING_VALUES_P0_PERIOD_ms  (100u)
#define CANTX_STRING_VALUES_P0_PHASE_ms   (80u)
#define CANTX_STRING_VALUES_P0_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_STRING_VALUES_P0_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for string values part 1. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_STRING_VALUES_P1_ID         (0x244u)
#define CANTX_STRING_VALUES_P1_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_STRING_VALUES_P1_PERIOD_ms  (100u)
#define CANTX_STRING_VALUES_P1_PHASE_ms   (10u)
#define CANTX_STRING_VALUES_P1_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_STRING_VALUES_P1_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for string minimum and maximum values. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_STRING_MINIMUM_MAXIMUM_VALUES_ID         (0x241u)
#define CANTX_STRING_MINIMUM_MAXIMUM_VALUES_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_STRING_MINIMUM_MAXIMUM_VALUES_PERIOD_ms  (100u)
#define CANTX_STRING_MINIMUM_MAXIMUM_VALUES_PHASE_ms   (90u)
#define CANTX_STRING_MINIMUM_MAXIMUM_VALUES_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_STRING_MINIMUM_MAXIMUM_VALUES_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/** CAN message properties for string state estimation values. Required properties are:
 *  - ID
 *  - Identifier type (standard or extended)
 *  - Period and phase in ms
 *  - Endianness
 *  - data length @{*/
#define CANTX_STRING_STATE_ESTIMATION_ID         (0x245u)
#define CANTX_STRING_STATE_ESTIMATION_ID_TYPE    (CAN_STANDARD_IDENTIFIER_11_BIT)
#define CANTX_STRING_STATE_ESTIMATION_PERIOD_ms  (1000u)
#define CANTX_STRING_STATE_ESTIMATION_PHASE_ms   (0u)
#define CANTX_STRING_STATE_ESTIMATION_ENDIANNESS (CAN_BIG_ENDIAN)
#define CANTX_STRING_STATE_ESTIMATION_DLC        (CAN_DEFAULT_DLC)
/**@}*/

/* composed Tx  messages */

/* AXIVION Disable Style Generic-NoUnsafeMacro: These macros MUST only be used
   to populate the 'can_txMessages' array. The type of this array expects data
   exactly as it is provided here and therefore these macros are good (for this
   and only for this purpose!).*/
#define CANTX_BMS_STATE_MESSAGE                                                \
    {                                                                          \
        .id         = CANTX_BMS_STATE_ID,                                      \
        .idType     = CANTX_BMS_STATE_ID_TYPE,                                 \
        .dlc        = CANTX_BMS_STATE_DLC,                                     \
        .endianness = CANTX_BMS_STATE_ENDIANNESS,                              \
    },                                                                         \
    {                                                                          \
        .period = CANTX_BMS_STATE_PERIOD_ms, .phase = CANTX_BMS_STATE_PHASE_ms \
    }

#define CANTX_BMS_STATE_DETAILS_MESSAGE                                                        \
    {                                                                                          \
        .id         = CANTX_BMS_STATE_DETAILS_ID,                                              \
        .idType     = CANTX_BMS_STATE_DETAILS_ID_TYPE,                                         \
        .dlc        = CANTX_BMS_STATE_DETAILS_DLC,                                             \
        .endianness = CANTX_BMS_STATE_DETAILS_ENDIANNESS,                                      \
    },                                                                                         \
    {                                                                                          \
        .period = CANTX_BMS_STATE_DETAILS_PERIOD_ms, .phase = CANTX_BMS_STATE_DETAILS_PHASE_ms \
    }

#define CANTX_CELL_VOLTAGES_MESSAGE                                                    \
    {                                                                                  \
        .id         = CANTX_CELL_VOLTAGES_ID,                                          \
        .idType     = CANTX_CELL_VOLTAGES_ID_TYPE,                                     \
        .dlc        = CANTX_CELL_VOLTAGES_DLC,                                         \
        .endianness = CANTX_CELL_VOLTAGES_ENDIANNESS,                                  \
    },                                                                                 \
    {                                                                                  \
        .period = CANTX_CELL_VOLTAGES_PERIOD_ms, .phase = CANTX_CELL_VOLTAGES_PHASE_ms \
    }

#define CANTX_PACK_MINIMUM_MAXIMUM_VALUES_MESSAGE                                                                  \
    {                                                                                                              \
        .id         = CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ID,                                                        \
        .idType     = CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ID_TYPE,                                                   \
        .dlc        = CANTX_PACK_MINIMUM_MAXIMUM_VALUES_DLC,                                                       \
        .endianness = CANTX_PACK_MINIMUM_MAXIMUM_VALUES_ENDIANNESS,                                                \
    },                                                                                                             \
    {                                                                                                              \
        .period = CANTX_PACK_MINIMUM_MAXIMUM_VALUES_PERIOD_ms, .phase = CANTX_PACK_MINIMUM_MAXIMUM_VALUES_PHASE_ms \
    }

#define CANTX_PACK_LIMITS_MESSAGE                                                  \
    {                                                                              \
        .id         = CANTX_PACK_LIMITS_ID,                                        \
        .idType     = CANTX_PACK_LIMITS_ID_TYPE,                                   \
        .dlc        = CANTX_PACK_LIMITS_DLC,                                       \
        .endianness = CANTX_PACK_LIMITS_ENDIANNESS,                                \
    },                                                                             \
    {                                                                              \
        .period = CANTX_PACK_LIMITS_PERIOD_ms, .phase = CANTX_PACK_LIMITS_PHASE_ms \
    }

#define CANTX_CELL_TEMPERATURES_MESSAGE                                                        \
    {                                                                                          \
        .id         = CANTX_CELL_TEMPERATURES_ID,                                              \
        .idType     = CANTX_CELL_TEMPERATURES_ID_TYPE,                                         \
        .dlc        = CANTX_CELL_TEMPERATURES_DLC,                                             \
        .endianness = CANTX_CELL_TEMPERATURES_ENDIANNESS,                                      \
    },                                                                                         \
    {                                                                                          \
        .period = CANTX_CELL_TEMPERATURES_PERIOD_ms, .phase = CANTX_CELL_TEMPERATURES_PHASE_ms \
    }

#define CANTX_PACK_STATE_ESTIMATION_MESSAGE                                                            \
    {                                                                                                  \
        .id         = CANTX_PACK_STATE_ESTIMATION_ID,                                                  \
        .idType     = CANTX_PACK_STATE_ESTIMATION_ID_TYPE,                                             \
        .dlc        = CANTX_PACK_STATE_ESTIMATION_DLC,                                                 \
        .endianness = CANTX_PACK_STATE_ESTIMATION_ENDIANNESS,                                          \
    },                                                                                                 \
    {                                                                                                  \
        .period = CANTX_PACK_STATE_ESTIMATION_PERIOD_ms, .phase = CANTX_PACK_STATE_ESTIMATION_PHASE_ms \
    }

#define CANTX_PACK_VALUES_P0_MESSAGE                                                     \
    {                                                                                    \
        .id         = CANTX_PACK_VALUES_P0_ID,                                           \
        .idType     = CANTX_PACK_VALUES_P0_ID_TYPE,                                      \
        .dlc        = CANTX_PACK_VALUES_P0_DLC,                                          \
        .endianness = CANTX_PACK_VALUES_P0_ENDIANNESS,                                   \
    },                                                                                   \
    {                                                                                    \
        .period = CANTX_PACK_VALUES_P0_PERIOD_ms, .phase = CANTX_PACK_VALUES_P0_PHASE_ms \
    }

#define CANTX_PACK_VALUES_P1_MESSAGE                                                     \
    {                                                                                    \
        .id         = CANTX_PACK_VALUES_P1_ID,                                           \
        .idType     = CANTX_PACK_VALUES_P1_ID_TYPE,                                      \
        .dlc        = CANTX_PACK_VALUES_P1_DLC,                                          \
        .endianness = CANTX_PACK_VALUES_P1_ENDIANNESS,                                   \
    },                                                                                   \
    {                                                                                    \
        .period = CANTX_PACK_VALUES_P1_PERIOD_ms, .phase = CANTX_PACK_VALUES_P1_PHASE_ms \
    }

#define CANTX_STRING_STATE_MESSAGE                                                   \
    {                                                                                \
        .id         = CANTX_STRING_STATE_ID,                                         \
        .idType     = CANTX_STRING_STATE_ID_TYPE,                                    \
        .dlc        = CANTX_STRING_STATE_DLC,                                        \
        .endianness = CANTX_STRING_STATE_ENDIANNESS,                                 \
    },                                                                               \
    {                                                                                \
        .period = CANTX_STRING_STATE_PERIOD_ms, .phase = CANTX_STRING_STATE_PHASE_ms \
    }

#define CANTX_STRING_VALUES_P0_MESSAGE                                                       \
    {                                                                                        \
        .id         = CANTX_STRING_VALUES_P0_ID,                                             \
        .idType     = CANTX_STRING_VALUES_P0_ID_TYPE,                                        \
        .dlc        = CANTX_STRING_VALUES_P0_DLC,                                            \
        .endianness = CANTX_STRING_VALUES_P0_ENDIANNESS,                                     \
    },                                                                                       \
    {                                                                                        \
        .period = CANTX_STRING_VALUES_P0_PERIOD_ms, .phase = CANTX_STRING_VALUES_P0_PHASE_ms \
    }

#define CANTX_STRING_VALUES_P1_MESSAGE                                                       \
    {                                                                                        \
        .id         = CANTX_STRING_VALUES_P1_ID,                                             \
        .idType     = CANTX_STRING_VALUES_P1_ID_TYPE,                                        \
        .dlc        = CANTX_STRING_VALUES_P1_DLC,                                            \
        .endianness = CANTX_STRING_VALUES_P1_ENDIANNESS,                                     \
    },                                                                                       \
    {                                                                                        \
        .period = CANTX_STRING_VALUES_P1_PERIOD_ms, .phase = CANTX_STRING_VALUES_P1_PHASE_ms \
    }

#define CANTX_STRING_MINIMUM_MAXIMUM_VALUES_MESSAGE                                                                    \
    {                                                                                                                  \
        .id         = CANTX_STRING_MINIMUM_MAXIMUM_VALUES_ID,                                                          \
        .idType     = CANTX_STRING_MINIMUM_MAXIMUM_VALUES_ID_TYPE,                                                     \
        .dlc        = CANTX_STRING_MINIMUM_MAXIMUM_VALUES_DLC,                                                         \
        .endianness = CANTX_STRING_MINIMUM_MAXIMUM_VALUES_ENDIANNESS,                                                  \
    },                                                                                                                 \
    {                                                                                                                  \
        .period = CANTX_STRING_MINIMUM_MAXIMUM_VALUES_PERIOD_ms, .phase = CANTX_STRING_MINIMUM_MAXIMUM_VALUES_PHASE_ms \
    }

#define CANTX_STRING_STATE_ESTIMATION_MESSAGE                                                              \
    {                                                                                                      \
        .id         = CANTX_STRING_STATE_ESTIMATION_ID,                                                    \
        .idType     = CANTX_STRING_STATE_ESTIMATION_ID_TYPE,                                               \
        .dlc        = CANTX_STRING_STATE_ESTIMATION_DLC,                                                   \
        .endianness = CANTX_STRING_STATE_ESTIMATION_ENDIANNESS,                                            \
    },                                                                                                     \
    {                                                                                                      \
        .period = CANTX_STRING_STATE_ESTIMATION_PERIOD_ms, .phase = CANTX_STRING_STATE_ESTIMATION_PHASE_ms \
    }
/* AXIVION Enable Style Generic-NoUnsafeMacro */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__CAN_CFG_TX_CYCLIC_MESSAGE_DEFINITIONS_H_ */
