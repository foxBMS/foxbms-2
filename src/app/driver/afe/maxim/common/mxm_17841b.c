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
 * @updated 2021-12-06 (date of last update)
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
#include "os.h"

/*========== Macros and Definitions =========================================*/
/** bit shift half byte length */
#define MXM_41B_BIT_SHIFT_HALF_BYTE (4u)

/** low nibble (of uint8_t) bit mask */
#define MXM_41B_BIT_MASK_LOW_NIBBLE (0xFu)

/** high nibble (of uint8_t) bit mask */
#define MXM_41B_BIT_MASK_HIGH_NIBBLE (0xF0u)

/** (uint8_t) one byte bit mask */
#define MXM_41B_BIT_MASK_ONE_BYTE (0xFFu)

/** threshold above which a reset in this driver occurs in case of hangup */
#define MXM_41B_WAIT_COUNTER_THRESHOLD (75u)
#if MXM_41B_WAIT_COUNTER_THRESHOLD > (UINT8_MAX - 1)
#error "invalid wait counter threshold (counter is implemented in uint8_t)"
#endif

/**
 * time interval that shall ensure that the bridge IC is completely reset in ms
 */
#define MXM_41B_BRIDGE_RESET_TIME_MS (100u)

/** default config register bank length
 *
 * Length of the array that writes the registers referenced in
 * mxm_41B_reg_default_values in #MXM_41BStateHandlerInit().
 */
#define MXM_41B_CONFIG_REGISTER_LENGTH (7u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

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
 * will write to adjacent registers. Please see the MAX17841B data sheet
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
    uint8_t lengthPayload);

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
 * will read also from adjacent registers. Please see the MAX17841B data sheet
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
    uint8_t length);

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
 * The extendMessage parameter describes with how much bytes the
 * command shall be stretched. This number will be added to the
 * length of the command and written into the length field of the
 * buffer.
 * After this action the user has to select the next load queue with
 * the appropriate command in order to mark the load queue as sendable.
 *
 * @param[in,out]   pInstance       pointer to the state of the MAX17841B-state-machine
 * @param[in]       kpkMessage      pointer to an array containing the message
 * @param[in]       messageLength   length of the supplied array
 * @param[in]       extendMessage   stretch the message by number of bytes
 * @return #STD_NOT_OK for inconsistent input or a blocked SPI interface,
 *         otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MXM_41BBufferWrite(
    MXM_41B_INSTANCE_s *pInstance,
    const uint16_t *const kpkMessage,
    uint8_t messageLength,
    uint8_t extendMessage);

/**
 * @brief   Transition into idle, mark as successfull
 * @param[out]  pInstance   pointer to the state-struct
 */
static void MXM_41BTransitionToIdleSuccess(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   Transition into idle, mark as an error occurred
 * @param[out]  pInstance   pointer to the state-struct
 */
static void MXM_41BTransitionToIdleError(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   Reset register copies to default
 * @param[out]  pInstance   pointer to the state-struct
 */
static void MXM_41BInitializeRegisterCopies(MXM_41B_INSTANCE_s *pInstance);

/**
 * \defgroup mxm-41b-state-handlers State handler functions for #MXM_41BStateMachine()
 * @{
 */

/**
 * @brief   init state handler
 * @details This function contains all states for #MXM_STATEMACH_41B_INIT.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerInit(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "get version"
 * @details This function contains all states for #MXM_STATEMACH_41B_GET_VERSION.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerGetVersion(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "idle"
 * @details This function contains all states for #MXM_STATEMACH_41B_IDLE.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerIdle(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "write conf and int register"
 * @details This functions contains all states for #MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerWriteConfAndIntRegister(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "read status register"
 * @details This functions contains all states for #MXM_STATEMACH_41B_READ_STATUS_REGISTER.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerReadStatusRegister(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "uart transaction"
 * @details This functions contains all states for #MXM_STATEMACH_41B_UART_TRANSACTION.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerUartTransaction(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "check fmea"
 * @details This functions contains all states for #MXM_STATEMACH_41B_CHECK_FMEA.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerCheckFmea(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "clear receive buffer"
 * @details This functions contains all states for #MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerClearReceiveBuffer(MXM_41B_INSTANCE_s *pInstance);

/**
 * @brief   state handler for "clear transmit buffer"
 * @details This functions contains all states for #MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_41BStateHandlerClearTransmitBuffer(MXM_41B_INSTANCE_s *pInstance);

/** @} */

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e MXM_41BRegisterWrite(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_ADD_t command,
    const uint8_t *const kpkPayload,
    uint8_t lengthPayload) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    FAS_ASSERT(lengthPayload < (MXM_SPI_TX_BUFFER_LENGTH - 1u));
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* check if command is a write command (write addresses in MAX17841B are even) */
    if ((command % 2u) == 0u) {
        /* construct tx buffer */
        pInstance->spiTXBuffer[0] = command;
        /* message has payload --> copy into buffer */
        if ((kpkPayload != NULL_PTR) && (lengthPayload != 0u)) {
            for (uint8_t i = 0u; i < lengthPayload; i++) {
                pInstance->spiTXBuffer[i + 1u] = kpkPayload[i];
            }
            /* null rest of tx buffer */
            for (uint8_t i = lengthPayload + 1u; i < MXM_SPI_TX_BUFFER_LENGTH; i++) {
                pInstance->spiTXBuffer[i] = 0u;
            }
            /* send command with payload */
            retval = MXM_SendData(pInstance->spiTXBuffer, (uint16_t)lengthPayload + 1u);
        } else if ((kpkPayload == NULL_PTR) && (lengthPayload == 0u)) {
            /* send command without payload */
            retval = MXM_SendData(pInstance->spiTXBuffer, 1);
        } else {
            /* invalid configuration */
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_41BRegisterRead(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_41B_REG_ADD_t command,
    uint16_t *pRxBuffer,
    uint8_t length) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    /* RX Buffer may not be NULL pointer for this function */
    FAS_ASSERT(pRxBuffer != NULL_PTR);
    FAS_ASSERT(length <= MXM_SPI_RX_BUFFER_LENGTH);

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
        retval = MXM_ReceiveData(pInstance->spiTXBuffer, pRxBuffer, ((uint16_t)length + 1u));
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_41BConfigRegisterWrite(MXM_41B_INSTANCE_s *pInstance) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    uint8_t mxm_spi_temp_buffer[MXM_41B_CONFIG_REGISTER_LENGTH] = {0};
    /* TODO refactor so that int flags are not cleared by this function (because otherwise we would have to read them before every write) */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Magic numbers for index value of array is clear in usage */
    mxm_spi_temp_buffer[0u] = pInstance->regRXIntEnable;
    mxm_spi_temp_buffer[1u] = pInstance->regTXIntEnable;
    mxm_spi_temp_buffer[2u] = MXM_41B_RX_INT_FLAG_DEFAULT_VALUE; /* TODO add reg_*x_int_flag */
    mxm_spi_temp_buffer[3u] = MXM_41B_TX_INT_FLAG_DEFAULT_VALUE;
    mxm_spi_temp_buffer[4u] = pInstance->regConfig1;
    mxm_spi_temp_buffer[5u] = pInstance->regConfig2;
    mxm_spi_temp_buffer[6u] = pInstance->regConfig3;
    /* AXIVION Enable Style Generic-NoMagicNumbers: */
    static_assert((6u < MXM_41B_CONFIG_REGISTER_LENGTH), "Revise this function and config register length!");

    return MXM_41BRegisterWrite(
        pInstance, MXM_REG_RX_INTERRUPT_ENABLE_W, mxm_spi_temp_buffer, MXM_41B_CONFIG_REGISTER_LENGTH);
}

static STD_RETURN_TYPE_e MXM_41BBufferWrite(
    MXM_41B_INSTANCE_s *pInstance,
    const uint16_t *const kpkMessage,
    uint8_t messageLength,
    uint8_t extendMessage) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);
    /* check if message-pointer is valid */
    FAS_ASSERT(kpkMessage != NULL_PTR);
    FAS_ASSERT(messageLength >= 1u);
    FAS_ASSERT(messageLength <= 6u);

    /* write address and length to buffer */
    pInstance->spiTXBuffer[0] = (uint16_t)MXM_BUF_WR_LD_Q_0;
    pInstance->spiTXBuffer[1] = (uint16_t)messageLength + extendMessage;
    /* iterate of complete TX buffer and
     * write into proper fields, null rest
     */
    for (uint8_t i = 0; i < (MXM_SPI_TX_BUFFER_LENGTH - 2u); i++) {
        if (i < messageLength) {
            pInstance->spiTXBuffer[i + 2u] = kpkMessage[i];
        } else {
            pInstance->spiTXBuffer[i + 2u] = 0x00u;
        }
    }

    /* send data */
    return MXM_SendData(pInstance->spiTXBuffer, ((uint16_t)messageLength + 2u));
}

static void MXM_41BTransitionToIdleSuccess(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    FAS_ASSERT(pInstance->processed != NULL_PTR);
    pInstance->state      = MXM_STATEMACH_41B_IDLE;
    pInstance->substate   = MXM_41B_ENTRY_SUBSTATE;
    *pInstance->processed = MXM_41B_STATE_PROCESSED;
    return;
}

static void MXM_41BTransitionToIdleError(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    FAS_ASSERT(pInstance->processed != NULL_PTR);
    pInstance->state      = MXM_STATEMACH_41B_IDLE;
    pInstance->substate   = MXM_41B_ENTRY_SUBSTATE;
    *pInstance->processed = MXM_41B_STATE_ERROR;
    return;
}

static void MXM_41BInitializeRegisterCopies(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    pInstance->regRXStatus    = 0u;
    pInstance->regTXStatus    = 0u;
    pInstance->regRXIntEnable = MXM_41B_RX_INT_ENABLE_DEFAULT_VALUE;
    pInstance->regTXIntEnable = MXM_41B_TX_INT_ENABLE_DEFAULT_VALUE;
    pInstance->regConfig1     = MXM_41B_CONFIG_1_DEFAULT_VALUE;
    pInstance->regConfig2     = MXM_41B_CONFIG_2_DEFAULT_VALUE;
    pInstance->regConfig3     = MXM_41B_CONFIG_3_DEFAULT_VALUE;
    return;
}

static void MXM_41BStateHandlerInit(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        pInstance->substate = MXM_41B_INIT_RESET_BRIDGE_IC;
    }

    if (pInstance->substate == MXM_41B_INIT_RESET_BRIDGE_IC) {
        MXM_ShutDownBridgeIc();
        pInstance->shutdownTimeStamp = OS_GetTickCount();
        pInstance->substate          = MXM_41B_INIT_START_BRIDGE_IC;
    } else if (pInstance->substate == MXM_41B_INIT_START_BRIDGE_IC) {
        const bool resetTimeHasPassed =
            OS_CheckTimeHasPassed(pInstance->shutdownTimeStamp, MXM_41B_BRIDGE_RESET_TIME_MS);
        if (resetTimeHasPassed) {
            MXM_EnableBridgeIc();
            pInstance->substate = MXM_41B_INIT_WRITE_DEFAULT_VALUES;
        }
    } else if (pInstance->substate == MXM_41B_INIT_WRITE_DEFAULT_VALUES) {
        /* set default register values according to data sheet */
        MXM_41BInitializeRegisterCopies(pInstance);
        const STD_RETURN_TYPE_e retval = MXM_41BConfigRegisterWrite(pInstance);
        if (retval == STD_OK) {
            pInstance->substate = MXM_41B_INIT_READ_CONFIG_REGISTERS;
        }
    } else if (pInstance->substate == MXM_41B_INIT_READ_CONFIG_REGISTERS) {
        const STD_RETURN_TYPE_e retval = MXM_41BRegisterRead(
            pInstance, MXM_REG_RX_INTERRUPT_ENABLE_R, pInstance->spiRXBuffer, MXM_41B_CONFIG_REGISTER_LENGTH);

        if (retval == STD_OK) {
            pInstance->substate = MXM_41B_INIT_CHECK_INITIALIZATION;
        }
    } else if (pInstance->substate == MXM_41B_INIT_CHECK_INITIALIZATION) {
        STD_RETURN_TYPE_e retval = STD_OK;

        if (MXM_GetSPIStateReady() == STD_OK) {
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
            const uint8_t mxm_41B_reg_default_values[MXM_41B_CONFIG_REGISTER_LENGTH] = {
                MXM_41B_RX_INT_ENABLE_DEFAULT_VALUE,
                MXM_41B_TX_INT_ENABLE_DEFAULT_VALUE,
                MXM_41B_RX_INT_FLAG_DEFAULT_VALUE,
                MXM_41B_TX_INT_FLAG_DEFAULT_VALUE,
                MXM_41B_CONFIG_1_DEFAULT_VALUE,
                MXM_41B_CONFIG_2_DEFAULT_VALUE,
                MXM_41B_CONFIG_3_DEFAULT_VALUE};

            for (uint8_t i = 0; i < MXM_41B_CONFIG_REGISTER_LENGTH; i++) {
                if (pInstance->spiRXBuffer[i + 1u] != mxm_41B_reg_default_values[i]) {
                    retval = STD_NOT_OK;
                }
            }
        } else {
            retval = STD_NOT_OK;
        }

        if (retval == STD_NOT_OK) {
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            MXM_41BTransitionToIdleSuccess(pInstance);
        }
    } else {
        MXM_41BTransitionToIdleError(pInstance);
    }
}

static void MXM_41BStateHandlerGetVersion(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
        pInstance->substate = MXM_41B_VERSION_REQUEST_REGISTER;
    }

    if (pInstance->substate == MXM_41B_VERSION_REQUEST_REGISTER) {
        /* read two byte in order to read also the adjacent version register */
        const STD_RETURN_TYPE_e retval = MXM_41BRegisterRead(pInstance, MXM_REG_MODEL_R, pInstance->spiRXBuffer, 2);

        if (retval == STD_OK) {
            pInstance->substate = MXM_41B_VERSION_VERIFY;
        }
    } else if (pInstance->substate == MXM_41B_VERSION_VERIFY) {
        if (MXM_GetSPIStateReady() == STD_OK) {
            /* get model from model register and high nibble of mask revision (should be 0x8410) */
            pInstance->hwModel =
                (uint16_t)((pInstance->spiRXBuffer[1] & MXM_41B_BIT_MASK_ONE_BYTE) << MXM_41B_BIT_SHIFT_HALF_BYTE);
            pInstance->hwModel |=
                (uint16_t)((pInstance->spiRXBuffer[2] & MXM_41B_BIT_MASK_HIGH_NIBBLE) >> MXM_41B_BIT_SHIFT_HALF_BYTE);
            /* extract mask revision from low nibble */
            pInstance->hwMaskRevision = (uint8_t)(pInstance->spiRXBuffer[2] & MXM_41B_BIT_MASK_LOW_NIBBLE);

            MXM_41BTransitionToIdleSuccess(pInstance);
        }

    } else {
        /* something is very broken */
        MXM_41BTransitionToIdleError(pInstance);
    }
}

static void MXM_41BStateHandlerIdle(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    /* do nothing in idle state
       just clean up substate */
    pInstance->substate = MXM_41B_ENTRY_SUBSTATE;
}

static void MXM_41BStateHandlerWriteConfAndIntRegister(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    const STD_RETURN_TYPE_e retval = MXM_41BConfigRegisterWrite(pInstance);

    if (retval == STD_NOT_OK) {
        MXM_41BTransitionToIdleError(pInstance);
    } else {
        MXM_41BTransitionToIdleSuccess(pInstance);
    }
}

static void MXM_41BStateHandlerReadStatusRegister(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    /* TODO read status register and parse into static variables */
    if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        pInstance->substate = MXM_41B_READ_STATUS_REGISTER_SEND;
    }

    if (pInstance->substate == MXM_41B_READ_STATUS_REGISTER_SEND) {
        /* read rx and tx status register */
        const STD_RETURN_TYPE_e retval = MXM_41BRegisterRead(pInstance, MXM_REG_RX_STATUS_R, pInstance->spiRXBuffer, 2);
        if (retval == STD_NOT_OK) {
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            pInstance->substate = MXM_41B_READ_STATUS_REGISTER_PROCESS;
        }
    } else if (pInstance->substate == MXM_41B_READ_STATUS_REGISTER_PROCESS) {
        pInstance->regRXStatus = (uint8_t)(pInstance->spiRXBuffer[1] & 0xFFu);
        pInstance->regTXStatus = (uint8_t)(pInstance->spiRXBuffer[2] & 0xFFu);
        MXM_41BTransitionToIdleSuccess(pInstance);
    } else {
        /* something is very broken */
        MXM_41BTransitionToIdleError(pInstance);
    }
}

static void MXM_41BStateHandlerUartTransaction(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        pInstance->substate = MXM_41B_UART_READ_RX_SPACE;
    }

    if (pInstance->substate == MXM_41B_UART_READ_RX_SPACE) {
        const STD_RETURN_TYPE_e retval = MXM_41BRegisterRead(pInstance, MXM_REG_RX_SPACE_R, pInstance->spiRXBuffer, 1u);
        if (retval == STD_NOT_OK) {
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            pInstance->substate = MXM_41B_UART_READ_RX_SPACE_PARSE;
        }
    } else if (pInstance->substate == MXM_41B_UART_READ_RX_SPACE_PARSE) {
        pInstance->regRxSpace = (uint8_t)(pInstance->spiRXBuffer[1] & MXM_41B_BIT_MASK_ONE_BYTE);
        pInstance->substate   = MXM_41B_UART_WRITE_LOAD_QUEUE;

    } else if (pInstance->substate == MXM_41B_UART_WRITE_LOAD_QUEUE) {
        /* load queue with message */
        const STD_RETURN_TYPE_e retval =
            MXM_41BBufferWrite(pInstance, pInstance->pPayload, pInstance->payloadLength, pInstance->extendMessageBytes);

        if (retval == STD_NOT_OK) {
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            pInstance->substate = MXM_41B_UART_READ_LOAD_QUEUE;
        }
    } else if (pInstance->substate == MXM_41B_UART_READ_LOAD_QUEUE) {
        /* check assumption that incremented payloadlength fits into uint8_t */
        FAS_ASSERT(pInstance->payloadLength < (uint8_t)UINT8_MAX);
        const uint8_t payloadLength = pInstance->payloadLength + 1u;
        /* send read load queue */
        const STD_RETURN_TYPE_e retval =
            MXM_41BRegisterRead(pInstance, MXM_BUF_RD_LD_Q_0, pInstance->spiRXBuffer, payloadLength);

        if (retval == STD_NOT_OK) {
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            pInstance->substate = MXM_41B_UART_VERIFY_LOAD_QUEUE_AND_TRANSMIT;
        }
    } else if (pInstance->substate == MXM_41B_UART_VERIFY_LOAD_QUEUE_AND_TRANSMIT) {
        /* verify load queue */
        STD_RETURN_TYPE_e retval = STD_OK;
        /* check message length */
        if (pInstance->spiRXBuffer[1] != (pInstance->payloadLength + (uint16_t)pInstance->extendMessageBytes)) {
            retval = STD_NOT_OK;
        }
        for (uint8_t i = 0; i < pInstance->payloadLength; i++) {
            FAS_ASSERT(pInstance->pPayload != NULL_PTR);
            if (pInstance->spiRXBuffer[i + 2u] != pInstance->pPayload[i]) {
                /* message corrupted during SPI transfer */
                retval = STD_NOT_OK;
            }
        }
        if (retval == STD_NOT_OK) {
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            /* transmit queue */
            retval = MXM_41BRegisterWrite(pInstance, MXM_BUF_WR_NXT_LD_Q_0, NULL_PTR, 0);

            if (retval == STD_NOT_OK) {
                MXM_41BTransitionToIdleError(pInstance);
            } else {
                pInstance->substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE;
            }
        }
    } else if (pInstance->substate == MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE) {
        /* poll RX status change */
        const STD_RETURN_TYPE_e retval = MXM_41BRegisterRead(pInstance, MXM_REG_RX_STATUS_R, pInstance->spiRXBuffer, 1);

        if (retval == STD_NOT_OK) {
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            pInstance->substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF;
        }
    } else if (pInstance->substate == MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_READ_AND_READ_BACK_RCV_BUF) {
        /* update RX status register copy with received buffer */
        pInstance->regRXStatus = (uint8_t)(pInstance->spiRXBuffer[1] & MXM_41B_BIT_MASK_ONE_BYTE);
        /* check if RX_OVERFLOW_Status is 1 */
        MXM_41B_REG_BIT_VALUE rx_overflow_status_value = MXM_41B_REG_FALSE;
        const STD_RETURN_TYPE_e resultWrongRegisterOverflow =
            MXM_41BReadRegisterFunction(pInstance, MXM_41B_REG_FUNCTION_RX_OVERFLOW_STATUS, &rx_overflow_status_value);
        FAS_ASSERT(resultWrongRegisterOverflow == STD_OK);
        /* check if RX_STOP_Status is 1 */
        MXM_41B_REG_BIT_VALUE rx_stop_status_value = MXM_41B_REG_FALSE;
        const STD_RETURN_TYPE_e resultWrongRegisterStop =
            MXM_41BReadRegisterFunction(pInstance, MXM_41B_REG_FUNCTION_RX_STOP_STATUS, &rx_stop_status_value);
        FAS_ASSERT(resultWrongRegisterStop == STD_OK);
        if (rx_overflow_status_value == MXM_41B_REG_TRUE) {
            /* overflow, we have to discard the rx buffer */
            const STD_RETURN_TYPE_e retval = MXM_41BRegisterWrite(pInstance, MXM_BUF_CLR_RX_BUF, NULL_PTR, 0);
            if (retval == STD_NOT_OK) {
                MXM_41BTransitionToIdleError(pInstance);
            } else {
                MXM_41BTransitionToIdleError(pInstance);
            }
        } else if (rx_stop_status_value == MXM_41B_REG_TRUE) {
            /* received full UART frame --> continue */
            /* check assumption that payload length fits into uint8_t */
            FAS_ASSERT((pInstance->payloadLength + (uint16_t)1u + pInstance->extendMessageBytes) <= (uint8_t)UINT8_MAX);
            const uint8_t payloadLength = pInstance->payloadLength + 1u + pInstance->extendMessageBytes;
            /* read back receive buffer */
            const STD_RETURN_TYPE_e retval =
                MXM_41BRegisterRead(pInstance, MXM_BUF_RD_NXT_MSG, pInstance->spiRXBuffer, payloadLength);

            if (retval == STD_NOT_OK) {
                MXM_41BTransitionToIdleError(pInstance);
            } else {
                pInstance->substate    = MXM_41B_UART_READ_BACK_RECEIVE_BUFFER_SAVE;
                pInstance->waitCounter = 0u;
            }
        } else {
            /* no UART frame received yet --> check again */
            pInstance->substate = MXM_41B_UART_WAIT_FOR_RX_STATUS_CHANGE_WRITE;
            /* increment wait counter (only to 1 above MXM_41B_WAIT_COUNTER_THRESHOLD,
            then other parts of the code will reset). */
            if (pInstance->waitCounter <= MXM_41B_WAIT_COUNTER_THRESHOLD) {
                pInstance->waitCounter++;
            }
        }
    } else if (pInstance->substate == MXM_41B_UART_READ_BACK_RECEIVE_BUFFER_SAVE) {
        if ((pInstance->spiRXBuffer != NULL_PTR) && (pInstance->pRxBuffer != NULL_PTR)) {
            for (uint16_t i = 0; i < ((uint16_t)pInstance->payloadLength + pInstance->extendMessageBytes); i++) {
                if (i < pInstance->rxBufferLength) {
                    pInstance->pRxBuffer[i] = pInstance->spiRXBuffer[i + 1u];
                }
            }
        }
        MXM_41BTransitionToIdleSuccess(pInstance);
    } else {
        /* we should not be here */
        FAS_ASSERT(FAS_TRAP);
    }
}

static void MXM_41BStateHandlerCheckFmea(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    if (pInstance->substate == MXM_41B_ENTRY_SUBSTATE) {
        pInstance->substate = MXM_41B_FMEA_REQUEST_REGISTER;
    }

    if (pInstance->substate == MXM_41B_FMEA_REQUEST_REGISTER) {
        const STD_RETURN_TYPE_e retval = MXM_41BRegisterRead(pInstance, MXM_REG_FMEA_R, pInstance->spiRXBuffer, 1);

        if (retval == STD_OK) {
            pInstance->substate = MXM_41B_FMEA_VERIFY;
        }
    } else if (pInstance->substate == MXM_41B_FMEA_VERIFY) {
        STD_RETURN_TYPE_e retval = STD_NOT_OK;
        if (MXM_GetSPIStateReady() == STD_OK) {
            if (pInstance->spiRXBuffer[1] == 0u) {
                retval = STD_OK;
            }
        }

        if (retval == STD_NOT_OK) {
            /* FMEA check went bad */
            MXM_41BTransitionToIdleError(pInstance);
        } else {
            MXM_41BTransitionToIdleSuccess(pInstance);
        }
    } else {
        /* something is very broken */
        MXM_41BTransitionToIdleError(pInstance);
    }
}

static void MXM_41BStateHandlerClearReceiveBuffer(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    /* clear receive buffer --> reset UART RX into defined state */
    const STD_RETURN_TYPE_e retval = MXM_41BRegisterWrite(pInstance, MXM_BUF_CLR_RX_BUF, NULL_PTR, 0);

    if (retval == STD_OK) {
        /* writing successful, return to idle */
        MXM_41BTransitionToIdleSuccess(pInstance);
    } else {
        /* an error has occurred, retry and set error */
        MXM_41BTransitionToIdleError(pInstance);
    }
}

static void MXM_41BStateHandlerClearTransmitBuffer(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    /* clear receive buffer --> reset UART RX into defined state */
    const STD_RETURN_TYPE_e retval = MXM_41BRegisterWrite(pInstance, MXM_BUF_CLR_TX_BUF, NULL_PTR, 0);

    if (retval == STD_OK) {
        /* writing successful, return to idle */
        MXM_41BTransitionToIdleSuccess(pInstance);
    } else {
        /* an error has occurred, retry and set error */
        MXM_41BTransitionToIdleError(pInstance);
    }
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e MXM_41BSetStateRequest(
    MXM_41B_INSTANCE_s *pInstance,
    MXM_STATEMACH_41B_e state,
    uint16_t *pPayload,
    uint8_t payloadLength,
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
        case MXM_41B_REG_FUNCTION_RX_OVERFLOW_STATUS:
            *pValue = mxm_41bReadValue(kpkInstance->regRXStatus, 1, MXM_41B_RX_OVERFLOW_STATUS); /* 3rd bit */
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

void MXM_41BStateMachine(MXM_41B_INSTANCE_s *pInstance) {
    /* sanity check: state-pointer may not be null */
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->waitCounter > MXM_41B_WAIT_COUNTER_THRESHOLD) {
        /* error, reset to idle state */
        MXM_41BTransitionToIdleError(pInstance);
        pInstance->waitCounter = 0u;
    }
    switch (pInstance->state) {
        case MXM_STATEMACH_41B_UNINITIALIZED:
            break;
        case MXM_STATEMACH_41B_INIT:
            MXM_41BStateHandlerInit(pInstance);
            break;
        case MXM_STATEMACH_41B_GET_VERSION:
            MXM_41BStateHandlerGetVersion(pInstance);
            break;
        case MXM_STATEMACH_41B_IDLE:
            MXM_41BStateHandlerIdle(pInstance);
            break;
        case MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER:
            MXM_41BStateHandlerWriteConfAndIntRegister(pInstance);
            break;
        case MXM_STATEMACH_41B_READ_STATUS_REGISTER:
            MXM_41BStateHandlerReadStatusRegister(pInstance);
            break;
        case MXM_STATEMACH_41B_UART_TRANSACTION:
            MXM_41BStateHandlerUartTransaction(pInstance);
            break;
        case MXM_STATEMACH_41B_CHECK_FMEA:
            MXM_41BStateHandlerCheckFmea(pInstance);
            break;
        case MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER:
            MXM_41BStateHandlerClearReceiveBuffer(pInstance);
            break;
        case MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER:
            MXM_41BStateHandlerClearTransmitBuffer(pInstance);
            break;
        default:
            /* this default case should never be reached */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

extern void MXM_41BInitializeStateStruct(MXM_41B_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    MXM_41BInitializeRegisterCopies(pInstance);

    pInstance->state              = MXM_STATEMACH_41B_UNINITIALIZED;
    pInstance->substate           = MXM_41B_ENTRY_SUBSTATE;
    pInstance->pPayload           = NULL_PTR;
    pInstance->payloadLength      = 0u;
    pInstance->pRxBuffer          = NULL_PTR;
    pInstance->rxBufferLength     = 0u;
    pInstance->processed          = NULL_PTR;
    pInstance->extendMessageBytes = 0u;
    pInstance->waitCounter        = 0u;
    pInstance->regRxSpace         = 0u;
    pInstance->hwModel            = 0u;
    pInstance->hwMaskRevision     = 0u;
    pInstance->shutdownTimeStamp  = 0u;

    for (uint32_t i = 0u; i < MXM_SPI_RX_BUFFER_LENGTH; i++) {
        pInstance->spiRXBuffer[i] = 0u;
    }

    for (uint32_t i = 0u; i < MXM_SPI_TX_BUFFER_LENGTH; i++) {
        pInstance->spiTXBuffer[i] = 0u;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
