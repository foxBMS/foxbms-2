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
 * @file    adi_ades183x_commands_voltages.c
 * @author  foxBMS Team
 * @date    2022-12-06 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Command definitions related to voltage measurement only specific to
 *          the driver for the ADI ades183x.
 *
 */

/*========== Includes =======================================================*/
#include "adi_ades183x_commands_voltages.h"

#include "adi_ades183x_defs.h"
#include "fassert.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
const uint16_t adi_cmdRdcva[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCVA_BYTE0, ADI_RDCVA_BYTE1, ADI_RDCVA_INC, ADI_RDCVA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCVA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdcvb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCVB_BYTE0, ADI_RDCVB_BYTE1, ADI_RDCVB_INC, ADI_RDCVB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCVB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdcvc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCVC_BYTE0, ADI_RDCVC_BYTE1, ADI_RDCVC_INC, ADI_RDCVC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCVC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdcvd[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCVD_BYTE0, ADI_RDCVD_BYTE1, ADI_RDCVD_INC, ADI_RDCVD_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCVD_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdcve[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCVE_BYTE0, ADI_RDCVE_BYTE1, ADI_RDCVE_INC, ADI_RDCVE_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCVE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdcvf[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDCVF_BYTE0, ADI_RDCVF_BYTE1, ADI_RDCVF_INC, ADI_RDCVF_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDCVF_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRdaca[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDACA_BYTE0, ADI_RDACA_BYTE1, ADI_RDACA_INC, ADI_RDACA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDACA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdacb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDACB_BYTE0, ADI_RDACB_BYTE1, ADI_RDACB_INC, ADI_RDACB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDACB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdacc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDACC_BYTE0, ADI_RDACC_BYTE1, ADI_RDACC_INC, ADI_RDACC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDACC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdacd[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDACD_BYTE0, ADI_RDACD_BYTE1, ADI_RDACD_INC, ADI_RDACD_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDACD_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdace[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDACE_BYTE0, ADI_RDACE_BYTE1, ADI_RDACE_INC, ADI_RDACE_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDACE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdacf[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDACF_BYTE0, ADI_RDACF_BYTE1, ADI_RDACF_INC, ADI_RDACF_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDACF_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRdfca[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDFCA_BYTE0, ADI_RDFCA_BYTE1, ADI_RDFCA_INC, ADI_RDFCA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDFCA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdfcb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDFCB_BYTE0, ADI_RDFCB_BYTE1, ADI_RDFCB_INC, ADI_RDFCB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDFCB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdfcc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDFCC_BYTE0, ADI_RDFCC_BYTE1, ADI_RDFCC_INC, ADI_RDFCC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDFCC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdfcd[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDFCD_BYTE0, ADI_RDFCD_BYTE1, ADI_RDFCD_INC, ADI_RDFCD_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDFCD_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdfce[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDFCE_BYTE0, ADI_RDFCE_BYTE1, ADI_RDFCE_INC, ADI_RDFCE_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDFCE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdfcf[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDFCF_BYTE0, ADI_RDFCF_BYTE1, ADI_RDFCF_INC, ADI_RDFCF_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDFCF_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

const uint16_t adi_cmdRdsva[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSVA_BYTE0, ADI_RDSVA_BYTE1, ADI_RDSVA_INC, ADI_RDSVA_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSVA_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdsvb[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSVB_BYTE0, ADI_RDSVB_BYTE1, ADI_RDSVB_INC, ADI_RDSVB_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSVB_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdsvc[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSVC_BYTE0, ADI_RDSVC_BYTE1, ADI_RDSVC_INC, ADI_RDSVC_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSVC_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdsvd[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSVD_BYTE0, ADI_RDSVD_BYTE1, ADI_RDSVD_INC, ADI_RDSVD_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSVD_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdsve[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSVE_BYTE0, ADI_RDSVE_BYTE1, ADI_RDSVE_INC, ADI_RDSVE_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSVE_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");
const uint16_t adi_cmdRdsvf[ADI_COMMAND_DEFINITION_LENGTH] =
    {ADI_RDSVF_BYTE0, ADI_RDSVF_BYTE1, ADI_RDSVF_INC, ADI_RDSVF_LEN};
FAS_STATIC_ASSERT(
    (ADI_RDSVF_LEN <= ADI_MAX_REGISTER_SIZE_IN_BYTES),
    "Register length must not be greater than ADI_MAX_REGISTER_SIZE_IN_BYTES");

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
