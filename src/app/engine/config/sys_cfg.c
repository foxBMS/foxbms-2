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
 * @file    sys_cfg.c
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  SYS
 *
 * @brief   Sys driver configuration
 */

/*========== Includes =======================================================*/
#include "sys_cfg.h"

#include "can_cbs_tx_debug-response.h"
#include "fassert.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void SYS_SendBootMessage(void) {
    /* The magic boot sequence on CAN is:
       - <magic boot ID>:         0xFE 0xFE 0xFE 0xFE 0xFE 0xFE 0xFE 0x0F
       - <bms version info ID>:   <what ever the version is>
       - <MCU die ID>:            <what ever the die ID is>
       - <MCU lot number>:        <what ever the lot number is>
       - <MCU wafer information>: <what ever the wafer information is>
       - <magic boot ID>:         0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x0F
    */
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
    if (CANTX_DebugResponse(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END) != STD_OK) {
        FAS_ASSERT(FAS_TRAP);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
