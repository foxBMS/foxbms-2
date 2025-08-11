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
 * @file    test_os_freertos_cache_enabled.c
 * @author  foxBMS Team
 * @date    2025-04-09 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the OS implementation for FreeRTOS
 * @details Test functions:
 *          - testOS_InitializeScheduler
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_sys_core.h"
#include "Mockcan_cbs_tx_crash-dump.h"
#include "Mockftask.h"
#include "Mockftask_cfg.h"
#include "Mockportmacro.h"
#include "Mockqueue.h"
#include "Mockrtc.h"
#include "Mocktask.h"

#include "os.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("os_freertos.c")

TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing extern function #OS_InitializeScheduler
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/2: Cache is disabled (tested in 'test_os_freertos_cache_disabled.c')
 *            - RT2/2: Cache is enabled (tested in 'test_os_freertos_cache_enabled.c')
 */
void testOS_InitializeScheduler(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */

    /* ======= RT1/2: see test_os_freertos_cache_disabled.c */

    /* ======= RT2/2: Test implementation */
    _cacheEnable__Expect();

    /* ======= RT2/2: call function under test */
    OS_InitializeScheduler();

    /* ======= RT2/2: test output verification */
    /* nothing to be verified */
}
