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
 * @file    adi_ades183x_commands_voltages.h
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Header file of some software
 * @details TODO
 */

#ifndef FOXBMS__ADI_ADES183X_COMMANDS_VOLTAGES_H_
#define FOXBMS__ADI_ADES183X_COMMANDS_VOLTAGES_H_

/*========== Includes =======================================================*/
#include "adi_ades183x_defs.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/** Declarations of voltage measurement related commands */
/**@{*/

extern const uint16_t adi_cmdRdcvb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdcvc[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdcvd[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdcve[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdcvf[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdaca[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdacb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdacc[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdacd[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdace[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdacf[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdfca[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdfcb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdfcc[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdfcd[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdfce[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdfcf[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdsva[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdsvb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdsvc[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdsvd[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdsve[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdsvf[ADI_COMMAND_DEFINITION_LENGTH];

/**@}*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__ADI_ADES183X_COMMANDS_VOLTAGES_H_ */
