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
 * @file    mxm_17841b.h
 * @author  foxBMS Team
 * @date    2018-12-14 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Headers for the driver for the MAX17841B ASCI and
 *          MAX1785x monitoring chip
 *
 * @details def
 *
 */

#ifndef FOXBMS__MXM_17841B_H_
#define FOXBMS__MXM_17841B_H_

/*========== Includes =======================================================*/
#include "mxm_cfg.h"

#include "mxm_bitextract.h"

/*========== Macros and Definitions =========================================*/

/**
 * @brief SPI TX buffer length
 *
 * This define defines the length of the SPI TX buffer.
 * This TX buffer is declared in #MXM_41B_INSTANCE_s::spiTXBuffer.
 * The buffer has to be large enough to accommodate every
 * relevant SPI transaction.
 */
#define MXM_SPI_TX_BUFFER_LENGTH (10u)

/** SPI RX buffer length */
#define MXM_SPI_RX_BUFFER_LENGTH (100u)

/**
 * @brief States of the MAX17841B state-machine
 */
typedef enum {
    MXM_STATEMACH_41B_UNINITIALIZED, /*!< Uninitialized state that the state-machine starts in */
    MXM_STATEMACH_41B_INIT,          /*!< Initialization sequence, afterwards transition into idle state */
    MXM_STATEMACH_41B_IDLE,          /*!< Idle state, transition into other states is available here */
    MXM_STATEMACH_41B_CHECK_FMEA,    /*!< Checks the FMEA register of MAX17841B. */
    MXM_STATEMACH_41B_GET_VERSION,   /*!< Retrieves the version of the connected ASCI. */
    MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER, /*!< Writes the copy of configuration and interrupt register to the MAX17841B. */
    MXM_STATEMACH_41B_READ_STATUS_REGISTER,  /*!< Reads the status registers of MAX17841B. */
    MXM_STATEMACH_41B_UART_TRANSACTION,      /*!< Sends a complete UART transaction. */
    MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER,  /*!< Clears the receive buffer. */
    MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER, /*!< Clears the transmit buffer */
    MXM_STATEMACH_41B_MAXSTATE,              /*!< Highest state */
} MXM_STATEMACH_41B_e;

/**
 * @brief Substates of the MAX17841B state-machine
 */
typedef enum {
    MXM_41B_ENTRY_SUBSTATE,
    MXM_41B_INIT_RESET_BRIDGE_IC,
    MXM_41B_INIT_START_BRIDGE_IC,
    MXM_41B_INIT_WRITE_DEFAULT_VALUES,
    MXM_41B_INIT_READ_CONFIG_REGISTERS,
    MXM_41B_INIT_CHECK_INITIALIZATION,
    MXM_41B_FMEA_REQUEST_REGISTER,
    MXM_41B_FMEA_VERIFY,
    MXM_41B_VERSION_REQUEST_REGISTER,
    MXM_41B_VERSION_VERIFY,
    /* MXM_41B_UART_CLEAR_TRANSMIT_BUFFER, */
    MXM_41B_UART_READ_RX_SPACE,
    MXM_41B_UART_READ_RX_SPACE_PARSE,
    MXM_41B_UART_WRITE_LOAD_QUEUE,
    MXM_41B_UART_READ_LOAD_QUEUE,
    MXM_41B_UART_VERIFY_LOAD_QUEUE_AND_TRANSMIT,
    MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE,
    MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF,
    MXM_41B_UART_READ_BACK_RECEIVE_BUFFER_SAVE,
    MXM_41B_READ_STATUS_REGISTER_SEND,
    MXM_41B_READ_STATUS_REGISTER_PROCESS,
} MXM_41B_SUBSTATES_e;

/**
 * @brief Request status of MAX17841B states.
 *
 * This enum describes the states that a state in the
 * #MXM_41BStateMachine() can have.
 */
typedef enum {
    MXM_41B_STATE_UNSENT,      /*!< The request has not been sent to the state-machine yet. */
    MXM_41B_STATE_UNPROCESSED, /*!< The request has been received by the state-machine, but not been processed yet. */
    MXM_41B_STATE_PROCESSED,   /*!< The request has been process successfully */
    MXM_41B_STATE_ERROR,       /*!< An error has occurred during processing of the request. */
} MXM_41B_STATE_REQUEST_STATUS_e;

/**
 * @brief Register functions
 */
typedef enum {
    MXM_41B_REG_FUNCTION_RX_BUSY_STATUS,     /*!< read the rx busy bit from rx status register */
    MXM_41B_REG_FUNCTION_RX_OVERFLOW_STATUS, /*!< read the rx overflow bit from rx status register */
    MXM_41B_REG_FUNCTION_RX_STOP_STATUS,     /*!< read the rx stop bit from rx status register */
    MXM_41B_REG_FUNCTION_RX_EMPTY_STATUS,    /*!< read the rx empty bit from rx status register */
    MXM_41B_REG_FUNCTION_TX_PREAMBLES,       /*!< set the tx preambles bit */
    MXM_41B_REG_FUNCTION_KEEP_ALIVE,         /*!< set the keep alive bits */
    MXM_41B_REG_FUNCTION_RX_ERROR_INT,       /*!< read the rx error bit */
    MXM_41B_REG_FUNCTION_RX_OVERFLOW_INT,
} MXM_41B_REG_FUNCTION_e;

/**
 * @brief Struct for the state-variable of state-machine
 *
 * This struct defines the state-variable of
 * the #MXM_41BStateMachine().
 */
typedef struct {
    MXM_STATEMACH_41B_e state;                 /*!< state of Driver State Machine */
    MXM_41B_SUBSTATES_e substate;              /*!< substate of current Driver State */
    uint16_t *pPayload;                        /*!< payload that is processed by the state-machine */
    uint8_t payloadLength;                     /*!< length of the payload array */
    uint16_t *pRxBuffer;                       /*!< pointer to an RX-buffer that will be filled by the state-machine */
    uint16_t rxBufferLength;                   /*!< length of the RX-buffer-array */
    MXM_41B_STATE_REQUEST_STATUS_e *processed; /*!< status-indicator of the state-machine */
    uint8_t extendMessageBytes;                /*!< pass on number of bytes by which the TX-message shall be extended */
    uint8_t waitCounter;                       /*!< general error counter, will be reset in funtions */
    uint8_t regRXIntEnable;                    /*!< local storage for the RX Interrupt Enable register */
    uint8_t regTXIntEnable;                    /*!< local storage for the TX Interrupt Enable register */
    uint8_t regRXStatus;                       /*!< local storage for the RX Status register */
    uint8_t regTXStatus;                       /*!< local storage for the TX status register */
    uint8_t regConfig1;                        /*!< local storage for the Config 1 register */
    uint8_t regConfig2;                        /*!< local storage for the Config 2 register */
    uint8_t regConfig3;                        /*!< local storage for the Config 3 register */
    uint8_t regRxSpace;                        /*!< RX space register (shows the left space in RX buffer) */
    uint16_t hwModel;                          /*!< model number of the connected IC */
    uint8_t hwMaskRevision;                    /*!< mask revision of the connected IC */
    uint32_t shutdownTimeStamp;                /*!< timestamp of the last shutdown (or startup) action */
    uint16_t spiRXBuffer[MXM_SPI_RX_BUFFER_LENGTH]; /*!< rx buffer for SPI */
    uint16_t spiTXBuffer[MXM_SPI_TX_BUFFER_LENGTH]; /*!< tx buffer for SPI */
} MXM_41B_INSTANCE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Write a register function
 *
 * Functions of the ASCI are mapped onto parts of the register.
 * This function is used to write the proper values into the register copy
 * of the ASCI that is inside the driver.
 * In order to use the function, provide the name of the ASCI function that you
 * want to write (#MXM_41B_REG_FUNCTION_e) and the value that
 * should be written (#MXM_41B_REG_BIT_VALUE).
 * Currently, only a relevant subset of the ASCI features is implemented.
 *
 * After updating the register copy, it has to be written to the ASCI memory.
 * For the configuration registers this is done with the function
 * MXM_41BConfigRegisterWrite(). This function is for example called
 * in state #MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER.
 *
 * @param[in,out]   pInstance           pointer to the state of the
 *                                      MAX17841B-state-machine
 * @param[in]       registerFunction    name of the function that shall be
 *                                      written
 * @param[in]       value               value of the bits that shall be written
 * @return          #STD_NOT_OK for unknown or unimplemented register
 *                  functions, otherwise #STD_OK
 */
extern STD_RETURN_TYPE_e MXM_41BWriteRegisterFunction(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_FUNCTION_e registerFunction,
    MXM_41B_REG_BIT_VALUE value);

/**
 * @brief Read the value of a register function
 *
 * Corresponding read-function to MXM_41BWriteRegisterFunction().
 * This function reads a value from the register-copy of MAX17841B and writes
 * it into a supplied pointer.
 * The register function is specified by the
 * corresponding #MXM_41B_REG_FUNCTION_e value.
 *
 * This function implements only the needed subset of register functions.
 *
 * @param[in,out]   kpkInstance         pointer to the state of the
 *                                      MAX17841B-state-machine
 * @param[in]       registerFunction    name of the function that shall be
 *                                      written
 * @param[out]      pValue              pointer to a variable in which the bits
 *                                      shall be written
 * @return          #STD_NOT_OK for unknown or unimplemented register function,
 *                  otherwise #STD_OK
 */
extern STD_RETURN_TYPE_e MXM_41BReadRegisterFunction(
    const MXM_41B_INSTANCE_s *const kpkInstance,
    MXM_41B_REG_FUNCTION_e registerFunction,
    MXM_41B_REG_BIT_VALUE *pValue);

/**
 * @brief Execute state-machine for the MAX17841B.
 *
 * This function executes the state-machine that communicates
 * with the MAX17841B over SPI.
 */
extern void MXM_41BStateMachine(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief Set state transition for MAX17841B-state-machine.
 *
 * This function is used to transition into another state of the
 * state-machine which is handled in MXM_41BStateMachine().
 * The user sets a state that should be transitioned into.
 * For most states a payload and a payload-length can be supplied.
 * The payload-length has to be consistent with the length in payload.
 * Typically, this supplied payload describes the byte-sequence that
 * should be written into the load-queue.
 *
 * For the case of a e.g. READALL-command the message-length has to
 * be stretched by the number of connected monitoring ICs times two.
 * This is achieved by passing on extendMessageBytes.
 *
 * In case that the state generates data that has to be handled in the
 * higher layers, the pointer pRxBuffer will be used. Please make sure that
 * the length of pRxBuffer is also consistent.
 *
 * In order to determine success or failure of the requested state,
 * the higher layer supplies a pointer into which the current status
 * of the requested state will be written.
 *
 * @param[in,out]   pInstance           pointer to the state of the
 *                                      MAX17841B-state-machine
 * @param[in]       state               state into which the state-machine
 *                                      shall transition
 * @param[in]       pPayload            pointer to an array with data that will
 *                                      be processed in the requested state
 * @param[in]       payloadLength       length of the payload-array
 * @param[in]       extendMessageBytes  number of bytes that shall be appended
 *                                      by the ASCI
 * @param[out]      pRxBuffer           pointer to an array that will be filled
 *                                      by the requested state with data that
 *                                      has to be processed in higher layers
 * @param[in]       rxBufferLength      length of the RX buffer array
 * @param[out]      processed           pointer of the status of the requested
 *                                      state
 * @return          #STD_NOT_OK for inconsistent input or forbidden state
 *                  transitions, otherwise #STD_OK
 */
extern STD_RETURN_TYPE_e MXM_41BSetStateRequest(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_STATEMACH_41B_e state,
    uint16_t *pPayload,
    uint8_t payloadLength,
    uint8_t extendMessageBytes,
    uint16_t *pRxBuffer,
    uint16_t rxBufferLength,
    MXM_41B_STATE_REQUEST_STATUS_e *processed);

/**
 * @brief   Initializes the state struct with default values
 * @details This function is called through the startup of the driver in order
 *          to ensure proper default values.
 * @param[out]  pInstance   instance of the state struct that shall be initialized
 */
extern void MXM_41BInitializeStateStruct(MXM_41B_INSTANCE_s *pInstance);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_17841B_H_ */
