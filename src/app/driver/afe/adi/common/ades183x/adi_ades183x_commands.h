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
 * @file    adi_ades183x_commands.h
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Header file of some software
 *
 */

#ifndef FOXBMS__ADI_ADES183X_COMMANDS_H_
#define FOXBMS__ADI_ADES183X_COMMANDS_H_

/*========== Includes =======================================================*/
#include "adi_ades183x_defs.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/** Declarations of commands */
/**@{*/

extern const uint16_t adi_cmdAdcv[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdAdsv[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdWrcfga[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdWrcfgb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdcfga[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdcfgb[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdSnap[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdUnsnap[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdMute[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdUnmute[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRstcc[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdAdax[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdAdax2[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdauxa[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdauxb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdauxc[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdauxd[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdauxe[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdraxa[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdraxb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdraxc[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdraxd[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdClraux[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdstata[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdstatb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdstatc[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdstatd[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdstate[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdClrcell[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdClrflag[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdcva[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdWrpwma[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdWrpwmb[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdRdpwma[ADI_COMMAND_DEFINITION_LENGTH];

extern const uint16_t adi_cmdRdsid[ADI_COMMAND_DEFINITION_LENGTH];
extern const uint16_t adi_cmdSrst[ADI_COMMAND_DEFINITION_LENGTH];
/**@}*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__ADI_ADES183X_COMMANDS_H_ */
