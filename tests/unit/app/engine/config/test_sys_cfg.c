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
 * @file    test_sys_cfg.c
 * @author  foxBMS Team
 * @date    2020-04-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the sys_cfg
 * @details Tests sending a sys boot message
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcan_cbs_tx_debug-response.h"
#include "Mockcan_cfg.h"

#include "sys_cfg.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testSysSendBootMessage(void) {
    /* all debug responses are ok */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_TIMESTAMP, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END, STD_OK);
    SYS_SendBootMessage();

    /* all debug responses are not ok and trigger trap */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_START, STD_NOT_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO, STD_NOT_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH, STD_NOT_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID, STD_NOT_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER, STD_NOT_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION, STD_NOT_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_TIMESTAMP, STD_NOT_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_MAGIC_END, STD_NOT_OK);
    SYS_SendBootMessage();
}
