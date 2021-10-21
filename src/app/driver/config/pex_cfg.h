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
 * @file    pex_cfg.h
 * @author  foxBMS Team
 * @date    2021-08-02 (date of creation)
 * @updated 2021-09-30 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  PEX
 *
 * @brief   Headers for the configuration for the NXP PCA9539 port expander module
 *
 *
 */

#ifndef FOXBMS__PEX_CFG_H_
#define FOXBMS__PEX_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/

/** Number of port expanders */
#define PEX_NR_OF_PORT_EXPANDERS (3u)

/** Defines to match expander numbering from hardware description */
/**@{*/
/* AXIVION Disable Style MisraC2012-2.5: Values are defined even if unused for driver useage. */
#define PEX_PORT_EXPANDER1 (0u)
#define PEX_PORT_EXPANDER2 (1u)
#define PEX_PORT_EXPANDER3 (2u)
/**@}*/

/** Port expander pin positions in registers */
/**@{*/
#define PEX_PIN00 (0u)
#define PEX_PIN01 (1u)
#define PEX_PIN02 (2u)
#define PEX_PIN03 (3u)
#define PEX_PIN04 (4u)
#define PEX_PIN05 (5u)
#define PEX_PIN06 (6u)
#define PEX_PIN07 (7u)
#define PEX_PIN10 (8u)
#define PEX_PIN11 (9u)
#define PEX_PIN12 (10u)
#define PEX_PIN13 (11u)
#define PEX_PIN14 (12u)
#define PEX_PIN15 (13u)
#define PEX_PIN16 (14u)
#define PEX_PIN17 (15u)
/* AXIVION Enable Style MisraC2012-2.5: */
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/

extern const uint8_t pex_addressList[PEX_NR_OF_PORT_EXPANDERS];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__PEX_CFG_H_ */
