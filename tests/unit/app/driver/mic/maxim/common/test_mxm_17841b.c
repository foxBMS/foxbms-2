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
 * @file    test_mxm_17841b.c
 * @author  foxBMS Team
 * @date    2020-06-22 (date of creation)
 * @updated 2020-06-24 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  MXM
 *
 * @brief   Test for the Maxim MAX17841B driver.
 *
 * @details def
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockfassert.h"
#include "Mockmxm_cfg.h"

#include "mxm_17841b.h"
#include "mxm_bitextract.h"

/*========== Definitions and Implementations for Unit Test ==================*/
static MXM_41B_INSTANCE_s mxm_41b_state = {
    .state              = MXM_STATEMACH_41B_UNINITIALIZED,
    .substate           = MXM_41B_ENTRY_SUBSTATE,
    .aliveCounter       = 0,
    .extendMessageBytes = 0,
    .waitCounter        = 0,
    .regRXIntEnable     = 0x00u,
    .regTXIntEnable     = 0x00u,
    .regRXStatus        = 0x00u,
    .regTXStatus        = 0x00u,
    .regConfig1         = 0x60u,
    .regConfig2         = 0x10u,
    .regConfig3         = 0x0Fu,
    .hwModel            = 0,
    .hwMaskRevision     = 0,
    .spiRXBuffer        = {0},
    .spiTXBuffer        = {0},
};

uint16_t commandBuffer[10] = {0};
uint16_t rxBuffer[100]     = {0};

MXM_41B_STATE_REQUEST_STATUS_e status41b = MXM_41B_STATE_UNPROCESSED;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    mxm_41b_state.state              = MXM_STATEMACH_41B_UNINITIALIZED;
    mxm_41b_state.substate           = MXM_41B_ENTRY_SUBSTATE;
    mxm_41b_state.aliveCounter       = 0;
    mxm_41b_state.extendMessageBytes = 0;
    mxm_41b_state.waitCounter        = 0;
    mxm_41b_state.regRXIntEnable     = 0x00u;
    mxm_41b_state.regTXIntEnable     = 0x00u;
    mxm_41b_state.regRXStatus        = 0x00u;
    mxm_41b_state.regTXStatus        = 0x00u;
    mxm_41b_state.regConfig1         = 0x60u;
    mxm_41b_state.regConfig2         = 0x10u;
    mxm_41b_state.regConfig3         = 0x0Fu;
    mxm_41b_state.hwModel            = 0;
    mxm_41b_state.hwMaskRevision     = 0;

    commandBuffer[0] = 0;
    commandBuffer[1] = 0;
    commandBuffer[2] = 0;
    commandBuffer[3] = 0;
    commandBuffer[4] = 0;
    commandBuffer[5] = 0;
    commandBuffer[6] = 0;
    commandBuffer[7] = 0;
    commandBuffer[8] = 0;
    commandBuffer[9] = 0;

    status41b = MXM_41B_STATE_UNPROCESSED;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/* tests for function MXM_41BSetStateRequest */

void testOnlyAllowedTransitionFromUnitializedIsInit(void) {
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(
            &mxm_41b_state, MXM_STATEMACH_41B_UNINITIALIZED, commandBuffer, 6, 0, rxBuffer, 100, &status41b));
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(
            &mxm_41b_state, MXM_STATEMACH_41B_MAXSTATE, commandBuffer, 6, 0, rxBuffer, 100, &status41b));
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(
            &mxm_41b_state, MXM_STATEMACH_41B_CHECK_FMEA, commandBuffer, 6, 0, rxBuffer, 100, &status41b));
    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, commandBuffer, 6, 0, rxBuffer, 100, &status41b));
}

void testTransitionsOnlyAllowedInIdleAfterInit(void) {
    /* start in Idle state */
    mxm_41b_state.state = MXM_STATEMACH_41B_IDLE;

    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BSetStateRequest(
            &mxm_41b_state, MXM_STATEMACH_41B_CHECK_FMEA, commandBuffer, 6, 0, rxBuffer, 100, &status41b));

    /* try additional transition (even though not in idle state anymore) */
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(
            &mxm_41b_state, MXM_STATEMACH_41B_CHECK_FMEA, commandBuffer, 6, 0, rxBuffer, 100, &status41b));
}

void testStateRequestConsistentInput(void) {
    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, commandBuffer, 6, 0, rxBuffer, 100, &status41b));
}

void testStateRequestInconsistentInputPayload(void) {
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, commandBuffer, 0, 0, rxBuffer, 100, &status41b));

    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, NULL_PTR, 6, 0, rxBuffer, 100, &status41b));

    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, NULL_PTR, 0, 0, rxBuffer, 100, &status41b));
}

void testStateRequestInconsistentInputRXBuffer(void) {
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, commandBuffer, 6, 0, rxBuffer, 0, &status41b));

    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, commandBuffer, 6, 0, NULL_PTR, 100, &status41b));

    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, commandBuffer, 6, 0, NULL_PTR, 0, &status41b));
}

void testStateRequestInconsistentInputProcessed(void) {
    TEST_ASSERT_EQUAL(
        STD_NOT_OK,
        MXM_41BSetStateRequest(&mxm_41b_state, MXM_STATEMACH_41B_INIT, commandBuffer, 6, 0, rxBuffer, 100, NULL_PTR));
}

/* end of tests for function MXM_41BSetStateRequest */

/* tests for function MXM_41BWriteRegisterFunction */
void testWriteRegisterFunctionWithIllegalValues(void) {
    TEST_ASSERT_EQUAL(STD_NOT_OK, MXM_41BWriteRegisterFunction(&mxm_41b_state, 0xFF, 0));
}
void testWriteRegisterFunctionWithLegalValues(void) {
    /* standard state should be only fourth bit set */
    TEST_ASSERT_EQUAL(0x10u, mxm_41b_state.regConfig2);
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_TX_PREAMBLES, MXM_41B_REG_FALSE));
    TEST_ASSERT_EQUAL(0x10u, mxm_41b_state.regConfig2);

    /* after writing register, fifth bit should be set in addition */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_TX_PREAMBLES, MXM_41B_REG_TRUE));
    TEST_ASSERT_EQUAL((0x10u | (1u << 5u)), mxm_41b_state.regConfig2);

    /* standard state should be third, second, first and zero bit set */
    TEST_ASSERT_EQUAL(0xFu, mxm_41b_state.regConfig3);

    /* 0us --> 0 */
    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_KEEP_ALIVE, MXM_41B_REG_KEEP_ALIVE_0US));
    TEST_ASSERT_EQUAL(0x0u, mxm_41b_state.regConfig3);

    /* 10us --> 1 */
    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_KEEP_ALIVE, MXM_41B_REG_KEEP_ALIVE_10US));
    TEST_ASSERT_EQUAL(0x1u, mxm_41b_state.regConfig3);

    /* 640us --> 0x7 */
    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_KEEP_ALIVE, MXM_41B_REG_KEEP_ALIVE_640US));
    TEST_ASSERT_EQUAL(0x7u, mxm_41b_state.regConfig3);

    /* INF DLY --> 0xF */
    TEST_ASSERT_EQUAL(
        STD_OK,
        MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_KEEP_ALIVE, MXM_41B_REG_KEEP_ALIVE_INF_DLY));
    TEST_ASSERT_EQUAL(0xFu, mxm_41b_state.regConfig3);

    /* standard state should be zero */
    TEST_ASSERT_EQUAL(0x0u, mxm_41b_state.regRXIntEnable);

    /* rx error int on --> 0x80 */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_ERROR_INT, MXM_41B_REG_TRUE));
    TEST_ASSERT_EQUAL(0x80u, mxm_41b_state.regRXIntEnable);

    /* rx error int off --> 0 */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_ERROR_INT, MXM_41B_REG_FALSE));
    TEST_ASSERT_EQUAL(0x0u, mxm_41b_state.regRXIntEnable);

    /* standard state should be zero */
    TEST_ASSERT_EQUAL(0x0u, mxm_41b_state.regRXIntEnable);

    /* rx overflow int on --> 0x8 */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_OVERFLOW_INT, MXM_41B_REG_TRUE));
    TEST_ASSERT_EQUAL(0x8u, mxm_41b_state.regRXIntEnable);

    /* rx overflow int off --> 0 */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BWriteRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_OVERFLOW_INT, MXM_41B_REG_FALSE));
    TEST_ASSERT_EQUAL(0x0u, mxm_41b_state.regRXIntEnable);
}
/* end of tests for function MXM_41BWriteRegisterFunction */

/* tests for function MXM_41BReadRegisterFunction */
void testReadRegisterFunctionWithIllegalValues(void) {
    TEST_ASSERT_EQUAL(STD_NOT_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, 0xFF, 0));
}
void testReadRegisterFunctionWithLegalValues(void) {
    MXM_41B_REG_BIT_VALUE result = 42;
    /* set register */
    mxm_41b_state.regRXStatus = 0u;

    /* read all functions in this register */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_BUSY_STATUS, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_FALSE, result);

    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_EMPTY_STATUS, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_FALSE, result);

    /* set register with bit RX_EMPTY */
    mxm_41b_state.regRXStatus = 1u;

    /* read all functions in this register */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_BUSY_STATUS, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_FALSE, result);

    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_EMPTY_STATUS, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_TRUE, result);

    /* set register with bit RX_BUSY */
    mxm_41b_state.regRXStatus = 0x20u;

    /* read all functions in this register */
    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_BUSY_STATUS, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_TRUE, result);

    TEST_ASSERT_EQUAL(
        STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_RX_EMPTY_STATUS, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_FALSE, result);

    /* set register to zero */
    mxm_41b_state.regConfig2 = 0u;

    /* read all functions in this register */
    TEST_ASSERT_EQUAL(STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_TX_PREAMBLES, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_FALSE, result);

    /* set register with TX_PREAMBLES */
    mxm_41b_state.regConfig2 = 0x20u;

    /* read all functions in this register */
    TEST_ASSERT_EQUAL(STD_OK, MXM_41BReadRegisterFunction(&mxm_41b_state, MXM_41B_REG_FUNCTION_TX_PREAMBLES, &result));
    TEST_ASSERT_EQUAL(MXM_41B_REG_TRUE, result);
}
/* end of tests for function MXM_41BReadRegisterFunction */

/* tests for the state-machine */
void testStateStayInUninitialized(void) {
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_UNINITIALIZED, mxm_41b_state.state);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_UNINITIALIZED, mxm_41b_state.state);
}

void testStateStayInIdle(void) {
    /* force state-machine in idle state */
    mxm_41b_state.state = MXM_STATEMACH_41B_IDLE;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_IDLE, mxm_41b_state.state);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_IDLE, mxm_41b_state.state);
}

void testStateCheckFMEA(void) {
    /* force state-machine in check_fmea state */
    mxm_41b_state.state = MXM_STATEMACH_41B_CHECK_FMEA;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CHECK_FMEA, mxm_41b_state.state);

    /* create a buffer with the assumed output of the ASCI and inject into SPI read */
    uint16_t rxBuffer[10] = {0x13u, 0x00u};
    MXM_ReceiveData_ExpectAndReturn(mxm_41b_state.spiTXBuffer, mxm_41b_state.spiRXBuffer, 2, STD_OK);
    MXM_ReceiveData_ReturnArrayThruPtr_rxBuffer(rxBuffer, 2);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CHECK_FMEA, mxm_41b_state.state);

    /* check if the values are parsed correctly */
    MXM_GetSPIStateReady_ExpectAndReturn(STD_OK);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_IDLE, mxm_41b_state.state);
    TEST_ASSERT_EQUAL(MXM_41B_STATE_PROCESSED, status41b);
}

void testStateCheckFMEAGNDLAlert(void) {
    /* force state-machine in check_fmea state */
    mxm_41b_state.state = MXM_STATEMACH_41B_CHECK_FMEA;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CHECK_FMEA, mxm_41b_state.state);

    /* create a buffer with the assumed output of the ASCI and inject into SPI read */
    /* simulate an GNDL Alert in FMEA register */
    uint16_t rxBuffer[10] = {0x13u, 0x01u};
    MXM_ReceiveData_ExpectAndReturn(mxm_41b_state.spiTXBuffer, mxm_41b_state.spiRXBuffer, 2, STD_OK);
    MXM_ReceiveData_ReturnArrayThruPtr_rxBuffer(rxBuffer, 2);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CHECK_FMEA, mxm_41b_state.state);

    /* check if the values are parsed correctly */
    MXM_GetSPIStateReady_ExpectAndReturn(STD_OK);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_IDLE, mxm_41b_state.state);
    TEST_ASSERT_EQUAL(MXM_41B_STATE_ERROR, status41b);
}

void testStateGetVersion(void) {
    /* force state-machine in get_version state */
    mxm_41b_state.state = MXM_STATEMACH_41B_GET_VERSION;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_GET_VERSION, mxm_41b_state.state);

    /* create a buffer with the assumed output of the ASCI and inject into SPI read */
    uint16_t rxBuffer[10] = {0x15u, 0x84u, 0x12u};
    MXM_ReceiveData_ExpectAndReturn(mxm_41b_state.spiTXBuffer, mxm_41b_state.spiRXBuffer, 3, STD_OK);
    MXM_ReceiveData_ReturnArrayThruPtr_rxBuffer(rxBuffer, 3);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_GET_VERSION, mxm_41b_state.state);

    /* check if the values are parsed correctly */
    MXM_GetSPIStateReady_ExpectAndReturn(STD_OK);
    MXM_41BStateMachine(&mxm_41b_state);
    TEST_ASSERT_EQUAL(0x841u, mxm_41b_state.hwModel);
    TEST_ASSERT_EQUAL(0x2u, mxm_41b_state.hwMaskRevision);
}

void testStateClearReceiveBuffer(void) {
    /* force state-machine in clear_receive_buffer state */
    mxm_41b_state.state = MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER, mxm_41b_state.state);

    /* create a buffer with the assumed input to the ASCI */
    uint16_t tx_buffer[10] = {0xE0u};
    MXM_SendData_ExpectAndReturn(tx_buffer, 1, STD_OK);
    MXM_41BStateMachine(&mxm_41b_state);

    /* check if the write was successful */
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_IDLE, mxm_41b_state.state);
    TEST_ASSERT_EQUAL(MXM_41B_STATE_PROCESSED, status41b);
}

void testStateClearReceiveBufferFailedSPIWrite(void) {
    /* force state-machine in clear_receive_buffer state */
    mxm_41b_state.state = MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER, mxm_41b_state.state);

    /* create a buffer with the assumed input to the ASCI */
    uint16_t tx_buffer[10] = {0xE0u};
    /* simulate a failure of writing */
    MXM_SendData_ExpectAndReturn(tx_buffer, 1, STD_NOT_OK);
    MXM_41BStateMachine(&mxm_41b_state);

    /* check if the write was successful */
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER, mxm_41b_state.state);
    TEST_ASSERT_EQUAL(MXM_41B_STATE_ERROR, status41b);
}

void testStateClearTransmitBuffer(void) {
    /* force state-machine in clear_transmit_buffer state */
    mxm_41b_state.state = MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER, mxm_41b_state.state);

    /* create a buffer with the assumed input to the ASCI */
    uint16_t tx_buffer[10] = {0x20u};
    MXM_SendData_ExpectAndReturn(tx_buffer, 1, STD_OK);
    MXM_41BStateMachine(&mxm_41b_state);

    /* check if the write was successful */
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_IDLE, mxm_41b_state.state);
    TEST_ASSERT_EQUAL(MXM_41B_STATE_PROCESSED, status41b);
}

void testStateClearTransmitBufferFailedSPIWrite(void) {
    /* force state-machine in clear_transmit_buffer state */
    mxm_41b_state.state = MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER;

    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER, mxm_41b_state.state);

    /* create a buffer with the assumed input to the ASCI */
    uint16_t tx_buffer[10] = {0x20u};
    /* simulate a failure of writing */
    MXM_SendData_ExpectAndReturn(tx_buffer, 1, STD_NOT_OK);
    MXM_41BStateMachine(&mxm_41b_state);

    /* check if the write was successful */
    TEST_ASSERT_EQUAL(MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER, mxm_41b_state.state);
    TEST_ASSERT_EQUAL(MXM_41B_STATE_ERROR, status41b);
}

void testStateUARTWaitForRXStatusChangeFail(void) {
    /* force state-machine in MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF */
    mxm_41b_state.state    = MXM_STATEMACH_41B_UART_TRANSACTION;
    mxm_41b_state.substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF;
    /* prepare RX buffer with not received RX_Stop_Status bit */
    mxm_41b_state.spiRXBuffer[1] = 0;

    /* transition */
    MXM_41BStateMachine(&mxm_41b_state);

    TEST_ASSERT_EQUAL(MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE, mxm_41b_state.substate);
}

void testStateUARTWaitForRXStatusChangeSuccess(void) {
    /* force state-machine in MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF */
    mxm_41b_state.state    = MXM_STATEMACH_41B_UART_TRANSACTION;
    mxm_41b_state.substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF;
    /* prepare RX buffer with  received RX_Stop_Status bit */
    mxm_41b_state.spiRXBuffer[1] |= (0x01u << 1u);

    /* transition */
    MXM_ReceiveData_IgnoreAndReturn(STD_OK);
    MXM_41BStateMachine(&mxm_41b_state);

    TEST_ASSERT_EQUAL(MXM_41B_UART_READ_BACK_RECEIVE_BUFFER_SAVE, mxm_41b_state.substate);
}

/* end tests for the state-machine */
