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
 * @file    test_contactor.c
 * @author  foxBMS Team
 * @date    2020-03-31 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the contactor.c module
 * @details Tests Initialization of Contactor registers
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcontactor_cfg.h"
#include "Mockdiag.h"
#include "Mockdiag_cfg.h"
#include "Mockfassert.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mocksps.h"
#include "Mocksps_cfg.h"

#include "contactor.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")

/*========== Definitions and Implementations for Unit Test ==================*/
BS_STRING_PRECHARGE_PRESENT_e bs_stringsWithPrecharge[BS_NR_OF_STRINGS] = {
    BS_STRING_WITH_PRECHARGE,
};

CONT_CONTACTOR_STATE_s cont_contactorStates[] = {
    /* String contactors configuration */
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_FEEDBACK_NORMALLY_OPEN,
     BS_STRING0,
     CONT_PLUS,
     SPS_CHANNEL_0,
     CONT_CHARGING_DIRECTION},
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_FEEDBACK_NORMALLY_OPEN,
     BS_STRING0,
     CONT_MINUS,
     SPS_CHANNEL_1,
     CONT_DISCHARGING_DIRECTION},
    /* Precharge contactors configuration */
    {CONT_SWITCH_OFF,
     CONT_SWITCH_OFF,
     CONT_HAS_NO_FEEDBACK,
     BS_STRING0,
     CONT_PRECHARGE,
     SPS_CHANNEL_2,
     CONT_BIDIRECTIONAL},
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testCONT_InitializationCheckOfContactorRegistry(void) {
    /* for the test we assume every contactor channel is correctly affiliated */
    for (CONT_CONTACTOR_INDEX contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
        SPS_GetChannelAffiliation_ExpectAndReturn(cont_contactorStates[contactor].spsChannel, SPS_AFF_CONTACTOR);
    }
    TEST_ASSERT_PASS_ASSERT(TEST_CONT_InitializationCheckOfContactorRegistry());
}

void testCONT_InitializationCheckOfContactorRegistryWrongAffiliation(void) {
    /* act as if a channel is wrongly affiliated */
    SPS_GetChannelAffiliation_ExpectAndReturn(cont_contactorStates[0].spsChannel, SPS_AFF_GENERAL_IO);
    TEST_ASSERT_FAIL_ASSERT(TEST_CONT_InitializationCheckOfContactorRegistry());
}

/**
 * @brief   Testing extern function #CONT_CheckFeedback
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - TODO
 */
void testCONT_CheckFeedback(void) {
    /* ======= Assertion tests ============================================= */

    /* ======= Routine tests =============================================== */
    DIAG_EVENT_e feedbackStatus = DIAG_EVENT_OK;

    /* ======= RT1/2 ======= */
    for (CONT_CONTACTOR_INDEX contactor = 0; contactor < BS_NR_OF_CONTACTORS - 1; contactor++) {
        SPS_GetChannelPexFeedback_ExpectAndReturn(cont_contactorStates[contactor].spsChannel, true, STD_OK);
    }
    /* Different DIAG_Handlers for all three test contactorStates */
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_STRING_PLUS_CONTACTOR_FEEDBACK,
        feedbackStatus,
        DIAG_STRING,
        (uint8_t)cont_contactorStates[0].stringIndex,
        STD_OK);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_STRING_MINUS_CONTACTOR_FEEDBACK,
        feedbackStatus,
        DIAG_STRING,
        (uint8_t)cont_contactorStates[1].stringIndex,
        STD_OK);
    DIAG_Handler_ExpectAndReturn(
        DIAG_ID_PRECHARGE_CONTACTOR_FEEDBACK,
        feedbackStatus,
        DIAG_STRING,
        (uint8_t)cont_contactorStates[2].stringIndex,
        STD_OK);

    CONT_CheckFeedback();
    /* ======= RT2/2 ======= */
    /*for (CONT_CONTACTOR_INDEX contactor = 0u; contactor < BS_NR_OF_CONTACTORS; contactor++) {
    }
    CONT_CheckFeedback();*/
}

/**
 * @brief   Testing extern function #CONT_OpenContactor
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid string number &rarr; assert
 *            - AT2/2: invalid contactor &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testCONT_OpenContactor(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS + 1;
    CONT_TYPE_e validContactor  = CONT_PLUS;
    CONT_OpenContactor(invalidStringNumber, validContactor);
    /* ======= AT1/2 ======= */
    uint8_t validStringNumber    = BS_NR_OF_STRINGS;
    CONT_TYPE_e invalidContactor = CONT_UNDEFINED;
    CONT_OpenContactor(validStringNumber, invalidContactor);

    /* ======= Routine tests =============================================== */
}

/**
 * @brief   Testing extern function #CONT_CloseContactor
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: invalid string number &rarr; assert
 *            - AT2/2: invalid contactor &rarr; assert
 *          - Routine validation:
 *            - TODO
 */
void testCONT_CloseContactor(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS + 1;
    CONT_TYPE_e validContactor  = CONT_PLUS;
    CONT_CloseContactor(invalidStringNumber, validContactor);
    /* ======= AT1/2 ======= */
    uint8_t validStringNumber    = BS_NR_OF_STRINGS;
    CONT_TYPE_e invalidContactor = CONT_UNDEFINED;
    CONT_CloseContactor(validStringNumber, invalidContactor);

    /* ======= Routine tests =============================================== */
}

/**
 * @brief   Testing extern function #CONT_ClosePrecharge
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid string number &rarr; assert
 *          - Routine validation:
 *            - RT1/1: precharge contactor shall be closed
 *            - RT1/2: TODO
 */
void testCONT_ClosePrecharge(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS + 1;
    CONT_ClosePrecharge(invalidStringNumber);

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2 ======= */
    uint8_t stringWithPrechargeContactor = 0u;
    uint8_t prechargeContactorIndex      = 2u;
    SPS_RequestContactorState_Expect(cont_contactorStates[prechargeContactorIndex].spsChannel, SPS_CHANNEL_ON);
    CONT_ClosePrecharge(stringWithPrechargeContactor);
}

/**
 * @brief   Testing extern function #CONT_OpenPrecharge
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: invalid string number &rarr; assert
 *          - Routine validation:
 *            - RT1/2: precharge contactor shall be closed
 *            - RT1/2: TODO
 */
void testCONT_OpenPrecharge(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    uint8_t invalidStringNumber = BS_NR_OF_STRINGS + 1;
    CONT_OpenPrecharge(invalidStringNumber);

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2 ======= */
    uint8_t stringWithPrechargeContactor = 0u;
    uint8_t prechargeContactorIndex      = 2u;
    SPS_RequestContactorState_Expect(cont_contactorStates[prechargeContactorIndex].spsChannel, SPS_CHANNEL_OFF);
    CONT_OpenPrecharge(stringWithPrechargeContactor);
}
