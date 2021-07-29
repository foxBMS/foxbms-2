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
 * @file    mxm_17841b.c
 * @author  foxBMS Team
 * @date    2018-12-14 (date of creation)
 * @updated 2021-06-16 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Driver for the MAX17841B ASCI and MAX1785x monitoring chip
 *
 * @details def
 *
 */

/*========== Includes =======================================================*/
#include "mxm_17841b.h"

#include "mxm_41b_register_map.h"
#include "mxm_bitextract.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief   Default values for the configuration and interrupt registers
 * @details This constant array contains the default values to which the
 *          configuration is compared when resetting it.
 *          The array is 7 entries long in order to fit onto the following
 *          registers which are placed in succession in the memory of
 *          MAX17841B:
 *              - RX_Interrupt_Enable
 *              - TX_Interrupt_Enable
 *              - RX_Interrupt_Flags
 *              - TX_Interrupt_Flags
 *              - Configuration_1
 *              - Configuration_2
 *              - Configuration_3
 */
static const uint8_t mxm_41B_reg_default_values[7] = {0, 0, 0, 0x80, 0x60, 0x10, 0x0F};

/*========== Extern Constant and Variable Definitions =======================*/
const uint8_t mxm_kConfig2EnableTransmitPreamblesMode41BRegister = 0x30;
const uint8_t mxm_kConfig3KeepAlive160us41BRegister              = 0x05;
const uint8_t mxm_kRXInterruptEnableRXErrorRXOverflow41BRegister = 0x88;

/*========== Static Function Prototypes =====================================*/
/* static MXM_SPI_STATE_s mxm_proto_enable_keep_alive(void); */
/**
 * @brief Write one or multiple registers of MAX17841B.
 *
 * This function puts together a SPI message consisting of command
 * and payload and writes it to the assigned SPI interface.
 *
 * The command should be one of the #MXM_41B_REG_ADD_t commands.
 * The function checks whether the chosen command is a write command.
 *
 * The payload-length has to be consistent with the payload.
 * Payload-lengths of one will write to one register only. Longer payloads
 * will write to adjacent registers. Please see the MAX17841B-datasheet
 * for reference.
 *
 * @param[in,out] pInstance pointer to the state of the MAX17841B-state-machine
 * @param[in] command register command of #MXM_41B_REG_ADD_t
 * @param[in] kpkPayload pointer to an array of data to be written
 * @param[in] lengthPayload length of the payload array
 * @return #STD_NOT_OK for inconsistent input or a blocked SPI interface,
 *         otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MXM_41BRegisterWrite(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_ADD_t command,
    const uint8_t *const kpkPayload,
    uint16_t lengthPayload);

/**
 * @brief Read one or multiple registers of MAX17841B.
 *
 * This function puts together a SPI message consisting of command
 * and bit-stuffing and writes it to the assigned SPI-interface.
 *
 * The command should be one of the #MXM_41B_REG_ADD_t commands.
 * The function checks whether the chosen command is a read command.
 *
 * The RX buffer length has to be consistent with the RX buffer.
 * RX buffer lengths of one will read one register only. Longer RX buffers
 * will read also from adjacent registers. Please see the MAX17841B-datasheet
 * for reference.
 *
 * @param[in,out] pInstance pointer to the state of the MAX17841B-state-machine
 * @param[in] command register command of #MXM_41B_REG_ADD_t
 * @param[out] pRxBuffer pointer to an array into which read data will be written
 * @param[in] length length of the RX buffer array
 * @return #STD_NOT_OK for inconsistent input or a blocked SPI interface,
 *         otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MXM_41BRegisterRead(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_ADD_t command,
    uint16_t *pRxBuffer,
    uint16_t length);

/**
 * @brief Write the config register of MAX17841B.
 *
 * This functions writes the config registers with the values
 * from the local register copies. It puts together a
 * buffer from these register values and calls MXM_41BRegisterWrite()
 * with this data.
 *
 * @param[in,out] pInstance state pointer
 * @return returnvalue of MXM_41BRegisterWrite()
 */
static STD_RETURN_TYPE_e MXM_41BConfigRegisterWrite(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief Write a buffer transaction to MAX17841B.
 *
 * Writes into the load-queue-buffer.
 * The supplied-message-length marks the length of the Battery Management
 * Protocol message without any stuffing-bytes for read-commands.
 * The extend_message parameter describes with how much bytes the
 * command shall be stretched. This number will be added to the
 * length of the command and written into the length field of the
 * buffer.
 * After this action the user has to select the next load queue with
 * the appropriate command in order to mark the load queue as sendable.
 *
 * @param[in,out] pInstance pointer to the state of the MAX17841B-state-machine
 * @param[in] kpkMessage pointer to an array containing the message
 * @param[in] message_length length of the supplied array
 * @param[in] extend_message stretch the message by number of bytes
 * @return #STD_NOT_OK for inconsistent input or a blocked SPI interface,
 *         otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MXM_41BBufferWrite(
    MXM_41B_INSTANCE_s *pInstance,
    const uint16_t *const kpkMessage,
    uint16_t message_length,
    uint8_t extend_message);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e MXM_41BRegisterWrite(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_ADD_t command,
    const uint8_t *const kpkPayload,
    uint16_t lengthPayload) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* check if command is a write command (write addresses in MAX17841B are even) */
    if ((command % 2u) == 0u) {
        /* construct tx buffer */
        pInstance->spiTXBuffer[0] = command;
        /* message has payload --> copy into buffer */
        if (kpkPayload != NULL_PTR) {
            for (uint16_t i = 0u; i < lengthPayload; i++) {
                pInstance->spiTXBuffer[i + 1u] = kpkPayload[i];
            }
            /* null rest of tx buffer */
            for (uint16_t i = lengthPayload + 1u; i < MXM_SPI_TX_BUFFER_LENGTH; i++) {
                pInstance->spiTXBuffer[i] = 0u;
            }
            /* send command with payload */
            retval = MXM_SendData(pInstance->spiTXBuffer, lengthPayload + 1u);
        } else {
            /* check if lengthPayload is consistent (should be 0) */
            if (lengthPayload != 0u) {
                retval = STD_NOT_OK;
            } else {
                /* send command without payload */
                retval = MXM_SendData(pInstance->spiTXBuffer, 1);
            }
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_41BRegisterRead(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_ADD_t command,
    uint16_t *pRxBuffer,
    uint16_t length) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    /* RX Buffer may not be NULL pointer for this function */
    FAS_ASSERT(pRxBuffer != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* check if command is a read command (read addresses in MAX17841B are odd) */
    if ((command % 2u) != 0u) {
        /* construct tx buffer */
        pInstance->spiTXBuffer[0] = command;
        /* null rest of tx buffer */
        for (uint16_t i = 1u; i < MXM_SPI_TX_BUFFER_LENGTH; i++) {
            pInstance->spiTXBuffer[i] = 0u;
        }
        /* send command with payload */
        retval = MXM_ReceiveData(pInstance->spiTXBuffer, pRxBuffer, length + 1u);
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_41BConfigRegisterWrite(MXM_41B_INSTANCE_s *pInstance) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    uint8_t mxm_spi_temp_buffer[10] = {0};
    /* TODO refactor so that int flags are not cleared by this function (because otherwise we would have to read them before every write) */
    mxm_spi_temp_buffer[0] = pInstance->regRXIntEnable;
    mxm_spi_temp_buffer[1] = pInstance->regTXIntEnable;
    mxm_spi_temp_buffer[2] = 0x00; /* TODO add reg_*x_int_flag */
    mxm_spi_temp_buffer[3] = 0x80;
    mxm_spi_temp_buffer[4] = pInstance->regConfig1;
    mxm_spi_temp_buffer[5] = pInstance->regConfig2;
    mxm_spi_temp_buffer[6] = pInstance->regConfig3;

    return MXM_41BRegisterWrite(pInstance, MXM_REG_RX_INTERRUPT_ENABLE_W, mxm_spi_temp_buffer, 7);
}

static STD_RETURN_TYPE_e MXM_41BBufferWrite(
    MXM_41B_INSTANCE_s *pInstance,
    const uint16_t *const kpkMessage,
    uint16_t message_length,
    uint8_t extend_message) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    /* check if message-pointer is valid */
    FAS_ASSERT(kpkMessage != NULL_PTR);
    FAS_ASSERT(message_length >= 1u);
    FAS_ASSERT(message_length <= 6u);

    /* write address and length to buffer */
    pInstance->spiTXBuffer[0] = (uint16_t)MXM_BUF_WR_LD_Q_0;
    pInstance->spiTXBuffer[1] = message_length + extend_message;
    /* iterate of complete TX buffer and
     * write into proper fields, null rest
     */
    for (uint8_t i = 0; i < (MXM_SPI_TX_BUFFER_LENGTH - 2u); i++) {
        if (i < message_length) {
            pInstance->spiTXBuffer[i + 2u] = kpkMessage[i];
        } else {
            pInstance->spiTXBuffer[i + 2u] = 0x00u;
        }
    }

    /* send data */
    return MXM_SendData(pInstance->spiTXBuffer, (message_length + 2u));
}

/*========== Extern Function Implementations ================================*/
STD_RETURN_TYPE_e MXM_41BSetStateRequest(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_STATEMACH_41B_e state,
    uint16_t *pPayload,
    uint16_t payloadLength,
    uint8_t extendMessageBytes,
    uint16_t *pRxBuffer,
    uint16_t rxBufferLength,
    MXM_41B_STATE_REQUEST_STATUS_e *processed) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;
    /* start by checking for input inconsistency */
    if (state >= MXM_STATEMACH_41B_MAXSTATE) {
        retval = STD_NOT_OK;
    } else if ((pPayload == NULL_PTR) && (payloadLength != 0u)) {
        retval = STD_NOT_OK;
    } else if ((payloadLength == 0u) && (pPayload != NULL_PTR)) {
        retval = STD_NOT_OK;
    } else if ((pRxBuffer == NULL_PTR) && (rxBufferLength != 0u)) {
        retval = STD_NOT_OK;
    } else if ((rxBufferLength == 0u) && (pRxBuffer != NULL_PTR)) {
        retval = STD_NOT_OK;
    } else if (processed == NULL_PTR) {
        retval = STD_NOT_OK;
    } else if (pInstance->state == MXM_STATEMACH_41B_UNINITIALIZED) {
        if (state == MXM_STATEMACH_41B_INIT) {
            pInstance->state              = state;
            pInstance->substate           = MXM_41B_ENTRY_SUBSTATE;
            pInstance->pPayload           = pPayload;
            pInstance->payloadLength      = payloadLength;
            pInstance->extendMessageBytes = extendMessageBytes;
            pInstance->pRxBuffer          = pRxBuffer;
            pInstance->rxBufferLength     = rxBufferLength;
            pInstance->processed          = processed;
            *pInstance->processed         = MXM_41B_STATE_UNPROCESSED;
        } else {
            retval = STD_NOT_OK;
        }
    } else if (pInstance->state == MXM_STATEMACH_41B_IDLE) {
        pInstance->state              = state;
        pInstance->substate           = MXM_41B_ENTRY_SUBSTATE;
        pInstance->pPayload           = pPayload;
        pInstance->payloadLength      = payloadLength;
        pInstance->extendMessageBytes = extendMessageBytes;
        pInstance->pRxBuffer          = pRxBuffer;
        pInstance->rxBufferLength     = rxBufferLength;
        pInstance->processed          = processed;
        *pInstance->processed         = MXM_41B_STATE_UNPROCESSED;
    } else {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e MXM_41BWriteRegisterFunction(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_FUNCTION_e registerFunction,
    MXM_41B_REG_BIT_VALUE value) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;
    /* TODO sanitize value */

    switch (registerFunction) {
        case MXM_41B_REG_FUNCTION_TX_PREAMBLES:
            pInstance->regConfig2 = mxm_41bWriteValue(
                value,
                1,
                MXM_41B_TX_PREAMBLES,
                pInstance->regConfig2); /* MXM_41B_TX_PREAMBLES is 5th bit of regConfig2 */
            break;
        case MXM_41B_REG_FUNCTION_KEEP_ALIVE:
            pInstance->regConfig3 = mxm_41bWriteValue(
                value, 4, MXM_41B_KEEP_ALIVE, pInstance->regConfig3); /* MXM_41B_KEEP_ALIVE is 0st bit of regConfig3 */
            break;
        case MXM_41B_REG_FUNCTION_RX_ERROR_INT:
            pInstance->regRXIntEnable = mxm_41bWriteValue(
                value,
                1,
                MXM_41B_RX_ERROR,
                pInstance->regRXIntEnable); /* MXM_41B_RX_ERROR is 7th bit of  regRXIntEnable */
            break;
        case MXM_41B_REG_FUNCTION_RX_OVERFLOW_INT:
            pInstance->regRXIntEnable = mxm_41bWriteValue(
                value,
                1,
                MXM_41B_RX_OVERFLOW_INT_ENABLE,
                pInstance->regRXIntEnable); /* MXM_41B_RX_OVERFLOW is 2nd bit of  regRXIntEnable */
            break;
        default:
            retval = STD_NOT_OK;
            break;
    }

    return retval;
}

extern STD_RETURN_TYPE_e MXM_41BReadRegisterFunction(
    const MXM_41B_INSTANCE_s *const kpkInstance,
    MXM_41B_REG_FUNCTION_e registerFunction,
    MXM_41B_REG_BIT_VALUE *pValue) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(kpkInstance != NULL_PTR);
    /* sanity check: pValue may not be null */
    FAS_ASSERT(pValue != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;

    switch (registerFunction) {
        case MXM_41B_REG_FUNCTION_RX_BUSY_STATUS:
            *pValue = mxm_41bReadValue(kpkInstance->regRXStatus, 1, MXM_41B_RX_BUSY_STATUS); /* 5th bit */
            break;
        case MXM_41B_REG_FUNCTION_RX_STOP_STATUS:
            *pValue = mxm_41bReadValue(kpkInstance->regRXStatus, 1, MXM_41B_RX_STOP_STATUS); /* 1st bit */
            break;
        case MXM_41B_REG_FUNCTION_RX_EMPTY_STATUS:
            *pValue = mxm_41bReadValue(kpkInstance->regRXStatus, 1, MXM_41B_RX_EMPTY_STATUS); /* 0th bit */
            break;
        case MXM_41B_REG_FUNCTION_TX_PREAMBLES:
            *pValue = mxm_41bReadValue(kpkInstance->regConfig2, 1, MXM_41B_TX_PREAMBLES); /* 5th bit */
            break;
        default:
            *pValue = MXM_41B_REG_FALSE;
            retval  = STD_NOT_OK;
            break;
    }

    return retval;
}

/* not used currently
 * extern uint8_t mxm_41b_get_alive_counter(void) {
 *  return pInstance->aliveCounter;
 * }
 */

void MXM_41BStateMachine(MXM_41B_INSTANCE_s *pInstance) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->waitCounter > 200u) {
        /* error, reset to idle state */
        pInstance->state       = MXM_STATEMACH_41B_IDLE;
        pInstance->substate    = MXM_41B_ENTRY_SUBSTATE;
        pInstance->waitCounter = 0u;
        *pInstance->processed  = MXM_41B_STATE_ERROR;
    }
    STD_RETURN_TYPE_e retval;
    switch (pInstance->state) {
        case MXM_STATEMACH_41B_UNINITIALIZED:

            break;
        case MXM_STATEMACH_41B_INIT:
            if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance->substate = MXM_41B_INIT_WRITE_DEFAULT_VALUES;
            }

            if (pInstance->substate == MXM_41B_INIT_WRITE_DEFAULT_VALUES) {
                /* set default register values according to datasheet */
                /* TODO reset register variables to default values */
                retval = MXM_41BConfigRegisterWrite(pInstance);
                if (retval == STD_OK) {
                    pInstance->substate = MXM_41B_INIT_READ_CONFIG_REGISTERS;
                }
            } else if (pInstance->substate == MXM_41B_INIT_READ_CONFIG_REGISTERS) {
                retval = MXM_41BRegisterRead(pInstance, MXM_REG_RX_INTERRUPT_ENABLE_R, pInstance->spiRXBuffer, 7);

                if (retval == STD_OK) {
                    pInstance->substate = MXM_41B_INIT_CHECK_INITIALIZATION;
                }
            } else if (pInstance->substate == MXM_41B_INIT_CHECK_INITIALIZATION) {
                retval = STD_OK;

                if (MXM_GetSPIStateReady() == STD_OK) {
                    for (uint8_t i = 0; i < 7u; i++) {
                        if (pInstance->spiRXBuffer[i + 1u] != mxm_41B_reg_default_values[i]) {
                            retval = STD_NOT_OK;
                        }
                    }
                } else {
                    retval = STD_NOT_OK;
                }

                if (retval == STD_NOT_OK) {
                    /* ERR
                 * TODO could not write config */
                    *pInstance->processed = MXM_41B_STATE_ERROR;
                } else {
                    /* TODO advance to next state or idle state */
                    pInstance->state      = MXM_STATEMACH_41B_IDLE;
                    *pInstance->processed = MXM_41B_STATE_PROCESSED;
                }
            } else {
                /* something is very broken */
                *pInstance->processed = MXM_41B_STATE_ERROR;
            }
            break;
        case MXM_STATEMACH_41B_IDLE:
            /* do nothing in idle state
         * TODO maybe trigger error checking from here e.g. every 100th cycle */
            /* clean up substate */
            pInstance->substate = MXM_41B_ENTRY_SUBSTATE;
            break;
        case MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER:
            retval = MXM_41BConfigRegisterWrite(pInstance);

            if (retval == STD_NOT_OK) {
                /* ERR
                 * TODO could not write config; later on implement retry */
                *pInstance->processed = MXM_41B_STATE_ERROR;
            } else {
                /* TODO advance to next state or idle state */
                pInstance->state      = MXM_STATEMACH_41B_IDLE;
                *pInstance->processed = MXM_41B_STATE_PROCESSED;
            }
            break;
        case MXM_STATEMACH_41B_READ_STATUS_REGISTER:
            /* TODO read status register and parse into static variables */
            if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance->substate = MXM_41B_READ_STATUS_REGISTER_SEND;
            }

            if (pInstance->substate == MXM_41B_READ_STATUS_REGISTER_SEND) {
                /* read rx and tx status register */
                retval = MXM_41BRegisterRead(pInstance, MXM_REG_RX_STATUS_R, pInstance->spiRXBuffer, 2);
                if (retval == STD_NOT_OK) {
                    /* TODO error handling and progress to ERROR state */
                } else {
                    pInstance->substate = MXM_41B_READ_STATUS_REGISTER_PROCESS;
                }
            } else if (pInstance->substate == MXM_41B_READ_STATUS_REGISTER_PROCESS) {
                pInstance->regRXStatus = (uint8_t)pInstance->spiRXBuffer[1];
                pInstance->regTXStatus = (uint8_t)pInstance->spiRXBuffer[2];
                pInstance->state       = MXM_STATEMACH_41B_IDLE;
                *pInstance->processed  = MXM_41B_STATE_PROCESSED;
            } else {
                /* something is very broken */
                *pInstance->processed = MXM_41B_STATE_ERROR;
            }
            break;
        case MXM_STATEMACH_41B_UART_TRANSACTION:
            if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance->substate = MXM_41B_UART_WRITE_LOAD_QUEUE;
            }

            if (pInstance->substate == MXM_41B_UART_WRITE_LOAD_QUEUE) {
                /* load queue with message */
                retval = MXM_41BBufferWrite(
                    pInstance, pInstance->pPayload, pInstance->payloadLength, pInstance->extendMessageBytes);

                if (retval == STD_NOT_OK) {
                    /* TODO error handling */
                    *pInstance->processed = MXM_41B_STATE_ERROR;
                } else {
                    pInstance->substate = MXM_41B_UART_READ_LOAD_QUEUE;
                }
            } else if (pInstance->substate == MXM_41B_UART_READ_LOAD_QUEUE) {
                /* send read load queue */
                retval = MXM_41BRegisterRead(
                    pInstance, MXM_BUF_RD_LD_Q_0, pInstance->spiRXBuffer, pInstance->payloadLength + 1u);

                if (retval == STD_NOT_OK) {
                    /* TODO error handling */
                    *pInstance->processed = MXM_41B_STATE_ERROR;
                } else {
                    pInstance->substate = MXM_41B_UART_VERIFY_LOAD_QUEUE_AND_TRANSMIT;
                }
            } else if (pInstance->substate == MXM_41B_UART_VERIFY_LOAD_QUEUE_AND_TRANSMIT) {
                /* verify load queue */
                retval = STD_OK;
                /* check message length */
                if (pInstance->spiRXBuffer[1] != (pInstance->payloadLength + pInstance->extendMessageBytes)) {
                    retval = STD_NOT_OK;
                }
                for (uint8_t i = 0; i < pInstance->payloadLength; i++) {
                    if (pInstance->spiRXBuffer[i + 2u] != pInstance->pPayload[i]) {
                        /* message corrupted during SPI transfer */
                        retval = STD_NOT_OK;
                    }
                }
                if (retval == STD_NOT_OK) {
                    /* TODO error handling
                 * transfer again? */
                    *pInstance->processed = MXM_41B_STATE_ERROR;
                } else {
                    /* transmit queue */
                    retval = MXM_41BRegisterWrite(pInstance, MXM_BUF_WR_NXT_LD_Q_0, NULL_PTR, 0);

                    if (retval == STD_NOT_OK) {
                        /* TODO error handling
                     * transfer again? */
                        *pInstance->processed = MXM_41B_STATE_ERROR;
                    } else {
                        pInstance->substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE;
                    }
                }
            } else if (pInstance->substate == MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE) {
                /* poll RX status change */
                retval = MXM_41BRegisterRead(pInstance, MXM_REG_RX_STATUS_R, pInstance->spiRXBuffer, 1);

                if (retval == STD_NOT_OK) {
                    /* TODO error handling
                 * transfer again? */
                    *pInstance->processed = MXM_41B_STATE_ERROR;
                } else {
                    pInstance->substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF;
                }
            } else if (pInstance->substate == MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF) {
                /* update RX status register copy with received buffer */
                pInstance->regRXStatus = (uint8_t)pInstance->spiRXBuffer[1];
                /* check if RX_STOP_Status is 1 */
                MXM_41B_REG_BIT_VALUE rx_stop_status_value = MXM_41B_REG_FALSE;
                MXM_41BReadRegisterFunction(pInstance, MXM_41B_REG_FUNCTION_RX_STOP_STATUS, &rx_stop_status_value);
                if (rx_stop_status_value == MXM_41B_REG_TRUE) {
                    /* received full UART frame --> continue */
                    /* read back receive buffer */
                    retval = MXM_41BRegisterRead(
                        pInstance,
                        MXM_BUF_RD_NXT_MSG,
                        pInstance->spiRXBuffer,
                        pInstance->payloadLength + 1u + pInstance->extendMessageBytes);

                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                        *pInstance->processed = MXM_41B_STATE_ERROR;
                    } else {
                        pInstance->substate    = MXM_41B_UART_READ_BACK_RECEIVE_BUFFER_SAVE;
                        pInstance->waitCounter = 0u;
                    }
                } else {
                    /* no UART frame received yet --> check again */
                    pInstance->substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE;
                    pInstance->waitCounter++;
                }
            } else if (pInstance->substate == MXM_41B_UART_READ_BACK_RECEIVE_BUFFER_SAVE) {
                for (uint8_t i = 0; i < (pInstance->payloadLength + pInstance->extendMessageBytes); i++) {
                    if (i < pInstance->rxBufferLength) {
                        pInstance->pRxBuffer[i] = pInstance->spiRXBuffer[i + 1u];
                    }
                }

                pInstance->state      = MXM_STATEMACH_41B_IDLE;
                *pInstance->processed = MXM_41B_STATE_PROCESSED;

            } else {
                /* TODO we should not be here */
            }
            break;
        case MXM_STATEMACH_41B_CHECK_FMEA:
            retval = STD_NOT_OK;
            if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
                pInstance->substate = MXM_41B_FMEA_REQUEST_REGISTER;
            }

            if (pInstance->substate == MXM_41B_FMEA_REQUEST_REGISTER) {
                retval = MXM_41BRegisterRead(pInstance, MXM_REG_FMEA_R, pInstance->spiRXBuffer, 1);

                if (retval == STD_OK) {
                    pInstance->substate = MXM_41B_FMEA_VERIFY;
                }
            } else if (pInstance->substate == MXM_41B_FMEA_VERIFY) {
                if (MXM_GetSPIStateReady() == STD_OK) {
                    if (pInstance->spiRXBuffer[1] == 0u) {
                        retval = STD_OK;
                    } else {
                        retval = STD_NOT_OK;
                    }
                }

                if (retval == STD_NOT_OK) {
                    /* ERR
                 * TODO FMEA check went bad */
                    pInstance->state      = MXM_STATEMACH_41B_IDLE;
                    *pInstance->processed = MXM_41B_STATE_ERROR;
                } else {
                    /* TODO advance to next state or idle state and set notice of good FMEA check somewhere */
                    pInstance->state      = MXM_STATEMACH_41B_IDLE;
                    *pInstance->processed = MXM_41B_STATE_PROCESSED;
                }
            } else {
                /* something is very broken */
                *pInstance->processed = MXM_41B_STATE_ERROR;
            }
            break;
        case MXM_STATEMACH_41B_GET_VERSION:
            if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
                pInstance->substate = MXM_41B_VERSION_REQUEST_REGISTER;
            }

            if (pInstance->substate == MXM_41B_VERSION_REQUEST_REGISTER) {
                /* read two byte in order to read also the adjacent version register */
                retval = MXM_41BRegisterRead(pInstance, MXM_REG_MODEL_R, pInstance->spiRXBuffer, 2);

                if (retval == STD_OK) {
                    pInstance->substate = MXM_41B_VERSION_VERIFY;
                }
            } else if (pInstance->substate == MXM_41B_VERSION_VERIFY) {
                if (MXM_GetSPIStateReady() == STD_OK) {
                    /* model is in model byte and high nibble of version byte */
                    pInstance->hwModel = (pInstance->spiRXBuffer[1] << 4u);
                    pInstance->hwModel |= (pInstance->spiRXBuffer[2] >> 4u);

                    /* mask revision is low nibble of version byte */
                    pInstance->hwMaskRevision = ((uint8_t)pInstance->spiRXBuffer[2] & 0xFu);

                    pInstance->state      = MXM_STATEMACH_41B_IDLE;
                    *pInstance->processed = MXM_41B_STATE_PROCESSED;
                }

            } else {
                /* something is very broken */
                *pInstance->processed = MXM_41B_STATE_ERROR;
            }
            break;
        case MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER:
            /* clear receive buffer --> reset UART RX into defined state */
            retval = MXM_41BRegisterWrite(pInstance, MXM_BUF_CLR_RX_BUF, NULL_PTR, 0);

            if (STD_OK == retval) {
                /* writing successful, return to idle */
                pInstance->state      = MXM_STATEMACH_41B_IDLE;
                *pInstance->processed = MXM_41B_STATE_PROCESSED;
            } else {
                /* an error has occurred, retry and set error */
                *pInstance->processed = MXM_41B_STATE_ERROR;
            }
            break;
        case MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER:
            /* clear receive buffer --> reset UART RX into defined state */
            retval = MXM_41BRegisterWrite(pInstance, MXM_BUF_CLR_TX_BUF, NULL_PTR, 0);

            if (STD_OK == retval) {
                /* writing successful, return to idle */
                pInstance->state      = MXM_STATEMACH_41B_IDLE;
                *pInstance->processed = MXM_41B_STATE_PROCESSED;
            } else {
                /* an error has occurred, retry and set error */
                *pInstance->processed = MXM_41B_STATE_ERROR;
            }
            break;
        default:
            /* this default case should never be reached */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
