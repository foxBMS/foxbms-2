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
 * @file    adi_ades183x_commands.c
 * @author  foxBMS Team
 * @date    2019-08-27 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Implementation of some software
 *
 */

/*========== Includes =======================================================*/
#include "adi_ades183x_commands.h"

#include "adi_ades183x_defs.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
const uint16_t adi_cmdAdcv[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_ADCV_BYTE0, ADI_ADCV_BYTE1, ADI_ADCV_INC, ADI_ADCV_LEN};
FAS_STATIC_ASSERT(
    (ADI_ADCV_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdAdsv[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_ADSV_BYTE0, ADI_ADSV_BYTE1, ADI_ADSV_INC, ADI_ADSV_LEN};
FAS_STATIC_ASSERT(
    (ADI_ADSV_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdWrcfga[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_WRCFGA_BYTE0, ADI_WRCFGA_BYTE1, ADI_WRCFGA_INC, ADI_WRCFGA_LEN};
FAS_STATIC_ASSERT(
    (ADI_WRCFGA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdWrcfgb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_WRCFGB_BYTE0, ADI_WRCFGB_BYTE1, ADI_WRCFGB_INC, ADI_WRCFGB_LEN};
FAS_STATIC_ASSERT(
    (ADI_WRCFGB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdcfga[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCFGA_BYTE0, ADI_RDCFGA_BYTE1, ADI_RDCFGA_INC, ADI_RDCFGA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCFGA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdcfgb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCFGB_BYTE0, ADI_RDCFGB_BYTE1, ADI_RDCFGB_INC, ADI_RDCFGB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCFGB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdSnap[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_SNAPSHOT_BYTE0, ADI_SNAPSHOT_BYTE1, ADI_SNAPSHOT_INC, ADI_SNAPSHOT_LEN};
FAS_STATIC_ASSERT(
    (ADI_SNAPSHOT_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdUnsnap[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_UNSNAPSHOT_BYTE0, ADI_UNSNAPSHOT_BYTE1, ADI_UNSNAPSHOT_INC, ADI_UNSNAPSHOT_LEN};
FAS_STATIC_ASSERT(
    (ADI_UNSNAPSHOT_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdMute[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_MUTE_BYTE0, ADI_MUTE_BYTE1, ADI_MUTE_INC, ADI_MUTE_LEN};
FAS_STATIC_ASSERT(
    (ADI_MUTE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdUnmute[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_UNMUTE_BYTE0, ADI_UNMUTE_BYTE1, ADI_UNMUTE_INC, ADI_UNMUTE_LEN};
FAS_STATIC_ASSERT(
    (ADI_UNMUTE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRstcc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RSTCC_BYTE0, ADI_RSTCC_BYTE1, ADI_RSTCC_INC, ADI_RSTCC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RSTCC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdAdax[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_ADAX_BYTE0, ADI_ADAX_BYTE1, ADI_ADAX_INC, ADI_ADAX_LEN};
FAS_STATIC_ASSERT(
    (ADI_ADAX_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdAdax2[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_ADAX2_BYTE0, ADI_ADAX2_BYTE1, ADI_ADAX2_INC, ADI_ADAX2_LEN};
FAS_STATIC_ASSERT(
    (ADI_ADAX2_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRdauxa[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDAUXA_BYTE0, ADI_RDAUXA_BYTE1, ADI_RDAUXA_INC, ADI_RDAUXA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDAUXA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdauxb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDAUXB_BYTE0, ADI_RDAUXB_BYTE1, ADI_RDAUXB_INC, ADI_RDAUXB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDAUXB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdauxc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDAUXC_BYTE0, ADI_RDAUXC_BYTE1, ADI_RDAUXC_INC, ADI_RDAUXC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDAUXC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdauxd[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDAUXD_BYTE0, ADI_RDAUXD_BYTE1, ADI_RDAUXD_INC, ADI_RDAUXD_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDAUXD_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdauxe[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDAUXE_BYTE0, ADI_RDAUXE_BYTE1, ADI_RDAUXE_INC, ADI_RDAUXE_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDAUXE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRdraxa[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDRAXA_BYTE0, ADI_RDRAXA_BYTE1, ADI_RDRAXA_INC, ADI_RDRAXA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDRAXA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdraxb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDRAXB_BYTE0, ADI_RDRAXB_BYTE1, ADI_RDRAXB_INC, ADI_RDRAXB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDRAXB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdraxc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDRAXC_BYTE0, ADI_RDRAXC_BYTE1, ADI_RDRAXC_INC, ADI_RDRAXC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDRAXC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdraxd[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDRAXD_BYTE0, ADI_RDRAXD_BYTE1, ADI_RDRAXD_INC, ADI_RDRAXD_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDRAXD_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdClraux[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_CLRAUX_BYTE0, ADI_CLRAUX_BYTE1, ADI_CLRAUX_INC, ADI_CLRAUX_LEN};
FAS_STATIC_ASSERT(
    (ADI_CLRAUX_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRdstata[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSTATA_BYTE0, ADI_RDSTATA_BYTE1, ADI_RDSTATA_INC, ADI_RDSTATA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSTATA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdstatb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSTATB_BYTE0, ADI_RDSTATB_BYTE1, ADI_RDSTATB_INC, ADI_RDSTATB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSTATB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdstatc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSTATC_BYTE0, ADI_RDSTATC_BYTE1, ADI_RDSTATC_INC, ADI_RDSTATC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSTATC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdstatd[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSTATD_BYTE0, ADI_RDSTATD_BYTE1, ADI_RDSTATD_INC, ADI_RDSTATD_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSTATD_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdstate[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSTATE_BYTE0, ADI_RDSTATE_BYTE1, ADI_RDSTATE_INC, ADI_RDSTATE_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSTATE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdClrcell[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_CLRCELL_BYTE0, ADI_CLRCELL_BYTE1, ADI_CLRCELL_INC, ADI_CLRCELL_LEN};
FAS_STATIC_ASSERT(
    (ADI_CLRCELL_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdClrflag[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_CLRFLAG_BYTE0, ADI_CLRFLAG_BYTE1, ADI_CLRFLAG_INC, ADI_CLRFLAG_LEN};
FAS_STATIC_ASSERT(
    (ADI_CLRFLAG_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdWrpwma[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_WRPWMA_BYTE0, ADI_WRPWMA_BYTE1, ADI_WRPWMA_INC, ADI_WRPWMA_LEN};
FAS_STATIC_ASSERT(
    (ADI_WRPWMA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdWrpwmb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_WRPWMB_BYTE0, ADI_WRPWMB_BYTE1, ADI_WRPWMB_INC, ADI_WRPWMB_LEN};
FAS_STATIC_ASSERT(
    (ADI_WRPWMB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdpwma[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDPWMA_BYTE0, ADI_RDPWMA_BYTE1, ADI_RDPWMA_INC, ADI_RDPWMA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDPWMA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRdsid[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSID_BYTE0, ADI_RDSID_BYTE1, ADI_RDSID_INC, ADI_RDSID_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSID_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdSrst[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_SRST_BYTE0, ADI_SRST_BYTE1, ADI_SRST_INC, ADI_SRST_LEN};
FAS_STATIC_ASSERT(
    (ADI_SRST_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
