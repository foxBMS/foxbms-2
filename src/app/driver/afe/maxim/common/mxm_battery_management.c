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
 * @file    mxm_battery_management.c
 * @author  foxBMS Team
 * @date    2019-01-14 (date of creation)
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
#include "mxm_battery_management.h"

#include "os.h"

/*========== Macros and Definitions =========================================*/

/** length of the helloall command @{*/
#define HELLOALL_TX_LENGTH (3u)
#define HELLOALL_RX_LENGTH HELLOALL_TX_LENGTH
/**@}*/

/** threshold above which an error handling procedure is triggered */
#define MXM_5X_ERROR_THRESHOLD (3u)

/** time in milliseconds that should be waited in order to ensure that the slaves shut off */
#define MXM_5X_SLAVE_SHUTDOWN_TIMEOUT_MS (400u)

/** (uint8_t) one byte bit mask */
#define MXM_5X_BIT_MASK_ONE_BYTE (0xFFu)

/**
 * @brief bit masks for writing the device address in write device command
 */
#define MXM_5X_BIT_MASK_WRITE_DEVICE_ADDRESS ((uint16_t)0xF8u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Clear the command-buffer.
 * @details Clears #MXM_5X_INSTANCE_s::commandBuffer by writing 0x00 to every entry.
 * @param[in,out]   pInstance   pointer to the state-struct
 * @return  always return #STD_OK
 */
static void MXM_5XClearCommandBuffer(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   Check if a register address is user accessible
 * @details Checks if a register address is inside the user accessible memory
 *          range. This range is specified in the data sheet of the monitoring
 *          IC as following:
 *              - user memory is contained in the range 0x00 to 0x98
 *              - reserved addresses in the user address space are:
 *                  - 0x2C, 0x2D, 0x2E, 0x2F
 *                  - 0x46
 *                  - 0x84 through 0x8B
 * @param[in]   regAddress  register address to be checked
 * @param[in]   model       model id of the IC that shall be addressed
 * @return      #STD_OK if the register address is good, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e MXM_5XIsUserAccessibleRegister(uint8_t regAddress, MXM_MODEL_ID_e model);

/**
 * @brief   Check if a register address is user accessible in MAX17852
 * @details Checks if a register address is inside the user accessible memory
 *          range.
 *          This range is specified in the data sheet of the monitoring IC.
 * @param[in]   regAddress register address to be checked
 * @return      #STD_OK if the register address is good, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e MXM_52IsUserAccessibleRegister(uint8_t regAddress);

/**
 * @brief   Check if a register address is user accessible in MAX17853
 * @details Checks if a register address is inside the user accessible memory
 *          range.
 *          This range is specified in the data sheet of the monitoring IC as
 *          the following:
 *              - user memory is contained in the range 0x00 to 0x98
 *              - reserved addresses in the user address space are:
 *                  - 0x2C, 0x2D, 0x2E, 0x2F
 *                  - 0x46
 *                  - 0x84 through 0x8B
 * @param[in]   regAddress register address to be checked
 * @return      #STD_OK if the register address is good, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e MXM_53IsUserAccessibleRegister(uint8_t regAddress);

/**
 * @brief   clears the command buffer and writes HELLOALL into the buffer
 * @details Fills the command buffer with a HELLOALL message after having it
 *          cleaned.
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_5XConstructCommandBufferHelloall(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   clears the command buffer and writes WRITEALL into the buffer
 * @details Fills the command buffer with a WRITEALL command. This command
 *          writes the same lsb and msb to every satellite in the daisy-chain
 *          in the same register. The data to be written has to be set before
 *          calling this function in #MXM_5X_INSTANCE_s::commandPayload.
 * @param[in,out]   pInstance   pointer to the state-struct
 * @return      #STD_OK if an accessible register address has been selected,
 *              #STD_NOT_OK if not.
 */
static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteall(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   clears the command buffer and writes a WRITEDEVICE message
 * @details Fills the command buffer with a WRITEDEVICE message. This message
 *          is addressed to one specific device in the daisy-chain. Therefore
 *          the address of the device has to be supplied together with the
 *          register and the data that should be written. The data to be
 *          written has to be set before calling this function in
 *          #MXM_5X_INSTANCE_s::commandPayload.
 * @param[in,out]   pInstance       pointer to the state-struct
 * @return      #STD_OK if an accessible register address has been selected,
 *              #STD_NOT_OK if not.
 */
static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteDevice(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   clears the command buffer and writes READALL into the buffer
 * @details Fills the command buffer with a READALL command. This command
 *          retrieves the LSB and MSB of exactly one register of every device
 *          in the daisy-chain. The data to be written has to be set before
 *          calling this function in #MXM_5X_INSTANCE_s::commandPayload.
 * @param[in,out]   pInstance   pointer to the state-struct
 * @return      #STD_OK if an accessible register address has been selected,
 *              #STD_NOT_OK if not.
 */
static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferReadall(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   handles the error of the underlying state-machine (by resetting it and counting the error)
 * @param[in,out]   pInstance   pointer to the state-struct
 */
static void MXM_5XHandle41BErrorState(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   sets all internal state variables so that upon next execution the next substate is entered
 * @param[out]      pInstance   pointer to the state-struct
 * @param[in]       substate    identifier of the next substate
 */
static void MXM_5XTransitionToSubstate(MXM_5X_INSTANCE_s *pInstance, MXM_5X_SUBSTATES_e substate);

/**
 * @brief   repeat the current substate (by resetting to the entry state)
 * @param[out]      pInstance   pointer to the state-struct
 */
static void MXM_5XRepeatCurrentSubstate(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   Signal that a chain of substates has been successfully handled
 * @param[out]      pInstance   pointer to the state-struct
 */
static void MXM_5XSignalSuccess(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   Signal that an error has occurred in a chain of substates
 * @param[out]      pInstance   pointer to the state-struct
 */
static void MXM_5XSignalError(MXM_5X_INSTANCE_s *pInstance);

/**
 * @brief   Handle the state #MXM_STATEMACH_5X_41B_FMEA_CHECK
 * @param[in,out]   pInstance5x     pointer to the state-struct of the battery management state machine
 * @param[in,out]   pInstance41b    pointer to the state-struct of the bridge IC
 */
static void MXM_5XStateHandler41BFmeaCheck(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b);

/**
 * @brief   Handle the state #MXM_STATEMACH_5X_INIT
 * @param[in,out]   pInstance5x     pointer to the state-struct of the battery management state machine
 * @param[in,out]   pInstance41b    pointer to the state-struct of the bridge IC
 */
static void MXM_5XStateHandlerInit(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b);

/**
 * @brief   Handle the state #MXM_STATEMACH_5X_WRITEALL
 * @param[in,out]   pInstance5x     pointer to the state-struct of the battery management state machine
 * @param[in,out]   pInstance41b    pointer to the state-struct of the bridge IC
 */
static void MXM_5XStateHandlerWriteAll(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b);

/**
 * @brief   Handle the state #MXM_STATEMACH_5X_WRITE_DEVICE
 * @param[in,out]   pInstance5x     pointer to the state-struct of the battery management state machine
 * @param[in,out]   pInstance41b    pointer to the state-struct of the bridge IC
 */
static void MXM_5XStateHandlerWriteDevice(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b);

/**
 * @brief   Handle the state #MXM_STATEMACH_5X_READALL
 * @param[in,out]   pInstance5x     pointer to the state-struct of the battery management state machine
 * @param[in,out]   pInstance41b    pointer to the state-struct of the bridge IC
 */
static void MXM_5XStateHandlerReadAll(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b);

/*========== Static Function Implementations ================================*/
static void MXM_5XClearCommandBuffer(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    for (uint8_t i = 0; i < COMMAND_BUFFER_LENGTH; i++) {
        pInstance->commandBuffer[i] = 0x00U;
    }
    pInstance->commandBufferCurrentLength = COMMAND_BUFFER_LENGTH;
}

static STD_RETURN_TYPE_e MXM_5XIsUserAccessibleRegister(uint8_t regAddress, MXM_MODEL_ID_e model) {
    FAS_ASSERT(model <= MXM_MODEL_ID_invalid);

    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    switch (model) {
        case MXM_MODEL_ID_MAX17852:
            retval = MXM_52IsUserAccessibleRegister(regAddress);
            break;
        case MXM_MODEL_ID_MAX17853:
            retval = MXM_53IsUserAccessibleRegister(regAddress);
            break;
        case MXM_MODEL_ID_MAX17854:
        case MXM_MODEL_ID_NONE:
        case MXM_MODEL_ID_invalid:
            /* not implemented or invalid model id */
            break;
        default:
            /* invalid state, should not happen */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_52IsUserAccessibleRegister(uint8_t regAddress) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* check if regAddress is outside user-accessible area */
    /* AXIVION Disable Style Generic-NoMagicNumbers: memory limits of ICs are specific and unchangeable, therefore hardcoded */
    bool registerAddressIsInvalid = (regAddress == 0x5Du);
    registerAddressIsInvalid      = registerAddressIsInvalid || (regAddress == 0x5Eu);
    registerAddressIsInvalid      = registerAddressIsInvalid || (regAddress > 0x98u);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    if (registerAddressIsInvalid == false) {
        /* valid MAX17852 register address */
        retval = STD_OK;
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_53IsUserAccessibleRegister(uint8_t regAddress) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* check if regAddress is outside user-accessible area */
    /* AXIVION Disable Style Generic-NoMagicNumbers: memory limits of ICs are specific and unchangeable, therefore hardcoded */
    bool registerAddressIsInvalid = (regAddress == 0x46u);
    registerAddressIsInvalid      = registerAddressIsInvalid || ((0x2Cu <= regAddress) && (regAddress <= 0x2Fu));
    registerAddressIsInvalid      = registerAddressIsInvalid || ((0x84u <= regAddress) && (regAddress <= 0x8Bu));
    registerAddressIsInvalid      = registerAddressIsInvalid || (regAddress > 0x98u);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    if (registerAddressIsInvalid == false) {
        /* valid MAX17853 register address */
        retval = STD_OK;
    }
    return retval;
}

static void MXM_5XConstructCommandBufferHelloall(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    MXM_5XClearCommandBuffer(pInstance);
    pInstance->commandBuffer[0]           = BATTERY_MANAGEMENT_HELLOALL;
    pInstance->commandBuffer[1]           = 0x00;
    pInstance->commandBuffer[2]           = HELLOALL_START_SEED;
    pInstance->commandBufferCurrentLength = 3;
}

static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteall(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    const MXM_5X_COMMAND_PAYLOAD_s *const pPayload = &pInstance->commandPayload;
    FAS_ASSERT(pPayload != NULL_PTR);

    if (MXM_5XIsUserAccessibleRegister((uint8_t)pPayload->regAddress, pPayload->model) == STD_OK) {
        /* clear command buffer */
        MXM_5XClearCommandBuffer(pInstance);

        /* construct command buffer */
        pInstance->commandBuffer[0] = BATTERY_MANAGEMENT_WRITEALL;
        pInstance->commandBuffer[1] = (uint8_t)pPayload->regAddress;
        pInstance->commandBuffer[2] = pPayload->lsb;
        pInstance->commandBuffer[3] = pPayload->msb;
        /* PEC byte */
        pInstance->commandBuffer[4] = MXM_CRC8(pInstance->commandBuffer, 4);
        /* TODO alive-counter? */
        pInstance->commandBufferCurrentLength = 5;
        retval                                = STD_OK;
    }

    return retval;
}

static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteDevice(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    const MXM_5X_COMMAND_PAYLOAD_s *const pPayload = &pInstance->commandPayload;
    FAS_ASSERT(pPayload != NULL_PTR);

    if (MXM_5XIsUserAccessibleRegister((uint8_t)pPayload->regAddress, pPayload->model) == STD_OK) {
        /* clear command buffer */
        MXM_5XClearCommandBuffer(pInstance);

        /* construct command buffer */

        /* commandBuffer[0] = Device address in a daisy chain + 0b100
         * DA = deviceAddress
         * Bit     |   7   |   6   |   5   |   4   |   3   | 2 | 1 | 0
         * Content | DA[4] | DA[3] | DA[2] | DA[1] | DA[0] | 1 | 0 | 0
         */
        pInstance->commandBuffer[0] =
            ((((uint16_t)pPayload->deviceAddress << 3u) & MXM_5X_BIT_MASK_WRITE_DEVICE_ADDRESS) |
             (uint16_t)BATTERY_MANAGEMENT_WRITEDEVICE);
        pInstance->commandBuffer[1] = (uint8_t)pPayload->regAddress;
        pInstance->commandBuffer[2] = pPayload->lsb;
        pInstance->commandBuffer[3] = pPayload->msb;
        /* PEC byte */
        pInstance->commandBuffer[4] = MXM_CRC8(pInstance->commandBuffer, 4);
        /* TODO alive-counter? */
        pInstance->commandBufferCurrentLength = 5;
        retval                                = STD_OK;
    }

    return retval;
}

static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferReadall(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* TODO test these functions */

    const MXM_5X_COMMAND_PAYLOAD_s *const pPayload = &pInstance->commandPayload;
    FAS_ASSERT(pPayload != NULL_PTR);

    if (MXM_5XIsUserAccessibleRegister((uint8_t)pPayload->regAddress, pPayload->model) == STD_OK) {
        /* clear command buffer */
        MXM_5XClearCommandBuffer(pInstance);

        /* construct command buffer */
        pInstance->commandBuffer[0] = BATTERY_MANAGEMENT_READALL;
        pInstance->commandBuffer[1] = (uint8_t)pPayload->regAddress;
        pInstance->commandBuffer[2] = DATA_CHECK_BYTE_SEED;
        /* PEC byte */
        pInstance->commandBuffer[3] = MXM_CRC8(pInstance->commandBuffer, 3);
        /* TODO alive-counter? */
        pInstance->commandBufferCurrentLength = 4;
        retval                                = STD_OK;
    }

    if (pInstance->commandBufferCurrentLength != BATTERY_MANAGEMENT_TX_LENGTH_READALL) {
        /* define containing command buffer length does
         * not match actual buffer */
        retval = STD_NOT_OK;
    }

    return retval;
}

static void MXM_5XHandle41BErrorState(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    pInstance->status41b = MXM_41B_STATE_UNSENT;
    if (pInstance->errorCounter < (uint8_t)UINT8_MAX) {
        pInstance->errorCounter++;
    }
    return;
}

static void MXM_5XTransitionToSubstate(MXM_5X_INSTANCE_s *pInstance, MXM_5X_SUBSTATES_e substate) {
    FAS_ASSERT(pInstance != NULL_PTR);
    FAS_ASSERT(substate <= MXM_5X_ENTRY_SUBSTATE);
    pInstance->substate = substate;
    if ((pInstance->status41b == MXM_41B_STATE_PROCESSED) || (pInstance->status41b == MXM_41B_STATE_ERROR)) {
        pInstance->status41b = MXM_41B_STATE_UNSENT;
    }
    return;
}

static void MXM_5XRepeatCurrentSubstate(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    pInstance->status41b = MXM_41B_STATE_UNSENT;
    return;
}

static void MXM_5XSignalSuccess(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    MXM_5XTransitionToSubstate(pInstance, MXM_5X_ENTRY_SUBSTATE);
    FAS_ASSERT(pInstance->processed != NULL_PTR);
    *pInstance->processed = MXM_5X_STATE_PROCESSED;
    pInstance->state      = MXM_STATEMACH_5X_IDLE;
    return;
}

static void MXM_5XSignalError(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    MXM_5XTransitionToSubstate(pInstance, MXM_5X_ENTRY_SUBSTATE);
    FAS_ASSERT(pInstance->processed != NULL_PTR);
    *pInstance->processed = MXM_5X_STATE_ERROR;
    pInstance->state      = MXM_STATEMACH_5X_IDLE;
    return;
}

static void MXM_5XStateHandler41BFmeaCheck(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b) {
    FAS_ASSERT(pInstance5x != NULL_PTR);
    FAS_ASSERT(pInstance41b != NULL_PTR);
    if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_41B_FMEA_REQUEST);
    }

    if (pInstance5x->substate == MXM_5X_41B_FMEA_REQUEST) {
        const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
            pInstance41b, MXM_STATEMACH_41B_CHECK_FMEA, NULL_PTR, 0, 0, NULL_PTR, 0, &pInstance5x->status41b);
        FAS_ASSERT(stateRequestReturn == STD_OK);
        MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_41B_FMEA_CHECK);
    } else if (pInstance5x->substate == MXM_5X_41B_FMEA_CHECK) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            /* failure in FMEA; signal error */
            MXM_5XSignalError(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XSignalSuccess(pInstance5x);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else {
        FAS_ASSERT(FAS_TRAP);
    }
}

static void MXM_5XStateHandlerInit(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b) {
    FAS_ASSERT(pInstance5x != NULL_PTR);
    FAS_ASSERT(pInstance41b != NULL_PTR);
    if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_41B_INIT);
    }

    if (pInstance5x->substate == MXM_5X_INIT_41B_INIT) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b, MXM_STATEMACH_41B_INIT, NULL_PTR, 0, 0, NULL_PTR, 0, &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_41B_GET_VERSION);
            pInstance5x->resetWaitTimestamp = OS_GetTickCount();
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_41B_GET_VERSION) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b, MXM_STATEMACH_41B_GET_VERSION, NULL_PTR, 0, 0, NULL_PTR, 0, &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAIT_FOR_RESET);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAIT_FOR_RESET) {
        /* wait so that shutdown low of the satellites discharges and they switch off */
        const bool shutdownTimeoutHasPassed =
            OS_CheckTimeHasPassed(pInstance5x->resetWaitTimestamp, MXM_5X_SLAVE_SHUTDOWN_TIMEOUT_MS);
        if (shutdownTimeoutHasPassed) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_ENABLE_RX_INTERRUPT_FLAGS);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_ENABLE_RX_INTERRUPT_FLAGS) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e writeRegisterRxErrorReturn =
                MXM_41BWriteRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_RX_ERROR_INT, MXM_41B_REG_TRUE);
            FAS_ASSERT(writeRegisterRxErrorReturn == STD_OK);
            const STD_RETURN_TYPE_e writeRegisterRxOverflowReturn =
                MXM_41BWriteRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_RX_OVERFLOW_INT, MXM_41B_REG_TRUE);
            FAS_ASSERT(writeRegisterRxOverflowReturn == STD_OK);

            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_EN_PREAMBLES);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_EN_PREAMBLES) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e writeRegisterReturn =
                MXM_41BWriteRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_TX_PREAMBLES, MXM_41B_REG_TRUE);
            FAS_ASSERT(writeRegisterReturn == STD_OK);

            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_ENABLE_KEEP_ALIVE);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_ENABLE_KEEP_ALIVE) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const uint8_t mxm_kConfig3KeepAlive160us41BRegister = 0x05;
            const STD_RETURN_TYPE_e writeRegisterReturn         = MXM_41BWriteRegisterFunction(
                pInstance41b, MXM_41B_REG_FUNCTION_KEEP_ALIVE, mxm_kConfig3KeepAlive160us41BRegister);
            FAS_ASSERT(writeRegisterReturn == STD_OK);

            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_BUSY);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_BUSY) {
        /* wait for rx status change busy */
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_READ_STATUS_REGISTER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_41B_REG_BIT_VALUE functionValue;
            const STD_RETURN_TYPE_e readRegisterReturn =
                MXM_41BReadRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_RX_BUSY_STATUS, &functionValue);
            FAS_ASSERT(readRegisterReturn == STD_OK);
            if (functionValue == MXM_41B_REG_FALSE) {
                MXM_5XRepeatCurrentSubstate(pInstance5x);
            } else if (functionValue == MXM_41B_REG_TRUE) {
                MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_DIS_PREAMBLES);
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_DIS_PREAMBLES) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e writeRegisterReturn =
                MXM_41BWriteRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_TX_PREAMBLES, MXM_41B_REG_FALSE);
            FAS_ASSERT(writeRegisterReturn == STD_OK);

            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_EMPTY);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_EMPTY) {
        /* wait for rx status change busy */
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_READ_STATUS_REGISTER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_41B_REG_BIT_VALUE functionValue;
            const STD_RETURN_TYPE_e readRegisterReturn =
                MXM_41BReadRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_RX_EMPTY_STATUS, &functionValue);
            FAS_ASSERT(readRegisterReturn == STD_OK);

            if (functionValue == MXM_41B_REG_TRUE) {
                MXM_5XRepeatCurrentSubstate(pInstance5x);
            } else if (functionValue == MXM_41B_REG_FALSE) {
                MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_TRANSMIT_BUFFER);
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_TRANSMIT_BUFFER) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER_2);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER_2) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER,
                NULL_PTR,
                0,
                0,
                NULL_PTR,
                0,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            MXM_5XConstructCommandBufferHelloall(pInstance5x);
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_UART_TRANSACTION,
                pInstance5x->commandBuffer,
                pInstance5x->commandBufferCurrentLength,
                0,
                pInstance5x->rxBuffer,
                HELLOALL_RX_LENGTH,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            pInstance5x->substate = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL_VERIFY_MSG_AND_COUNT;
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
        /* TODO check for receive buffer errors and handle */
    } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL_VERIFY_MSG_AND_COUNT) {
        /* check if the commandBuffer matches with the receive buffer */
        STD_RETURN_TYPE_e commandBufferMatchesReceiveBuffer = STD_OK;
        for (uint8_t i = 0u; i < (pInstance5x->commandBufferCurrentLength - 1u); i++) {
            if (pInstance5x->commandBuffer[i] != pInstance5x->rxBuffer[i]) {
                commandBufferMatchesReceiveBuffer = STD_NOT_OK;
            }
        }
        /* update number of satellites */
        pInstance5x->numberOfSatellites = (uint8_t)(
            (pInstance5x->rxBuffer[HELLOALL_RX_LENGTH - 1u] - HELLOALL_START_SEED) & MXM_5X_BIT_MASK_ONE_BYTE);

        /* Plausibility check, compare with preset number of satellites */
        if (pInstance5x->numberOfSatellites == (BS_NR_OF_MODULES * BS_NR_OF_STRINGS)) {
            pInstance5x->numberOfSatellitesIsGood = STD_OK;
        }

        if (commandBufferMatchesReceiveBuffer == STD_NOT_OK) {
            /* TODO error handling */
        } else {
            MXM_5XSignalSuccess(pInstance5x);
        }
    } else {
        /* something is very broken */
        FAS_ASSERT(FAS_TRAP);
    }
}

static void MXM_5XStateHandlerWriteAll(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b) {
    FAS_ASSERT(pInstance5x != NULL_PTR);
    FAS_ASSERT(pInstance41b != NULL_PTR);
    if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_WRITEALL_UART_TRANSACTION);
    }

    if (pInstance5x->substate == MXM_5X_WRITEALL_UART_TRANSACTION) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e resultAddressCorrect = MXM_5XConstructCommandBufferWriteall(pInstance5x);
            FAS_ASSERT(resultAddressCorrect == STD_OK);
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_UART_TRANSACTION,
                pInstance5x->commandBuffer,
                pInstance5x->commandBufferCurrentLength,
                0,
                pInstance5x->rxBuffer,
                pInstance5x->commandBufferCurrentLength,
                &pInstance5x->status41b);
            /* TODO check CRC */
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XSignalSuccess(pInstance5x);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    }
}

static void MXM_5XStateHandlerWriteDevice(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b) {
    FAS_ASSERT(pInstance5x != NULL_PTR);
    FAS_ASSERT(pInstance41b != NULL_PTR);
    if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_WRITE_DEVICE_UART_TRANSACTION);
    }

    if (pInstance5x->substate == MXM_5X_WRITE_DEVICE_UART_TRANSACTION) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e resultAddressCorrect = MXM_5XConstructCommandBufferWriteDevice(pInstance5x);
            FAS_ASSERT(resultAddressCorrect == STD_OK);
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_UART_TRANSACTION,
                pInstance5x->commandBuffer,
                pInstance5x->commandBufferCurrentLength,
                0,
                pInstance5x->rxBuffer,
                pInstance5x->commandBufferCurrentLength,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XSignalSuccess(pInstance5x); /* TODO continue and check CRC */
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    }
}

static void MXM_5XStateHandlerReadAll(MXM_5X_INSTANCE_s *pInstance5x, MXM_41B_INSTANCE_s *pInstance41b) {
    FAS_ASSERT(pInstance5x != NULL_PTR);
    FAS_ASSERT(pInstance41b != NULL_PTR);
    if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
        /* entry of state --> set to first substate */
        MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_READALL_UART_TRANSACTION);
    }

    if (pInstance5x->substate == MXM_5X_READALL_UART_TRANSACTION) {
        if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
            const STD_RETURN_TYPE_e resultAddressCorrect = MXM_5XConstructCommandBufferReadall(pInstance5x);
            FAS_ASSERT(resultAddressCorrect == STD_OK);
            /* TODO parse rx buffer here into values and parse CRC before passing on*/
            /* stretch message length in order to accommodate 2 bytes per satellite */
            const STD_RETURN_TYPE_e stateRequestReturn = MXM_41BSetStateRequest(
                pInstance41b,
                MXM_STATEMACH_41B_UART_TRANSACTION,
                pInstance5x->commandBuffer,
                pInstance5x->commandBufferCurrentLength,
                2u * pInstance5x->numberOfSatellites,
                pInstance5x->rxBuffer,
                MXM_5X_RX_BUFFER_LEN,
                &pInstance5x->status41b);
            FAS_ASSERT(stateRequestReturn == STD_OK);
        } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
            /* wait for processing */
        } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
            MXM_5XHandle41BErrorState(pInstance5x);
        } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_READALL_CHECK_CRC);
        } else {
            FAS_ASSERT(FAS_TRAP);
        }
    } else if (pInstance5x->substate == MXM_5X_READALL_CHECK_CRC) {
        /* check CRC */
        if (MXM_CRC8(
                pInstance5x->rxBuffer,
                ((int32_t)pInstance5x->commandBufferCurrentLength + (2 * (int32_t)pInstance5x->numberOfSatellites))) ==
            0x00u) {
            MXM_5XTransitionToSubstate(pInstance5x, MXM_5X_READALL_GET_DC);
        } else {
            MXM_5XSignalError(pInstance5x);
        }
    } else if (pInstance5x->substate == MXM_5X_READALL_GET_DC) {
        /* get DC */ /* TODO check DC in this state */
        /* dc byte position is after data */
        FAS_ASSERT(((uint16_t)2u + ((uint16_t)2u * pInstance5x->numberOfSatellites)) <= (uint16_t)UINT8_MAX);
        uint8_t dc_byte_position = 2u + (2u * pInstance5x->numberOfSatellites);

        pInstance5x->lastDCByte = (uint8_t)(pInstance5x->rxBuffer[dc_byte_position] & MXM_5X_BIT_MASK_ONE_BYTE);

        MXM_5XSignalSuccess(pInstance5x);
    } else {
        FAS_ASSERT(FAS_TRAP);
    }
}

/*========== Extern Function Implementations ================================*/

extern void MXM_5X_InitializeStateStruct(MXM_5X_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    pInstance->state                        = MXM_STATEMACH_5X_UNINITIALIZED;
    pInstance->substate                     = MXM_5X_ENTRY_SUBSTATE;
    pInstance->commandPayload.regAddress    = MXM_REG_VERSION;
    pInstance->commandPayload.lsb           = 0u;
    pInstance->commandPayload.msb           = 0u;
    pInstance->commandPayload.blocksize     = 0u;
    pInstance->commandPayload.deviceAddress = 0u;
    pInstance->processed                    = NULL_PTR;
    pInstance->status41b                    = MXM_41B_STATE_UNSENT;
    pInstance->numberOfSatellites           = 0u;
    pInstance->numberOfSatellitesIsGood     = STD_NOT_OK;
    pInstance->lastDCByte                   = 0u;
    pInstance->errorCounter                 = 0u;
    pInstance->resetWaitTimestamp           = 0u;
    pInstance->commandBufferCurrentLength   = 0u;

    for (uint32_t i = 0u; i < COMMAND_BUFFER_LENGTH; i++) {
        pInstance->commandBuffer[i] = 0u;
    }

    for (uint32_t i = 0u; i < MXM_5X_RX_BUFFER_LEN; i++) {
        pInstance->rxBuffer[i] = 0u;
    }
}

extern STD_RETURN_TYPE_e MXM_5XGetRXBuffer(
    const MXM_5X_INSTANCE_s *const kpkInstance,
    uint8_t *rxBuffer,
    uint16_t rxBufferLength) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    FAS_ASSERT(rxBufferLength <= MXM_5X_RX_BUFFER_LEN);
    STD_RETURN_TYPE_e retval = STD_OK;

    if ((rxBuffer != NULL_PTR) && (rxBufferLength != 0u)) {
        for (uint16_t i = 0; i < rxBufferLength; i++) {
            if (i < MXM_5X_RX_BUFFER_LEN) {
                rxBuffer[i] = (uint8_t)(kpkInstance->rxBuffer[i] & MXM_5X_BIT_MASK_ONE_BYTE);
            }
        }
    } else {
        retval = STD_NOT_OK;
    }

    return retval;
}

extern MXM_DC_BYTE_e MXM_5XGetLastDCByte(const MXM_5X_INSTANCE_s *const kpkInstance) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    return (MXM_DC_BYTE_e)kpkInstance->lastDCByte;
}

extern uint8_t MXM_5XGetNumberOfSatellites(const MXM_5X_INSTANCE_s *const kpkInstance) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    const uint8_t numberOfSatellites = kpkInstance->numberOfSatellites;
    FAS_ASSERT(numberOfSatellites <= MXM_MAXIMUM_NR_OF_MODULES);
    return numberOfSatellites;
}

extern STD_RETURN_TYPE_e MXM_5XGetNumberOfSatellitesGood(const MXM_5X_INSTANCE_s *const kpkInstance) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    return kpkInstance->numberOfSatellitesIsGood;
}

extern STD_RETURN_TYPE_e MXM_5XSetStateRequest(
    MXM_5X_INSTANCE_s *pInstance5x,
    MXM_STATEMACHINE_5X_e state,
    MXM_5X_COMMAND_PAYLOAD_s commandPayload,
    MXM_5X_STATE_REQUEST_STATUS_e *processed) {
    FAS_ASSERT(pInstance5x != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;
    if (state >= MXM_STATEMACH_5X_MAXSTATE) {
        retval = STD_NOT_OK;
    } else if (processed == NULL_PTR) {
        retval = STD_NOT_OK;
    } else if (pInstance5x->state == MXM_STATEMACH_5X_UNINITIALIZED) {
        if (state == MXM_STATEMACH_5X_INIT) {
            pInstance5x->state          = state;
            pInstance5x->substate       = MXM_5X_ENTRY_SUBSTATE;
            pInstance5x->commandPayload = commandPayload;
            pInstance5x->processed      = processed;
            *pInstance5x->processed     = MXM_5X_STATE_UNPROCESSED;
        } else {
            retval = STD_NOT_OK;
        }
    } else if (pInstance5x->state == MXM_STATEMACH_5X_IDLE) {
        pInstance5x->state          = state;
        pInstance5x->substate       = MXM_5X_ENTRY_SUBSTATE;
        pInstance5x->commandPayload = commandPayload;
        pInstance5x->processed      = processed;
        *pInstance5x->processed     = MXM_5X_STATE_UNPROCESSED;
    } else {
        retval = STD_NOT_OK;
    }
    return retval;
}

void MXM_5XStateMachine(MXM_41B_INSTANCE_s *pInstance41b, MXM_5X_INSTANCE_s *pInstance5x) {
    FAS_ASSERT(pInstance41b != NULL_PTR);
    FAS_ASSERT(pInstance5x != NULL_PTR);

    /* failure handling */
    if (pInstance5x->errorCounter > MXM_5X_ERROR_THRESHOLD) {
        /* error, reset both this state-machine and the underlying */
        pInstance41b->state       = MXM_STATEMACH_41B_IDLE;
        pInstance41b->substate    = MXM_41B_ENTRY_SUBSTATE;
        pInstance41b->waitCounter = 0u;
        MXM_5XSignalError(pInstance5x);
        pInstance5x->errorCounter = 0u;
    }

    switch (pInstance5x->state) {
        case MXM_STATEMACH_5X_UNINITIALIZED:
            /* statemachine waits here for initialization */
            break;
        case MXM_STATEMACH_5X_IDLE:
            /* idle state currently does nothing */
            break;
        case MXM_STATEMACH_5X_41B_FMEA_CHECK:
            MXM_5XStateHandler41BFmeaCheck(pInstance5x, pInstance41b);
            break;
        case MXM_STATEMACH_5X_INIT:
            MXM_5XStateHandlerInit(pInstance5x, pInstance41b);
            break;
        case MXM_STATEMACH_5X_WRITEALL:
            MXM_5XStateHandlerWriteAll(pInstance5x, pInstance41b);
            break;
        case MXM_STATEMACH_5X_WRITE_DEVICE:
            MXM_5XStateHandlerWriteDevice(pInstance5x, pInstance41b);
            break;
        case MXM_STATEMACH_5X_READALL:
            MXM_5XStateHandlerReadAll(pInstance5x, pInstance41b);
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

extern STD_RETURN_TYPE_e must_check_return MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck(void) {
    /* check:
    * - user memory is contained in range 0x00 to 0x98
    * - reserved addresses in user address space:
    *   0x2C, 0x2D, 0x2E, 0x2F, 0x46 and 0x84 through 0x8B */

    /* AXIVION Disable Style Generic-NoMagicNumbers: This test function uses magic numbers to test predefined values. */
    /* expected #STD_OK */
    STD_RETURN_TYPE_e retval_check0 = MXM_5XIsUserAccessibleRegister(0x00u, MXM_MODEL_ID_MAX17852);
    STD_RETURN_TYPE_e retval_check1 = MXM_5XIsUserAccessibleRegister(0x42u, MXM_MODEL_ID_MAX17852);
    STD_RETURN_TYPE_e retval_check2 = MXM_5XIsUserAccessibleRegister(0x98u, MXM_MODEL_ID_MAX17852);

    STD_RETURN_TYPE_e retval_check3 = MXM_5XIsUserAccessibleRegister(0x00u, MXM_MODEL_ID_MAX17852);
    STD_RETURN_TYPE_e retval_check4 = MXM_5XIsUserAccessibleRegister(0x42u, MXM_MODEL_ID_MAX17852);
    STD_RETURN_TYPE_e retval_check5 = MXM_5XIsUserAccessibleRegister(0x98u, MXM_MODEL_ID_MAX17852);

    STD_RETURN_TYPE_e retval_check6 = MXM_5XIsUserAccessibleRegister(0x00u, MXM_MODEL_ID_MAX17853);
    STD_RETURN_TYPE_e retval_check7 = MXM_5XIsUserAccessibleRegister(0x42u, MXM_MODEL_ID_MAX17853);
    STD_RETURN_TYPE_e retval_check8 = MXM_5XIsUserAccessibleRegister(0x98u, MXM_MODEL_ID_MAX17853);

    /* expected #STD_NOT_OK */
    STD_RETURN_TYPE_e retval_check9  = MXM_5XIsUserAccessibleRegister(0x2Cu, MXM_MODEL_ID_MAX17853);
    STD_RETURN_TYPE_e retval_check10 = MXM_5XIsUserAccessibleRegister(0x2Eu, MXM_MODEL_ID_MAX17853);
    STD_RETURN_TYPE_e retval_check11 = MXM_5XIsUserAccessibleRegister(0x2Fu, MXM_MODEL_ID_MAX17853);
    STD_RETURN_TYPE_e retval_check12 = MXM_5XIsUserAccessibleRegister(0x46u, MXM_MODEL_ID_MAX17853);
    STD_RETURN_TYPE_e retval_check13 = MXM_5XIsUserAccessibleRegister(0x84u, MXM_MODEL_ID_MAX17853);
    STD_RETURN_TYPE_e retval_check14 = MXM_5XIsUserAccessibleRegister(0x8Bu, MXM_MODEL_ID_MAX17853);

    STD_RETURN_TYPE_e retval_check15 = MXM_5XIsUserAccessibleRegister(0x99u, MXM_MODEL_ID_MAX17852);
    STD_RETURN_TYPE_e retval_check16 = MXM_5XIsUserAccessibleRegister(0x99u, MXM_MODEL_ID_MAX17852);
    STD_RETURN_TYPE_e retval_check17 = MXM_5XIsUserAccessibleRegister(0x99u, MXM_MODEL_ID_MAX17853);

    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    if ((retval_check0 == STD_OK) && (retval_check1 == STD_OK) && (retval_check2 == STD_OK) &&
        (retval_check3 == STD_OK) && (retval_check4 == STD_OK) && (retval_check5 == STD_OK) &&
        (retval_check6 == STD_OK) && (retval_check7 == STD_OK) && (retval_check8 == STD_OK) &&
        (retval_check9 == STD_NOT_OK) && (retval_check10 == STD_NOT_OK) && (retval_check11 == STD_NOT_OK) &&
        (retval_check12 == STD_NOT_OK) && (retval_check13 == STD_NOT_OK) && (retval_check14 == STD_NOT_OK) &&
        (retval_check15 == STD_NOT_OK) && (retval_check16 == STD_NOT_OK) && (retval_check17 == STD_NOT_OK)) {
        retval = STD_OK;
    }
    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
