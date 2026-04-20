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
 * @file    uart.c
 * @author  foxBMS Team
 * @date    2025-08-12 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  UART
 *
 * @brief   Drivers for UART RS232
 * @details Implementation of the SCI/DMA setup, UART flow control, receive and
 *          transmit interfaces.
 */

/* This file is only compiled when UART support is enabled.
 * This is controlled through the build process.
 * In case UART support is disabled, src/app/hal/app-hl_notification.c provides
 * a dummy implementation for 'sciNotification'. */

/*========== Includes =======================================================*/

#include "uart.h"

#include "dma_cfg.h"
#include "uart_cfg.h"

#include "HL_reg_dma.h"
#include "HL_reg_sci.h"
#include "HL_sci.h"
#include "HL_sys_common.h"
#include "HL_sys_dma.h"

#include "fassert.h"
#include "fstd_types.h"
#include "fsystem.h"
#include "ftask.h"
#include "os.h"
#include "stdarg.h"
#include "stdio.h"

/*========== Macros and Definitions =========================================*/

/** Upper and lower cutoff for UART receive flow control; will send XOFF once
 *  Upper limit is reached and XON once lower is reached
 */
#define UART_UPPER_CUTOFF               (0.7f) /* percentage */
#define UART_LOWER_CUTOFF               (0.3f) /* percentage */
#define UART_UPPER_CUTOFF_MESSAGE_COUNT ((uint32_t)((float_t)FTSK_UART_RX_QUEUE_LENGTH * UART_UPPER_CUTOFF))
#define UART_LOWER_CUTOFF_MESSAGE_COUNT ((uint32_t)((float_t)FTSK_UART_RX_QUEUE_LENGTH * UART_LOWER_CUTOFF))

#ifdef UNITY_UNIT_TEST
/* cspell:ignore RAMBASE */
dmaRAMBASE_t test_uartDmaReg = {0u};
#define UART_dmaRAMREG (&test_uartDmaReg)
#else
#define UART_dmaRAMREG (dmaRAMREG)
#endif

/*========== Static Constant and Variable Definitions =======================*/

/** indicates the MCU is allowed to send over UART, i.e. no XOFF received */
static volatile bool uart_softwareFlowControlSending = true;
/** indicates whether the MCU send a XOFF to stop receiving data */
static volatile bool uart_softwareFlowControlReceiving = true;

/** variable to hold the byte received via the SCI Rx interrupt (sciReceive) */
static uint8_t uart_rxData = 0u;

/** buffer to format UART_Printf into */
static uint8_t uart_txBuffer[UART_PRINTF_BUFFER_SIZE] = {0};

/*========== Extern Constant and Variable Definitions =======================*/

OS_SEMAPHORE_HANDLE uart_txSemaphore = NULL_PTR;

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Writes to the SCI Bus. After being called once, this function must
 *          not be called again, until the DMA transfer has been completed.
 * @param   pSciInterface SCI interface to use
 * @param   writeData     buffer containing the data to write
 * @param   nrBytes       number of bytes in the writeData buffer;
 *                        must be <= 0x1FFFh
 * @note    This function does not implement a locking mechanism itself. It is
 *          the responsibility of the caller to ensure no one else calls it
 *          until the DAM transfer is done.
 *          Therefor this function assumes that uart_txSemaphore has been taken
 *          before it is called.
 */
static void UART_Write(sciBASE_t *const pSciInterface, const uint8_t *const writeData, const uint16_t nrBytes);

/**
 * @brief   Called in case of SCI RX interrupt.
 * @param   pSciInterface SCI interface on which message was received
 */
static void UART_RxInterrupt(const sciBASE_t *const pSciInterface);

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void UART_Initialize(void) {
    sciInit();

    /** static memory for the uart_txSemaphore */
    static StaticSemaphore_t uart_txSemaphoreBuffer = {0};

    uart_txSemaphore = xSemaphoreCreateBinaryStatic(&uart_txSemaphoreBuffer);

    /* uart_txSemaphore should never be NULL_PTR here, since uart_txSemaphoreBuffer
     * is not NULL */
    FAS_ASSERT(uart_txSemaphore != NULL_PTR);

    /* Semaphores are created in the 'empty' state, meaning the semaphore must
     * first be given */
    /* TODO: only give the semaphore for the first time, after DMA has been
     * properly initialized */
    OS_SemaphoreGive(uart_txSemaphore);

    /* Starts interrupt driven rx */
    sciReceive(UART_REG, 1u, &uart_rxData);
}

static void UART_Write(sciBASE_t *const pSciInterface, const uint8_t *const writeData, const uint16_t nrBytes) {
    FAS_ASSERT(pSciInterface != NULL_PTR);
    FAS_ASSERT(writeData != NULL_PTR);
    FAS_ASSERT(nrBytes <= DMA_INITIAL_FRAME_COUNTER_MAX_VALUE);

    /* When there are 0 bytes to be transferred, no DMA transaction will be
     * initiated, which means there will never be a dmaGroupANotification for
     * UART Tx in which the uart_txSemaphore is released.
     * Therefore the uart_txSemaphore has to be released manually */
    if (nrBytes == 0u) {
        OS_SemaphoreGive(uart_txSemaphore);
    } else {
        /* Go to privileged mode to write DMA config registers */
        const int32_t raisePrivilegeResult = FSYS_RaisePrivilege();
        FAS_ASSERT(raisePrivilegeResult == 0);

        UART_dmaRAMREG->PCP[(dmaChannel_t)DMA_CHANNEL_SCI4_TX].ISADDR = (uint32_t)(&writeData[0u]);
        UART_dmaRAMREG->PCP[(dmaChannel_t)DMA_CHANNEL_SCI4_TX].ITCOUNT =
            (uint32_t)((uint32_t)((uint32_t)nrBytes << DMA_INITIAL_FRAME_COUNTER_POSITION) | (uint32_t)1u);

        /* Set the DMA channels to trigger on h/w request */
        dmaSetChEnable((dmaChannel_t)DMA_CHANNEL_SCI4_TX, (dmaTriggerType_t)DMA_HW);

        FSYS_SwitchToUserMode();

        /* Enable TX DMA */
        sciEnableNotification(pSciInterface, (uint32)((uint32_t)1u << UART_SCI_DMA_INTERRUPT));
    }
}

extern uint32_t UART_Read(uint8_t *const readData, const uint32_t nrBytes) {
    FAS_ASSERT(readData != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: nrBytes: accepts whole
     * range */

    uint32_t read = 0u;

    if (ftsk_allQueuesCreated) {
        const uint32_t numberOfStoredMessages = OS_GetNumberOfStoredMessagesInQueue(ftsk_uartRxQueue);

        if (numberOfStoredMessages > 0u) {
            while (read < nrBytes) {
                if (OS_ReceiveFromQueue(ftsk_uartRxQueue, (void *)&readData[read], 0u) != OS_SUCCESS) {
                    break;
                }
                read++;
            }

            /* Notify the UART flow control task, since the number of stored
             * messages has changed */
            OS_NotifyGive(UART_TASK_HANDLE);
        }
    }

    return read;
}

extern void UART_HandleFlowControl(void) {
    const uint32_t numberOfStoredMessages = OS_GetNumberOfStoredMessagesInQueue(ftsk_uartRxQueue);

    if ((uart_softwareFlowControlReceiving == true) && (numberOfStoredMessages >= UART_UPPER_CUTOFF_MESSAGE_COUNT)) {
        if (OS_SemaphoreTake(uart_txSemaphore, portMAX_DELAY) == OS_SUCCESS) {
            sciSendByte(UART_REG, UART_XOFF);
            uart_softwareFlowControlReceiving = false;
            OS_SemaphoreGive(uart_txSemaphore);
        }
    } else if (
        (uart_softwareFlowControlReceiving == false) && (numberOfStoredMessages <= UART_LOWER_CUTOFF_MESSAGE_COUNT)) {
        if (OS_SemaphoreTake(uart_txSemaphore, portMAX_DELAY) == OS_SUCCESS) {
            sciSendByte(UART_REG, UART_XON);
            uart_softwareFlowControlReceiving = true;
            OS_SemaphoreGive(uart_txSemaphore);
        }
    } else {
        /* Nothing to do here since we are within our set limits */
    }
}

static void UART_RxInterrupt(const sciBASE_t *const pSciInterface) {
    FAS_ASSERT(pSciInterface == UART_REG);

    if (uart_rxData == UART_XOFF) {
        uart_softwareFlowControlSending = false;
    } else if (uart_rxData == UART_XON) {
        uart_softwareFlowControlSending = true;
    } else if (ftsk_allQueuesCreated) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        (void)OS_SendToBackOfQueueFromIsr(ftsk_uartRxQueue, (void *)&uart_rxData, NULL_PTR);

        OS_NotifyGiveFromIsr(UART_TASK_HANDLE, &xHigherPriorityTaskWoken);
        FSYS_PORT_YIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        /* Nothing can be done here even though we received use data, but the
         * reception queue has not been created yet.
         */
    }
}

#if !defined(UNITY_UNIT_TEST) || defined(COMPILE_FOR_UNIT_TEST)
/* in the unit test case we mock 'HL_sci.h', so we have an implementation */
/* AXIVION Next Codeline Style CodingStyle-Naming.Parameter: keep the parameter
   names as provided by HALCoGen */
extern void sciNotification(sciBASE_t *sci, uint32 flags) {
    /* AXIVION Routine Generic-MissingParameterAssert: sci: unchecked in
     * interrupt */
    /* AXIVION Routine Generic-MissingParameterAssert: flags: unchecked in
     * interrupt */

    if ((sci == UART_REG) && (flags == (uint32)SCI_RX_INT)) {
        /* Handle received byte */
        UART_RxInterrupt(sci);
        /* Receive the next byte */
        sciReceive(sci, 1, &uart_rxData);
    }
}
#endif

/* AXIVION Next Codeline Generic-NoEllipsis: 'UART_Printf': is a printf style
 * function for sending formatted text via UART and therefore should accept a
 * variable number of formatting arguments
 */
extern void UART_Printf(const char *const pcFormatString, ...) {
    FAS_ASSERT(pcFormatString != NULL_PTR);

    /* Check if we are allowed to send data according to the flow control */
    if ((uart_softwareFlowControlSending) && (OS_SemaphoreTake(uart_txSemaphore, portMAX_DELAY) == OS_SUCCESS)) {
        va_list args = {0};
        va_start(args, pcFormatString);

        int32_t count = 0;

        /* AXIVION Next Codeline Generic-ForbiddenFunctions: Call to
         * 'vsnprintf' is necessary here to use standard C formatting */
        count = vsnprintf((char *)uart_txBuffer, UART_PRINTF_BUFFER_SIZE, pcFormatString, args);
        va_end(args);

        if (count > 0) {
            /* Ensure, that at most the entire buffer is written to UART
             * and not more, since vsnprintf returns the total number of
             * characters that would have been written, even if the string
             * was truncated due to the buffer size */
            if (count > (int32_t)UART_PRINTF_BUFFER_SIZE) {
                count = (int32_t)UART_PRINTF_BUFFER_SIZE;
            }

            UART_Write(UART_REG, (uint8_t *)uart_txBuffer, (uint16_t)count);
        } else {
            /* if UART_Write is not called, the semaphore has to be
             * returned explicitly */
            OS_SemaphoreGive(uart_txSemaphore);
        }
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

extern void TEST_UART_RxInterrupt(const sciBASE_t *const pSciInterface) {
    UART_RxInterrupt(pSciInterface);
}

extern void TEST_UART_Write(sciBASE_t *const pSciInterface, const uint8_t *writeData, const uint16_t nrBytes) {
    UART_Write(pSciInterface, writeData, nrBytes);
}

extern bool TEST_UART_GetSending() {
    return uart_softwareFlowControlSending;
}

extern void TEST_UART_SetSending(const bool v) {
    uart_softwareFlowControlSending = v;
}

extern bool TEST_UART_GetReceiving() {
    return uart_softwareFlowControlReceiving;
}

extern void TEST_UART_SetReceiving(const bool v) {
    uart_softwareFlowControlReceiving = v;
}

extern void TEST_UART_SetRxData(const uint8_t b) {
    uart_rxData = b;
}

extern uint8_t *TEST_UART_GetRxDataAddr() {
    return &uart_rxData;
}

extern char *TEST_UART_GetTxBufferString() {
    return (char *)uart_txBuffer;
}

#endif
