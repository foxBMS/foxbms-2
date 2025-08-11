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
 * @file    test_fstartup.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Unit tests for the MCU startup routine of the application on the TI
 *          TMS570Lx43xx
 * @details Testing the startup procedure of the MCU is a relatively complex
 *          unit test. The startup procedure needs to the read a register
 *          (esmREG.SR1[...] defined in MockHL_reg_esm.h) to run mitigation
 *          strategies in case of errors. As reading these registers on the
 *          host platform is of course not possible, a workaround is needed.
 *          The workaround requires configuration in
 *          'conf/unit/bootloader_project_win32.yml'.
 *          This test can not be run on Linux, therefore **no**
 *          configuration is required in
 *          'conf/unit/bootloader_project_posix.yml'.
 *          The esmREG is mocked through
 *          'tests/unit/bootloader/main/helper/mock_esm.h'. This requires, that
 *          the test does **not** include 'MockHL_reg_esm.h', but only the
 *          mocked 'mock_esm.h'. This is achived by providing the include guard
 *          of 'MockHL_reg_esm.h' as compiler argument
 *          (gcc ... -D__REG_ESM_H__ ...).
 *          This further requires, that in the driver implementation
 *          'fstartup.c' includes 'mock_esm.h' (guarded for only the unit test
 *          case) first. By that, and by providing '-D__REG_ESM_H__' the driver
 *          compiles for the target as well as for the unit test with the
 *          correct implementation.
 *          'mock_esm.h' just keeps a minimal type configuration, just detailed
 *          enough to mock all required registers, but not more.
 *          The function to get the reset source of the MCU
 *          (STU_GetResetSourceWithoutFlagReset) is not tested, as this is
 *          based on register values, that can not be reasonably mocked on the
 *          host. Therefore, this unit test includes just a dummy function to
 *          emulate STU_GetResetSourceWithoutFlagReset and enables by that to
 *          check all startup cases.
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "mock_esm.h"

#include "HL_hal_stdtypes.h"

/* clang-format off */
#include "MockHL_sys_common.h"
#include "MockHL_hal_stdtypes.h"
#include "MockHL_system.h"
#include "MockHL_sys_vim.h"
#include "MockHL_sys_core.h"
#include "MockHL_reg_esm.h"
#include "MockHL_esm.h"
#include "MockHL_sys_mpu.h"
#include "MockHL_errata_SSWF021_45.h"
/* clang-format on */

#include "fstartup.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

TEST_INCLUDE_PATH("../../tests/unit/app/main/helper")
TEST_INCLUDE_PATH("C:/ti/Hercules/HALCoGen/v04.07.01/drivers/TMS570LC4357ZWT/SYSTEM570v000")

/*========== Definitions and Implementations for Unit Test ==================*/

esmBASE_t dummy   = {.SR1 = {0u, 0u, 0u}};
esmBASE_t *esmREG = &dummy;

#define DEFAULT_CASE (0xFFFFU)
static resetSource_t selector = POWERON_RESET;
extern resetSource_t STU_GetResetSourceWithoutFlagReset(void);

extern void __TI_auto_init(void) {
}

extern resetSource_t STU_GetResetSourceWithoutFlagReset(void) {
    resetSource_t returnValue;
    if (selector == POWERON_RESET) {
        returnValue = POWERON_RESET;
    }
    if (selector == OSC_FAILURE_RESET) {
        returnValue = OSC_FAILURE_RESET;
    }
    if (selector == WATCHDOG_RESET) {
        returnValue = WATCHDOG_RESET;
    }
    if (selector == WATCHDOG2_RESET) {
        returnValue = WATCHDOG2_RESET;
    }
    if (selector == DEBUG_RESET) {
        returnValue = DEBUG_RESET;
    }
    if (selector == INTERCONNECT_RESET) {
        returnValue = INTERCONNECT_RESET;
    }
    if (selector == CPU0_RESET) {
        returnValue = CPU0_RESET;
    }
    if (selector == SW_RESET) {
        returnValue = SW_RESET;
    }
    if (selector == EXT_RESET) {
        returnValue = EXT_RESET;
    }
    if (selector == NO_RESET) {
        returnValue = NO_RESET;
    }
    if (selector == DEFAULT_CASE) {
        returnValue = DEFAULT_CASE;
    }
    return returnValue;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Testing extern function _c_int00
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/11: POWERON_RESET restart and **no** ESM group3 errors
 *            - RT2/11:
 *            - RT3/11:
 *            - RT4/11:
 *            - RT5/11:
 *            - RT6/11:
 *            - RT7/11:
 *            - RT8/11:
 *            - RT9/11:
 */
void test_c_int00(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= None ======= */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/11: Test implementation */
    /* case POWERON_RESET: 'POWERON_RESET' is fallthrough and includes the
     *      cases 'DEBUG_RESET' and 'EXT_RESET'
     *      **no** ESM group3 errors
     */
    selector = POWERON_RESET;             /* set reset source: value to test for */
    STU_GetResetSourceWithoutFlagReset(); /* set reset source: sets value to desired 'selector' value */
    esmREG->SR1[2] = 0u;                  /* no ESM group3 errors */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _memInit__Expect();
    _coreEnableEventBusExport__Expect();
    systemInit_Expect();
    _coreEnableIrqVicOffset__Expect();
    vimInit_Expect();
    esmInit_Expect();
    _mpuInit__Expect();

    /* ======= RT1/11: call function under test */
    _c_int00();
    /* ======= RT1/11: test output verification */
    /* nothing to be verified */

    /* ======= RT2/11: Test implementation */
    /* case POWERON_RESET: 'POWERON_RESET' is fallthrough and includes the
     *      cases 'DEBUG_RESET' and 'EXT_RESET'
     *      ESM group3 errors
     */
    selector = POWERON_RESET;             /* set reset source: value to test for */
    STU_GetResetSourceWithoutFlagReset(); /* set reset source: sets value to desired 'selector' value */
    esmREG->SR1[2] = 1u;                  /* ESM group3 errors */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _memInit__Expect();
    _coreEnableEventBusExport__Expect();
    esmGroup3Notification_Expect(esmREG, esmREG->SR1[2u]);
    systemInit_Expect();
    _coreEnableIrqVicOffset__Expect();
    vimInit_Expect();
    esmInit_Expect();
    _mpuInit__Expect();

    /* ======= RT2/11: call function under test */
    _c_int00();
    /* ======= RT2/11: test output verification */
    /* nothing to be verified */

    /* ======= RT3/11: Test implementation */
    /* case DEBUG_RESET: 'DEBUG_RESET' is fallthrough and includes the case
     *      'EXT_RESET'
     *      **no** ESM group3 errors
     */
    selector = DEBUG_RESET;
    STU_GetResetSourceWithoutFlagReset(); /* returns value of 'selector' */
    esmREG->SR1[2] = 0u;                  /* no ESM group3 errors */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _memInit__Expect();
    _coreEnableEventBusExport__Expect();
    systemInit_Expect();
    _coreEnableIrqVicOffset__Expect();
    vimInit_Expect();
    esmInit_Expect();
    _mpuInit__Expect();

    /* ======= RT3/11: call function under test */
    _c_int00();

    /* ======= RT4/11: Test implementation */
    /* case OSC_FAILURE_RESET */
    selector = OSC_FAILURE_RESET;
    STU_GetResetSourceWithoutFlagReset(); /* returns value of 'selector' */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _mpuInit__Expect();

    /* ======= RT4/11: call function under test */
    _c_int00();

    /* ======= RT4/11: test output verification */
    /* nothing to be verified */

    /* ======= RT5/11: Test implementation */
    /* case WATCHDOG_RESET */
    selector = WATCHDOG_RESET;
    STU_GetResetSourceWithoutFlagReset(); /* returns value of 'selector' */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _mpuInit__Expect();

    /* ======= RT5/11: call function under test */
    _c_int00();

    /* ======= RT5/11: test output verification */
    /* nothing to be verified */

    /* ======= RT6/11: Test implementation */
    /* case WATCHDOG2_RESET */
    selector = WATCHDOG2_RESET;
    STU_GetResetSourceWithoutFlagReset(); /* returns value of 'selector' */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _mpuInit__Expect();

    /* ======= RT6/11: call function under test */
    _c_int00();

    /* ======= RT6/11: test output verification */
    /* nothing to be verified */

    /* ======= RT7/11: Test implementation */
    /* case CPU0_RESET */
    selector = CPU0_RESET;
    STU_GetResetSourceWithoutFlagReset(); /* returns value of 'selector' */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _coreEnableEventBusExport__Expect();
    _mpuInit__Expect();

    /* ======= RT7/11: call function under test */
    _c_int00();

    /* ======= RT7/11: test output verification */
    /* nothing to be verified */

    /* ======= RT8/11: Test implementation */
    /* case SW_RESET */
    selector = SW_RESET;
    STU_GetResetSourceWithoutFlagReset(); /* returns value of 'selector' */

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _mpuInit__Expect();

    /* ======= RT8/11: call function under test */
    _c_int00();

    /* ======= RT8/11: test output verification */
    /* nothing to be verified */

    /* ======= RT9/11: Test implementation */
    /* case default */
    selector = DEFAULT_CASE;
    STU_GetResetSourceWithoutFlagReset();

    _coreInitRegisters__Expect();
    _coreInitStackPointer__Expect();
    _mpuInit__Expect();

    /* ======= RT9/: call function under test */
    _c_int00();

    /* ======= RT9/: test output verification */
    /* nothing to be verified */
}
