/**
 *
 * @copyright &copy; 2010 - 2022, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cfg_tx-message-definitions.h
 * @author  foxBMS Team
 * @date    2022-07-01 (date of creation)
 * @updated 2022-10-27 (date of last update)
 * @version v1.4.1
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Header for the driver for the CAN module
 *
 */

#ifndef FOXBMS__CAN_CFG_TX_MESSAGE_DEFINITIONS_H_
#define FOXBMS__CAN_CFG_TX_MESSAGE_DEFINITIONS_H_

/*========== Includes =======================================================*/
#include "can_cfg.h"

/*========== Macros and Definitions =========================================*/
#define CANTX_NOT_PERIODIC (0u)

/* Message IDs */

/** CAN message ID for boot message */
#define CANTX_BOOT_ID (0x199u) /* check_ids silence: CANTX_BOOT_ID */
/** Periodicity of boot messages in ms */
#define CANTX_BOOT_PERIOD_ms (CANTX_NOT_PERIODIC) /* dummy */
#define CANTX_BOOT_PHASE_ms  (0u)                 /* dummy */
#define CANTX_BOOT_MESSAGE   (-1)                 /* dummy  */

/** CAN message ID for die ID message */
#define CANTX_DIE_ID_ID (0x227u) /* mark as dummy but it is not src/app/driver/config/can_cfg_tx_boot-message.c */
/** Periodicity of die ID messages in ms */
#define CANTX_DIE_ID_PERIOD_ms (CANTX_NOT_PERIODIC) /* dummy */
#define CANTX_DIE_ID_PHASE_ms  (0u)                 /* dummy */
#define CANTX_DIE_ID_MESSAGE   (-1)                 /* dummy */

/** CAN message ID to send state */
#define CANTX_BMS_STATE_ID (0x220u)
/** Periodicity of CAN state messages in ms */
#define CANTX_BMS_STATE_PERIOD_ms (100u)
/** Phase of CAN state messages in ms */
#define CANTX_BMS_STATE_PHASE_ms (0u)

/** CAN message ID to send detail state */
#define CANTX_BMS_STATE_DETAILS_ID (0x226u)
/** Periodicity of CAN detail state messages in ms */
#define CANTX_BMS_STATE_DETAILS_PERIOD_ms (1000u)
/** Phase of CAN detail state messages in ms */
#define CANTX_BMS_STATE_DETAILS_PHASE_ms (100u)

/** CAN message ID to send voltages */
#define CANTX_CELL_VOLTAGES_ID (0x240u)
/** Periodicity of CAN voltage messages in ms */
#define CANTX_CELL_VOLTAGES_PERIOD_ms (100u)
/** Phase of CAN voltage messages in ms */
#define CANTX_CELL_VOLTAGES_PHASE_ms (10u)

/** CAN message ID to send temperatures */
#define CANTX_CELL_TEMPERATURES_ID (0x250u)
/** Periodicity of CAN temperature messages in ms */
#define CANTX_CELL_TEMPERATURES_PERIOD_ms (200u)
/** Phase of CAN temperature messages in ms */
#define CANTX_CELL_TEMPERATURES_PHASE_ms (20u)

/** CAN message ID to send limit values */
#define CANTX_LIMIT_VALUES_ID (0x224u)
/** Periodicity of CAN limit messages in ms */
#define CANTX_LIMIT_VALUES_PERIOD_ms (100u)
/** Phase of CAN limit messages in ms */
#define CANTX_LIMIT_VALUES_PHASE_ms (30u)

/** CAN message ID to send minimum and maximum values */
#define CANTX_MINIMUM_MAXIMUM_VALUES_ID (0x223u)
/** Periodicity of CAN minimum and maximum value messages in ms */
#define CANTX_MINIMUM_MAXIMUM_VALUES_PERIOD_ms (100u)
/** Phase of CAN minimum and maximum value  messages in ms */
#define CANTX_MINIMUM_MAXIMUM_VALUES_PHASE_ms (40u)

/** CAN message ID to send pack state estimation values */
#define CANTX_PACK_STATE_ESTIMATION_ID (0x225u)
/** Periodicity of CAN pack state estimation messages in ms */
#define CANTX_PACK_STATE_ESTIMATION_PERIOD_ms (1000u)
/** Phase of CAN pack state estimation messages in ms */
#define CANTX_PACK_STATE_ESTIMATION_PHASE_ms (50u)

/** CAN message ID to send pack values */
#define CANTX_PACK_VALUES_ID (0x222u)
/** Periodicity of CAN pack values messages in ms */
#define CANTX_PACK_VALUES_PERIOD_ms (100u)
/** Phase of CAN pack values messages in ms */
#define CANTX_PACK_VALUES_PHASE_ms (60u)

/** TX messages - string related */

/** CAN message ID to send string state */
#define CANTX_STRING_STATE_ID (0x221u)
/** Periodicity of CAN string state messages in ms */
#define CANTX_STRING_STATE_PERIOD_ms (100u)
/** Phase of CAN string state messages in ms */
#define CANTX_STRING_STATE_PHASE_ms (70u)

/** CAN message ID to send string values */
#define CANTX_STRING_VALUES_P0_ID (0x280u)
/** Periodicity of CAN string state messages in ms */
#define CANTX_STRING_VALUES_P0_PERIOD_ms (100u)
/** Phase of CAN string state messages in ms */
#define CANTX_STRING_VALUES_P0_PHASE_ms (80u)

/** CAN message ID to send string minimum and maximum values */
#define CANTX_STRING_MIN_MAX_VALUES_ID (0x281u)
/** Periodicity of CAN string state messages in ms */
#define CANTX_STRING_MIN_MAX_VALUES_PERIOD_ms (100u)
/** Phase of CAN string state messages in ms */
#define CANTX_STRING_MIN_MAX_VALUES_PHASE_ms (90u)

/** CAN message ID to send string minimum and maximum values */
#define CANTX_STRING_STATE_ESTIMATION_ID (0x282u)
/** Periodicity of CAN string state messages in ms */
#define CANTX_STRING_STATE_ESTIMATION_PERIOD_ms (1000u)
/** Phase of CAN string state messages in ms */
#define CANTX_STRING_STATE_ESTIMATION_PHASE_ms (0u)

/** CAN message ID to send string values 2 */
#define CANTX_STRING_VALUES_P1_ID (0x283u)
/** Periodicity of CAN string state messages in ms */
#define CANTX_STRING_VALUES_P1_PERIOD_ms (100u)
/** Phase of CAN string state messages in ms */
#define CANTX_STRING_VALUES_P1_PHASE_ms (10u)

/* composed Tx  messages */

/* AXIVION Disable Style Generic-NoUnsafeMacro: These macros MUST only be used
   to populate the 'can_txMessages' array. The type of this array expects data
   exactly as it is provided here and therefore these macros are good (for this
   and only for this purpose!).*/
#define CANTX_BMS_STATE_MESSAGE                                                \
    {                                                                          \
        .dlc        = CAN_DEFAULT_DLC,                                         \
        .id         = CANTX_BMS_STATE_ID,                                      \
        .endianness = CAN_BIG_ENDIAN,                                          \
    },                                                                         \
    {                                                                          \
        .period = CANTX_BMS_STATE_PERIOD_ms, .phase = CANTX_BMS_STATE_PHASE_ms \
    }

#define CANTX_BMS_STATE_DETAILS_MESSAGE                                                        \
    {                                                                                          \
        .id         = CANTX_BMS_STATE_DETAILS_ID,                                              \
        .dlc        = CAN_DEFAULT_DLC,                                                         \
        .endianness = CAN_BIG_ENDIAN,                                                          \
    },                                                                                         \
    {                                                                                          \
        .period = CANTX_BMS_STATE_DETAILS_PERIOD_ms, .phase = CANTX_BMS_STATE_DETAILS_PHASE_ms \
    }

#define CANTX_CELL_VOLTAGES_MESSAGE                                                    \
    {                                                                                  \
        .id         = CANTX_CELL_VOLTAGES_ID,                                          \
        .dlc        = CAN_DEFAULT_DLC,                                                 \
        .endianness = CAN_BIG_ENDIAN,                                                  \
    },                                                                                 \
    {                                                                                  \
        .period = CANTX_CELL_VOLTAGES_PERIOD_ms, .phase = CANTX_CELL_VOLTAGES_PHASE_ms \
    }

#define CANTX_MINIMUM_MAXIMUM_VALUES_MESSAGE                                                             \
    {                                                                                                    \
        .id         = CANTX_MINIMUM_MAXIMUM_VALUES_ID,                                                   \
        .dlc        = CAN_DEFAULT_DLC,                                                                   \
        .endianness = CAN_BIG_ENDIAN,                                                                    \
    },                                                                                                   \
    {                                                                                                    \
        .period = CANTX_MINIMUM_MAXIMUM_VALUES_PERIOD_ms, .phase = CANTX_MINIMUM_MAXIMUM_VALUES_PHASE_ms \
    }

#define CANTX_LIMIT_VALUES_MESSAGE                                                   \
    {                                                                                \
        .id         = CANTX_LIMIT_VALUES_ID,                                         \
        .dlc        = CAN_DEFAULT_DLC,                                               \
        .endianness = CAN_BIG_ENDIAN,                                                \
    },                                                                               \
    {                                                                                \
        .period = CANTX_LIMIT_VALUES_PERIOD_ms, .phase = CANTX_LIMIT_VALUES_PHASE_ms \
    }

#define CANTX_CELL_TEMPERATURES_MESSAGE                                                        \
    {                                                                                          \
        .id         = CANTX_CELL_TEMPERATURES_ID,                                              \
        .dlc        = CAN_DEFAULT_DLC,                                                         \
        .endianness = CAN_BIG_ENDIAN,                                                          \
    },                                                                                         \
    {                                                                                          \
        .period = CANTX_CELL_TEMPERATURES_PERIOD_ms, .phase = CANTX_CELL_TEMPERATURES_PHASE_ms \
    }

#define CANTX_PACK_STATE_ESTIMATION_MESSAGE                                                            \
    {                                                                                                  \
        .id         = CANTX_PACK_STATE_ESTIMATION_ID,                                                  \
        .dlc        = CAN_DEFAULT_DLC,                                                                 \
        .endianness = CAN_BIG_ENDIAN,                                                                  \
    },                                                                                                 \
    {                                                                                                  \
        .period = CANTX_PACK_STATE_ESTIMATION_PERIOD_ms, .phase = CANTX_PACK_STATE_ESTIMATION_PHASE_ms \
    }

#define CANTX_PACK_VALUES_MESSAGE                                                  \
    {                                                                              \
        .id         = CANTX_PACK_VALUES_ID,                                        \
        .dlc        = CAN_DEFAULT_DLC,                                             \
        .endianness = CAN_BIG_ENDIAN,                                              \
    },                                                                             \
    {                                                                              \
        .period = CANTX_PACK_VALUES_PERIOD_ms, .phase = CANTX_PACK_VALUES_PHASE_ms \
    }

#define CANTX_STRING_STATE_MESSAGE                                                   \
    {                                                                                \
        .id         = CANTX_STRING_STATE_ID,                                         \
        .dlc        = CAN_DEFAULT_DLC,                                               \
        .endianness = CAN_BIG_ENDIAN,                                                \
    },                                                                               \
    {                                                                                \
        .period = CANTX_STRING_STATE_PERIOD_ms, .phase = CANTX_STRING_STATE_PHASE_ms \
    }

#define CANTX_STRING_VALUES_P0_MESSAGE                                                       \
    {                                                                                        \
        .id         = CANTX_STRING_VALUES_P0_ID,                                             \
        .dlc        = CAN_DEFAULT_DLC,                                                       \
        .endianness = CAN_BIG_ENDIAN,                                                        \
    },                                                                                       \
    {                                                                                        \
        .period = CANTX_STRING_VALUES_P0_PERIOD_ms, .phase = CANTX_STRING_VALUES_P0_PHASE_ms \
    }

#define CANTX_STRING_VALUES_P1_MESSAGE                                                       \
    {                                                                                        \
        .id         = CANTX_STRING_VALUES_P1_ID,                                             \
        .dlc        = CAN_DEFAULT_DLC,                                                       \
        .endianness = CAN_BIG_ENDIAN,                                                        \
    },                                                                                       \
    {                                                                                        \
        .period = CANTX_STRING_VALUES_P1_PERIOD_ms, .phase = CANTX_STRING_VALUES_P1_PHASE_ms \
    }

#define CANTX_STRING_MIN_MAX_VALUES_MESSAGE                                                            \
    {                                                                                                  \
        .id         = CANTX_STRING_MIN_MAX_VALUES_ID,                                                  \
        .dlc        = CAN_DEFAULT_DLC,                                                                 \
        .endianness = CAN_BIG_ENDIAN,                                                                  \
    },                                                                                                 \
    {                                                                                                  \
        .period = CANTX_STRING_MIN_MAX_VALUES_PERIOD_ms, .phase = CANTX_STRING_MIN_MAX_VALUES_PHASE_ms \
    }

#define CANTX_STRING_STATE_ESTIMATION_MESSAGE                                                              \
    {                                                                                                      \
        .id         = CANTX_STRING_STATE_ESTIMATION_ID,                                                    \
        .dlc        = CAN_DEFAULT_DLC,                                                                     \
        .endianness = CAN_BIG_ENDIAN,                                                                      \
    },                                                                                                     \
    {                                                                                                      \
        .period = CANTX_STRING_STATE_ESTIMATION_PERIOD_ms, .phase = CANTX_STRING_STATE_ESTIMATION_PHASE_ms \
    }
/* AXIVION Enable Style Generic-NoUnsafeMacro */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__CAN_CFG_TX_MESSAGE_DEFINITIONS_H_ */
