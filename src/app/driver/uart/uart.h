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
 * @file    uart.h
 * @author  foxBMS Team
 * @date    2025-08-12 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  UART
 *
 * @brief   Drivers for UART RS232
 * @details Provides the interface for the SCI/DMA setup, UART flow control,
 *          reception transmission.
 */

#ifndef FOXBMS__UART_H_
#define FOXBMS__UART_H_

/*========== Includes =======================================================*/

#include "HL_sci.h"

#include "os.h"

/*========== Macros and Definitions =========================================*/

/** Values of the SW flow control flags */
#define UART_XOFF (0x13u)
#define UART_XON  (0x11u)

/*========== Extern Constant and Variable Declarations ======================*/

/** a binary semaphore guarding the UART tx access */
extern OS_SEMAPHORE_HANDLE uart_txSemaphore;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Initialize the UART hardware with dedicated HAL functions.
 *          Has to be called before any call to the rest of this API.
 */
extern void UART_Initialize(void);

/**
 * @brief   Reads up to nrBytes number of bytes from the UART RX Queue.
 *          Careful when using the result readData as a C-string, since the
 *          data read does not get '\0' terminated.
 * @param   readData      buffer to write read data into
 * @param   nrBytes       maximum number of bytes to read
 * @return  the number of bytes actually read
 */
extern uint32_t UART_Read(uint8_t *const readData, const uint32_t nrBytes);

/**
 * @brief   Handles the flow control for UART in software.
 * @note    UART_HandleFlowControl is called from the UART task upon
 *          notification.
 *          Since the uart_txSemaphore has to be taken by this function, it is
 *          possible for priority inversion to occur between the UART task and
 *          another task attempting to acquire the semaphore, e.g. UART_Printf().
 */
extern void UART_HandleFlowControl(void);

/**
 * @brief   Prints via UART in a printf like fashion.
 *          This function also takes care of acquiring the uart_txSemaphore
 *          before calling UART_Write().
 * @param   pcFormatString format string
 */
extern void UART_Printf(const char *const pcFormatString, ...);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST

extern void TEST_UART_RxInterrupt(const sciBASE_t *const pSciInterface);
extern void TEST_UART_Write(sciBASE_t *const pSciInterface, const uint8_t *writeData, const uint16_t nrBytes);
extern bool TEST_UART_GetSending();
extern void TEST_UART_SetSending(const bool v);
extern bool TEST_UART_GetReceiving();
extern void TEST_UART_SetReceiving(const bool v);
extern uint8_t *TEST_UART_GetRxDataAddr();
extern void TEST_UART_SetRxData(const uint8_t b);
extern char *TEST_UART_GetTxBufferString();

#endif

#endif /* FOXBMS__UART_H_ */
