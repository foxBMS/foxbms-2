/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_adi_ades183x_diagnostic_w.c
 * @author  foxBMS Team
 * @date    2023-10-09 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of adi_ades183x_diagnostic_w.c
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag.h"
#include "Mockos.h"

#include "adi_ades183x_defs.h"
#include "adi_ades183x_diagnostic.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/

static ADI_ERROR_TABLE_s adi_errorTable = {0};

ADI_STATE_s adi_stateBase = {
    .data.errorTable = &adi_errorTable,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Test of ADI_Diagnostic
 * @details Cases:
 *          - Argument validation:
 *            - AT1/1: NULL state -> assertion
 */
void testADI_Diagnostic(void) {
    TEST_ASSERT_FAIL_ASSERT(ADI_Diagnostic(NULL_PTR));
}

/**
 * @brief   Test of ADI_EvaluateDiagnosticCellVoltages
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: NULL state and invalid module -> assertion
 *          - Routine validation:
 *            - RT1/3: CRC error -> not okay event
 *            - RT2/3: Stuck voltage register -> not okay event
 *            - RT3/3: Valid CRC and voltage register -> okay event
 */
void testADI_EvaluateDiagnosticCellVoltages(void) {
    const uint16_t module = 0u;
    const uint8_t string  = 0u;

    TEST_ASSERT_FAIL_ASSERT(ADI_EvaluateDiagnosticCellVoltages(NULL_PTR, module));
    TEST_ASSERT_FAIL_ASSERT(ADI_EvaluateDiagnosticCellVoltages(&adi_stateBase, BS_NR_OF_MODULES_PER_STRING));

    adi_stateBase.currentString            = string;
    adi_errorTable.crcIsOk[string][module] = false;
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_NOT_OK, DIAG_STRING, string, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_FALSE(ADI_EvaluateDiagnosticCellVoltages(&adi_stateBase, module));

    adi_errorTable.crcIsOk[string][module]                          = true;
    adi_errorTable.voltageRegisterContentIsNotStuck[string][module] = false;
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_NOT_OK, DIAG_STRING, string, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_FALSE(ADI_EvaluateDiagnosticCellVoltages(&adi_stateBase, module));

    adi_errorTable.voltageRegisterContentIsNotStuck[string][module] = true;
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_OK, DIAG_STRING, string, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_TRUE(ADI_EvaluateDiagnosticCellVoltages(&adi_stateBase, module));
}

/**
 * @brief   Test of ADI_EvaluateDiagnosticGpioVoltages
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: NULL state and invalid module -> assertion
 *          - Routine validation:
 *            - RT1/2: Invalid and valid CRC -> matching diagnostic event
 */
void testADI_EvaluateDiagnosticGpioVoltages(void) {
    const uint16_t module = 0u;
    const uint8_t string  = 0u;

    TEST_ASSERT_FAIL_ASSERT(ADI_EvaluateDiagnosticGpioVoltages(NULL_PTR, module));
    TEST_ASSERT_FAIL_ASSERT(ADI_EvaluateDiagnosticGpioVoltages(&adi_stateBase, BS_NR_OF_MODULES_PER_STRING));

    adi_stateBase.currentString            = string;
    adi_errorTable.crcIsOk[string][module] = false;
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_NOT_OK, DIAG_STRING, string, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_FALSE(ADI_EvaluateDiagnosticGpioVoltages(&adi_stateBase, module));

    adi_errorTable.crcIsOk[string][module] = true;
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_AFE_COMMUNICATION_INTEGRITY, DIAG_EVENT_OK, DIAG_STRING, string, DIAG_HANDLER_RETURN_OK);
    TEST_ASSERT_TRUE(ADI_EvaluateDiagnosticGpioVoltages(&adi_stateBase, module));
}

/**
 * @brief   Test of ADI_EvaluateDiagnosticStringAndModuleVoltages
 * @details Cases:
 *          - Argument validation:
 *            - AT1/2: NULL state and invalid module -> assertion
 *          - Routine validation:
 *            - RT1/3: CRC error, stuck register, and valid values
 */
void testADI_EvaluateDiagnosticStringAndModuleVoltages(void) {
    const uint16_t module = 0u;
    const uint8_t string  = 0u;

    TEST_ASSERT_FAIL_ASSERT(ADI_EvaluateDiagnosticStringAndModuleVoltages(NULL_PTR, module));
    TEST_ASSERT_FAIL_ASSERT(ADI_EvaluateDiagnosticStringAndModuleVoltages(&adi_stateBase, BS_NR_OF_MODULES_PER_STRING));

    adi_stateBase.currentString            = string;
    adi_errorTable.crcIsOk[string][module] = false;
    TEST_ASSERT_FALSE(ADI_EvaluateDiagnosticStringAndModuleVoltages(&adi_stateBase, module));

    adi_errorTable.crcIsOk[string][module]                            = true;
    adi_errorTable.auxiliaryRegisterContentIsNotStuck[string][module] = false;
    TEST_ASSERT_FALSE(ADI_EvaluateDiagnosticStringAndModuleVoltages(&adi_stateBase, module));

    adi_errorTable.auxiliaryRegisterContentIsNotStuck[string][module] = true;
    TEST_ASSERT_TRUE(ADI_EvaluateDiagnosticStringAndModuleVoltages(&adi_stateBase, module));
}

/**
 * @brief   Test of ADI_InitializeDiagnosis
 * @details Cases:
 *          - Routine validation:
 *            - RT1/1: State is accepted without side effects
 */
void testADI_InitializeDiagnosis(void) {
    ADI_InitializeDiagnosis(NULL_PTR);
    ADI_InitializeDiagnosis(&adi_stateBase);
}

/**
 * @brief   Test of ADI_IsFirstDiagnosticCycleFinished
 * @details Cases:
 *          - Argument validation:
 *            - AT1/1: NULL state -> assertion
 *          - Routine validation:
 *            - RT1/2: Return stored diagnostic status
 */
void testADI_IsFirstDiagnosticCycleFinished(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_IsFirstDiagnosticCycleFinished(NULL_PTR));

    adi_stateBase.firstDiagnosticMade = true;
    TEST_ASSERT_TRUE(TEST_ADI_IsFirstDiagnosticCycleFinished(&adi_stateBase));

    adi_stateBase.firstDiagnosticMade = false;
    TEST_ASSERT_FALSE(TEST_ADI_IsFirstDiagnosticCycleFinished(&adi_stateBase));
}

/**
 * @brief   Test of ADI_SetFirstDiagnosticCycleFinished
 * @details Cases:
 *          - Argument validation:
 *            - AT1/1: NULL state -> assertion
 *          - Routine validation:
 *            - RT1/1: Set status inside a critical section
 */
void testADI_SetFirstDiagnosticCycleFinished(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_SetFirstDiagnosticCycleFinished(NULL_PTR));

    adi_stateBase.firstDiagnosticMade = false;
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ADI_SetFirstDiagnosticCycleFinished(&adi_stateBase);
    TEST_ASSERT_TRUE(adi_stateBase.firstDiagnosticMade);
}
