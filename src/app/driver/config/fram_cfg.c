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
 * @file    fram_cfg.c
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2020-03-05 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  FRAM
 *
 * @brief   Configuration for the FRAM module
 *
 *
 *
 */

/*========== Includes =======================================================*/
#include "fram_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/**
 * Variables to be stored in FRAM
 * Must also be declared as external variables in fram_cfg.h
 */
/**@{*/
FRAM_VERSION_s fram_version  = {.project = FRAM_PROJECT_ID_FOXBMS_BASELINE, .version = 0};
FRAM_SOC_s fram_soc          = {0};
FRAM_SOE_s fram_soe          = {0};
FRAM_SBC_INIT_s fram_sbcInit = {
    .phase    = 0u,
    .finState = STD_NOT_OK,
};
FRAM_DEEP_DISCHARGE_FLAG_s fram_deepDischargeFlags = {0};
/**@}*/

/**
 * The zeros are the uninitialized addresses of the variables
 * in the FRAM. They are initialized by the call of FRAM_Initialize()
 */
FRAM_BASE_HEADER_s fram_base_header[] = {
    {(void *)(&fram_version), sizeof(fram_version), 0},
    {(void *)(&fram_soc), sizeof(fram_soc), 0},
    {(void *)(&fram_sbcInit), sizeof(fram_sbcInit), 0},
    {(void *)(&fram_deepDischargeFlags), sizeof(fram_deepDischargeFlags), 0},
    {(void *)(&fram_soe), sizeof(fram_soe), 0},
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
