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
 * @file    test_uart.c
 * @author  foxBMS Team
 * @date    2025-08-25 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the UART driver
 * @details TODO
 *
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "MockHL_sci.h"
#include "MockHL_sys_dma.h"
#include "Mockftask.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"

#include "dma_cfg.h"
#include "uart_cfg.h"

#include "string.h"
#include "test_assert_helper.h"
#include "uart.h"

/*========== Unit Testing Framework Directives ==============================*/

TEST_SOURCE_FILE("uart.c")

TEST_INCLUDE_PATH("../../src/app/driver/uart")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

/** Upper and lower cutoff for UART receive flow control; will send XOFF once
 * Upper limit is reached and XON once lower */
const float uart_upperCutoff   = 0.7f; /* percentage */
const float uart_lowerCutoff   = 0.3f; /* percentage */
const uint32_t uart_upperLimit = (uint32_t)(FTSK_UART_RX_QUEUE_LENGTH * uart_upperCutoff);
const uint32_t uart_lowerLimit = (uint32_t)(FTSK_UART_RX_QUEUE_LENGTH * uart_lowerCutoff);

/** a binary semaphore guarding the UART tx access */
extern OS_SEMAPHORE_HANDLE uart_txSemaphore;

/** static memory for the uart_txSemaphore */
static StaticSemaphore_t uart_testTxSemaphoreBuffer;

/** declarations for ftsk externals */
OS_QUEUE ftsk_uartRxQueue           = NULL_PTR;
volatile bool ftsk_allQueuesCreated = true;
OS_TASK_HANDLE ftsk_taskHandleUart  = 0u;

static uint8_t uart_fsysRaisePrivilegeReturnValue = 0u;

long FSYS_RaisePrivilege(void) {
    return uart_fsysRaisePrivilegeReturnValue;
}

/*========== Setup and Teardown =============================================*/
void setup(void) {
    uart_fsysRaisePrivilegeReturnValue = 0;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testUART_Initialize(void) {
    /* Test: Failed to create txSemaphore */
    sciInit_Expect();
    MPU_xQueueGenericCreateStatic_ExpectAndReturn(
        (UBaseType_t)1u,
        semSEMAPHORE_QUEUE_ITEM_LENGTH,
        NULL,
        (&uart_testTxSemaphoreBuffer),
        queueQUEUE_TYPE_BINARY_SEMAPHORE,
        NULL_PTR);
    TEST_ASSERT_FAIL_ASSERT(UART_Initialize());

    /* Test: Successful initialization */
    sciInit_Expect();
    MPU_xQueueGenericCreateStatic_ExpectAndReturn(
        (UBaseType_t)1u,
        semSEMAPHORE_QUEUE_ITEM_LENGTH,
        NULL,
        (&uart_testTxSemaphoreBuffer),
        queueQUEUE_TYPE_BINARY_SEMAPHORE,
        (SemaphoreHandle_t)uart_txSemaphore);
    OS_SemaphoreGive_Expect(uart_txSemaphore);
    sciReceive_Expect(UART_REG, 1u, TEST_UART_GetRxDataAddr());
    UART_Initialize();
}

void testUART_Write(void) {
    const uint8_t data_length = 3u;
    uint8_t data[3]           = "abc";

    /* Assertion tests */
    TEST_ASSERT_FAIL_ASSERT(TEST_UART_Write(NULL_PTR, data, data_length));
    TEST_ASSERT_FAIL_ASSERT(TEST_UART_Write(UART_REG, NULL_PTR, data_length));
    TEST_ASSERT_FAIL_ASSERT(TEST_UART_Write(UART_REG, data, DMA_INITIAL_FRAME_COUNTER_MAX_VALUE + 1u));

    /* an assertion should happen, when privileges cannot be raised */
    uart_fsysRaisePrivilegeReturnValue = 1u;
    TEST_ASSERT_FAIL_ASSERT(TEST_UART_Write(UART_REG, data, data_length));

    /* Testing an empty transmission */
    OS_SemaphoreGive_Expect(uart_txSemaphore);
    TEST_UART_Write(UART_REG, data, 0u);

    /* Testing actual DMA transmission */
    uart_fsysRaisePrivilegeReturnValue = 0u;
    dmaSetChEnable_Expect((dmaChannel_t)DMA_CHANNEL_SCI4_TX, (dmaTriggerType_t)DMA_HW);
    sciEnableNotification_Expect(UART_REG, (uint32)((uint32_t)1u << UART_SCI_DMA_INTERRUPT));
    TEST_UART_Write(UART_REG, data, data_length);
}

void testUART_Read(void) {
    const uint32_t data_length = 3;
    uint8_t data[data_length];

    /* Assertion tests */
    TEST_ASSERT_FAIL_ASSERT(UART_Read(NULL_PTR, data_length));

    /* Test queues not created yet */
    ftsk_allQueuesCreated = false;
    TEST_ASSERT_EQUAL(0, UART_Read(data, data_length));

    /* Test empty Rx Queue */
    ftsk_allQueuesCreated = true;
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, 0);
    TEST_ASSERT_EQUAL(0, UART_Read(data, data_length));

    /* Test ideal case when there are enough bytes in the rx queue to fullfill
     * the read request */
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, data_length);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_uartRxQueue, (void *)&data[0], 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_uartRxQueue, (void *)&data[1], 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_uartRxQueue, (void *)&data[2], 0u, OS_SUCCESS);
    OS_NotifyGive_ExpectAndReturn(UART_TASK_HANDLE, pdPASS);
    TEST_ASSERT_EQUAL(data_length, UART_Read(data, data_length));

    /* Test case when there are not enough bytes in the rx queue to fullfill
     * the read request */
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, data_length - 1);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_uartRxQueue, (void *)&data[0], 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_uartRxQueue, (void *)&data[1], 0u, OS_SUCCESS);
    OS_ReceiveFromQueue_ExpectAndReturn(ftsk_uartRxQueue, (void *)&data[2], 0u, OS_FAIL);
    OS_NotifyGive_ExpectAndReturn(UART_TASK_HANDLE, pdPASS);
    TEST_ASSERT_EQUAL(data_length - 1, UART_Read(data, data_length));
}

void testUART_HandleFlowControl(void) {
    bool uart_softwareFlowControlReceiving_old = TEST_UART_GetReceiving();

    /* Test Queue message count in between upper and lower limit */
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, uart_lowerLimit + 1);
    UART_HandleFlowControl();
    TEST_ASSERT_EQUAL(uart_softwareFlowControlReceiving_old, TEST_UART_GetReceiving());

    /* Test empty Rx Queue and already receiving */
    TEST_UART_SetReceiving(true);
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, 0u);
    UART_HandleFlowControl();
    TEST_ASSERT_EQUAL(true, TEST_UART_GetReceiving());

    /* Test empty Rx Queue and not receiving, but failing to get
     * uart_txSemaphore */
    TEST_UART_SetReceiving(false);
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, 0u);
    OS_SemaphoreTake_ExpectAndReturn(uart_txSemaphore, portMAX_DELAY, OS_FAIL);
    UART_HandleFlowControl();
    TEST_ASSERT_EQUAL(false, TEST_UART_GetReceiving());

    /* Test empty Rx Queue and not receiving */
    TEST_UART_SetReceiving(false);
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, 0u);
    OS_SemaphoreTake_ExpectAndReturn(uart_txSemaphore, portMAX_DELAY, OS_SUCCESS);
    sciSendByte_Expect(UART_REG, UART_XON);
    OS_SemaphoreGive_Expect(uart_txSemaphore);
    UART_HandleFlowControl();
    TEST_ASSERT_EQUAL(true, TEST_UART_GetReceiving());

    /* Test Full Rx Queue and still receiving */
    TEST_UART_SetReceiving(true);
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, uart_upperLimit);
    OS_SemaphoreTake_ExpectAndReturn(uart_txSemaphore, portMAX_DELAY, OS_SUCCESS);
    sciSendByte_Expect(UART_REG, UART_XOFF);
    OS_SemaphoreGive_Expect(uart_txSemaphore);
    UART_HandleFlowControl();
    TEST_ASSERT_EQUAL(false, TEST_UART_GetReceiving());

    /* Test Full Rx Queue and still receiving, but failing to get
     * uart_txSemaphore */
    TEST_UART_SetReceiving(true);
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, uart_upperLimit);
    OS_SemaphoreTake_ExpectAndReturn(uart_txSemaphore, portMAX_DELAY, OS_FAIL);
    UART_HandleFlowControl();
    TEST_ASSERT_EQUAL(true, TEST_UART_GetReceiving());

    /* Test full Rx Queue and no longer receiving */
    TEST_UART_SetReceiving(false);
    OS_GetNumberOfStoredMessagesInQueue_ExpectAndReturn(ftsk_uartRxQueue, uart_upperLimit);
    UART_HandleFlowControl();
    TEST_ASSERT_EQUAL(false, TEST_UART_GetReceiving());
}

void testUART_RxInterrupt(void) {
    bool uart_softwareFlowControlSending_old = TEST_UART_GetSending();
    BaseType_t xHigherPriorityTaskWoken      = pdFALSE;
    BaseType_t xHigherPriorityTaskWokenTrue  = pdTRUE;

    /* Assertion Test */
    TEST_ASSERT_FAIL_ASSERT(TEST_UART_RxInterrupt(NULL_PTR));
    /* Test Queues not created yet */
    ftsk_allQueuesCreated = false;
    TEST_UART_RxInterrupt(UART_REG);
    TEST_ASSERT_EQUAL(uart_softwareFlowControlSending_old, TEST_UART_GetSending());

    ftsk_allQueuesCreated = true;

    /* Test received XOFF */
    TEST_UART_SetRxData(UART_XOFF);
    TEST_UART_RxInterrupt(UART_REG);
    TEST_ASSERT_EQUAL(false, TEST_UART_GetSending());

    /* Test received XON */
    TEST_UART_SetRxData(UART_XON);
    TEST_UART_RxInterrupt(UART_REG);
    TEST_ASSERT_EQUAL(true, TEST_UART_GetSending());

    /* Test receiving actual use data */
    TEST_UART_SetRxData('a');
    OS_SendToBackOfQueueFromIsr_ExpectAndReturn(ftsk_uartRxQueue, TEST_UART_GetRxDataAddr(), NULL_PTR, OS_SUCCESS);
    OS_NotifyGiveFromIsr_Expect(UART_TASK_HANDLE, &xHigherPriorityTaskWoken);
    TEST_UART_RxInterrupt(UART_REG);

    /* Test receiving actual use data, with yield */
    TEST_UART_SetRxData('a');
    OS_SendToBackOfQueueFromIsr_ExpectAndReturn(ftsk_uartRxQueue, TEST_UART_GetRxDataAddr(), NULL_PTR, OS_SUCCESS);
    OS_NotifyGiveFromIsr_Expect(UART_TASK_HANDLE, &xHigherPriorityTaskWoken);
    OS_NotifyGiveFromIsr_ReturnThruPtr_pHigherPriorityTaskWoken(&xHigherPriorityTaskWokenTrue);
    TEST_UART_RxInterrupt(UART_REG);
}

void testUART_Printf(void) {
    /* Assertion Test */
    TEST_ASSERT_FAIL_ASSERT(UART_Printf(NULL_PTR));

    /* Received XOFF previously */
    TEST_UART_SetSending(false);
    UART_Printf("Unit %s\n", "Test");

    /* Test not getting uart_txSemaphore */
    TEST_UART_SetSending(true);
    OS_SemaphoreTake_ExpectAndReturn(uart_txSemaphore, portMAX_DELAY, OS_FAIL);
    UART_Printf("Unit %s\n", "Test");

    /* print empty string */
    TEST_UART_SetSending(true);
    OS_SemaphoreTake_ExpectAndReturn(uart_txSemaphore, portMAX_DELAY, OS_SUCCESS);
    OS_SemaphoreGive_Expect(uart_txSemaphore);
    UART_Printf("");

    /* print proper string */
    TEST_UART_SetSending(true);
    OS_SemaphoreTake_ExpectAndReturn(uart_txSemaphore, portMAX_DELAY, OS_SUCCESS);
    /* simulate successful UART_Write() */
    uart_fsysRaisePrivilegeReturnValue = 0u;
    dmaSetChEnable_Expect((dmaChannel_t)DMA_CHANNEL_SCI4_TX, (dmaTriggerType_t)DMA_HW);
    sciEnableNotification_Expect(UART_REG, (uint32)((uint32_t)1u << UART_SCI_DMA_INTERRUPT));
    UART_Printf("Unit %s %d\n", "Test", 1);
    TEST_ASSERT_EQUAL(0, strcmp(TEST_UART_GetTxBufferString(), "Unit Test 1\n"));
}
