/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    pex_cfg.h
 * @author  foxBMS Team
 * @date    2021-08-02 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  PEX
 *
 * @brief   Headers for the configuration for the NXP PCA9539 port expander module
 * @details TODO
 */

#ifndef FOXBMS__PEX_CFG_H_
#define FOXBMS__PEX_CFG_H_

/*========== Includes =======================================================*/

#include "battery_system_cfg.h"

#include "database.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Number of port expanders */
#define PEX_NR_OF_PORT_EXPANDERS (3u)

/** Defines to match expander numbering from hardware description */
/**@{*/
/* AXIVION Disable Style MisraC2012-2.5: Values are defined even if unused for driver usage. */
#define PEX_PORT_EXPANDER1 (0u)
#define PEX_PORT_EXPANDER2 (1u)
#define PEX_PORT_EXPANDER3 (2u)
/**@}*/

/** Port expander pin positions in registers */
/**@{*/
#define PEX_PORT_0_PIN_0 (0u)
#define PEX_PORT_0_PIN_1 (1u)
#define PEX_PORT_0_PIN_2 (2u)
#define PEX_PORT_0_PIN_3 (3u)
#define PEX_PORT_0_PIN_4 (4u)
#define PEX_PORT_0_PIN_5 (5u)
#define PEX_PORT_0_PIN_6 (6u)
#define PEX_PORT_0_PIN_7 (7u)
#define PEX_PORT_1_PIN_0 (8u)
#define PEX_PORT_1_PIN_1 (9u)
#define PEX_PORT_1_PIN_2 (10u)
#define PEX_PORT_1_PIN_3 (11u)
#define PEX_PORT_1_PIN_4 (12u)
#define PEX_PORT_1_PIN_5 (13u)
#define PEX_PORT_1_PIN_6 (14u)
#define PEX_PORT_1_PIN_7 (15u)
/* AXIVION Enable Style MisraC2012-2.5: */
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/

extern const uint8_t pex_addressList[PEX_NR_OF_PORT_EXPANDERS];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__PEX_CFG_H_ */
