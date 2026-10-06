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
 * @file    test_adi_ades183x_mux.c
 * @author  foxBMS Team
 * @date    2022-04-20 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the mux functionality
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockadi_ades183x_helpers.h"
#include "Mockos.h"

#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_defs.h"
#include "adi_ades183x_mux.h"

/* clang-format off */
#include "test_assert_helper.h"
/* clang-format on */

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
/**
 * Default multiplexer measurement sequence
 * Must be adapted to the application
 */
ADI_MUX_CH_CFG_s adi_muxSequence[ADI_MUX_SEQUENCE_LENGTH] = {
    /*  multiplexer 0 measurement */
    {
        .muxId      = 0,
        .muxChannel = 0,
    },
    {
        .muxId      = 0,
        .muxChannel = 1,
    },
    {
        .muxId      = 0,
        .muxChannel = 2,
    },
    {
        .muxId      = 0,
        .muxChannel = 3,
    },
    {
        .muxId      = 0,
        .muxChannel = 4,
    },
    {
        .muxId      = 0,
        .muxChannel = 5,
    },
    {
        .muxId      = 0,
        .muxChannel = 6,
    },
    {
        .muxId      = 0,
        .muxChannel = 7,
    },
#if BS_NR_OF_TEMP_SENSORS_PER_MODULE > ADI_MUX_GPIOS_PER_MUX
    /* multiplexer 1 measurement: switch between multiplexers for more temperature sensors */
    {
        .muxId      = 0,
        .muxChannel = ADI_MUX_DISABLE_VALUE, /* disable enabled mux */
    },
    {
        .muxId      = 1,
        .muxChannel = 0,
    },
    {
        .muxId      = 1,
        .muxChannel = 1,
    },
    {
        .muxId      = 1,
        .muxChannel = 2,
    },
    {
        .muxId      = 1,
        .muxChannel = 3,
    },
    {
        .muxId      = 1,
        .muxChannel = 4,
    },
    {
        .muxId      = 1,
        .muxChannel = 5,
    },
    {
        .muxId      = 1,
        .muxChannel = 6,
    },
    {
        .muxId      = 1,
        .muxChannel = 7,
    },
    {
        .muxId      = 1,
        .muxChannel = ADI_MUX_DISABLE_VALUE, /* disable enabled mux */
    },
#endif
};

ADI_STATE_s adiTestState = {
    .currentString = 0u,
    .pMuxSequence  = {adi_muxSequence},
    .currentMux    = {0u},
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testADI_IncrementMuxIndex(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(ADI_IncrementMuxIndex(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    adiTestState.currentMux[adiTestState.currentString] = 0u;
    /* ======= RT1/2: call function under test */
    TEST_ASSERT_PASS_ASSERT(ADI_IncrementMuxIndex(&adiTestState));
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(1, adiTestState.currentMux[0]);

    /* ======= RT2/2: Test implementation */
    adiTestState.currentMux[adiTestState.currentString] = ADI_MUX_SEQUENCE_LENGTH;
    /* ======= RT2/2: call function under test */
    TEST_ASSERT_PASS_ASSERT(ADI_IncrementMuxIndex(&adiTestState));
    /* ======= RT2/2: test output verification */
    TEST_ASSERT_EQUAL(0u, adiTestState.currentMux[adiTestState.currentString]);
}

void testADI_ResetAllMuxIndices(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(ADI_ResetAllMuxIndices(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    ADI_STATE_s adiTestState = {
        .currentString = 0u,
        .pMuxSequence  = {0},
        .currentMux    = {1u},
    };
    /* ======= RT1/1 ======= */
    /* ======= RT2/2: call function under test */
    ADI_ResetAllMuxIndices(&adiTestState);

    TEST_ASSERT_PASS_ASSERT(ADI_ResetAllMuxIndices(&adiTestState));
    TEST_ASSERT_EQUAL(0, adiTestState.currentMux[0]);
    TEST_ASSERT_EQUAL(adiTestState.pMuxSequence[0], adi_muxSequence);
}

void testADI_SetMuxChannel(void) {
    uint8_t test_COMM_writeData[ADI_MAX_REGISTER_SIZE_IN_BYTES]             = {0x68, 0x00, 0x09, 0x00, 0x78, 0x00};
    uint8_t test_COMM_readData[ADI_MAX_REGISTER_SIZE_IN_BYTES]              = {0u};
    uint8_t test_COMM_readData_filled_ACKed[ADI_MAX_REGISTER_SIZE_IN_BYTES] = {0x67, 0x00, 0x09, 0x00, 0x78, 0x00};
    uint8_t test_paddingData[ADI_I2C_STCOMM_PADLEN]                         = {0u};

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(ADI_SetMuxChannel(NULL_PTR));
    /* ======= AT1/1 ======= */
    adiTestState.pMuxSequence[0]->muxId = 4;
    TEST_ASSERT_FAIL_ASSERT(ADI_SetMuxChannel(&adiTestState));
    adiTestState.pMuxSequence[0]->muxId = 0;

    /* ======= Routine tests =============================================== */
    ADI_ResetAllMuxIndices(&adiTestState);

    /* ======= RT1/1 ======= */
    /* Everything ok */
    ADI_WriteRegisterGlobal_Expect(adi_cmdWrcomm, test_COMM_writeData, ADI_PEC_NO_FAULT_INJECTION, &adiTestState);
    ADI_WriteRegisterGlobal_Expect(adi_cmdStcomm, test_paddingData, ADI_PEC_NO_FAULT_INJECTION, &adiTestState);

    ADI_CopyCommandBytes_Expect(adi_cmdRdcomm, adi_command);
    ADI_ReadRegister_Expect(adi_command, test_COMM_readData, &adiTestState);

    uint8_t dataI2c_expected = (uint8_t)(1u << (adiTestState.pMuxSequence[adiTestState.currentString]->muxChannel));
    test_COMM_readData_filled_ACKed[ADI_REGISTER_OFFSET3] = dataI2c_expected;
    ADI_ReadRegister_ReturnArrayThruPtr_data(test_COMM_readData_filled_ACKed, 6u);
    ADI_Wait_Expect(2u);

    TEST_ASSERT_EQUAL(STD_OK, ADI_SetMuxChannel(&adiTestState));
}
