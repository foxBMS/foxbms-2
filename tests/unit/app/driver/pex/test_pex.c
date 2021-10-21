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
 * @file    test_pex.c
 * @author  foxBMS Team
 * @date    2021-09-29 (date of creation)
 * @updated 2021-09-29 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the port expander module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_i2c.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mocki2c.h"
#include "Mockos.h"
#include "Mockportmacro.h"

#include "pex_cfg.h"

#include "pex.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/

long FSYS_RaisePrivilege(void) {
    return 0;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testModuleInterfacesWithInvalidValues(void) {
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPin(5, PEX_PIN00));
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPin(PEX_PORT_EXPANDER1, 16u));

    TEST_ASSERT_FAIL_ASSERT(PEX_ResetPin(5, PEX_PIN00));
    TEST_ASSERT_FAIL_ASSERT(PEX_ResetPin(PEX_PORT_EXPANDER1, 16u));

    TEST_ASSERT_FAIL_ASSERT(PEX_GetPin(5, PEX_PIN00));
    TEST_ASSERT_FAIL_ASSERT(PEX_GetPin(PEX_PORT_EXPANDER1, 16u));

    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionInput(5, PEX_PIN00));
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionInput(PEX_PORT_EXPANDER1, 16u));

    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionOutput(5, PEX_PIN00));
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER1, 16u));

    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityInverted(5, PEX_PIN00));
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityInverted(PEX_PORT_EXPANDER1, 16u));

    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityRetained(5, PEX_PIN00));
    TEST_ASSERT_FAIL_ASSERT(PEX_SetPinPolarityRetained(PEX_PORT_EXPANDER1, 16u));
}
