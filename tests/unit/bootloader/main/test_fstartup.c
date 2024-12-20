/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @date    2024-09-17 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Unit tests for the MCU startup routine of the bootloader on the TI
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

#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

TEST_INCLUDE_PATH("../../tests/unit/bootloader/main/helper")
TEST_INCLUDE_PATH("C:/ti/Hercules/HALCoGen/v04.07.01/drivers/TMS570LC4357ZWT/SYSTEM570v000")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void test_c_int00(void) {
}
