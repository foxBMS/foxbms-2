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
#include "mxm_battery_management.h"

/*========== Macros and Definitions =========================================*/

/** length of the helloall command @{*/
#define HELLOALL_TX_LENGTH (3u)
#define HELLOALL_RX_LENGTH HELLOALL_TX_LENGTH
/**@}*/

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
 *          range. This range is specified in the datasheet of the monitoring
 *          IC as following:
 *              - user memory is contained in the range 0x00 to 0x98
 *              - reserved addresses in the user address space are:
 *                  - 0x2C, 0x2D, 0x2E, 0x2F
 *                  - 0x46
 *                  - 0x84 through 0x8B
 * @param[in]   regAddress register address to be checked
 * @return      #STD_OK if the register address is good, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e MXM_5XIsUserAccessibleRegister(uint8_t regAddress);

/**
 * @brief   Check if a register address is user accessible in MAX17852
 * @details Checks if a register address is inside the user accessible memory
 *          range.
 *          This range is specified in the datasheet of the monitoring IC.
 * @param[in]   regAddress register address to be checked
 * @return      #STD_OK if the register address is good, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e MXM_52IsUserAccessibleRegister(uint8_t regAddress);

/**
 * @brief   Check if a register address is user accessible in MAX17853
 * @details Checks if a register address is inside the user accessible memory
 *          range.
 *          This range is specified in the datasheet of the monitoring IC as
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
 *          in the same register.
 * @param[in,out]   pInstance   pointer to the state-struct
 * @param[in]       regAddress  address of the register that should be written
 * @param[in]       dataLSB     LSB of the register that should be written
 * @param[in]       dataMSB     MSB of the register that should be written
 * @return      #STD_OK if an accessible register address has been selected,
 *              #STD_NOT_OK if not.
 */
static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteall(
    MXM_5X_INSTANCE_s *pInstance,
    uint8_t regAddress,
    uint8_t dataLSB,
    uint8_t dataMSB);

/**
 * @brief   clears the command buffer and writes a WRITEDEVICE message
 * @details Fills the command buffer with a WRITEDEVICE message. This message
 *          is addressed to one specific device in the daisy-chain. Therefore
 *          the address of the device has to be supplied together with the
 *          register and the data that should be written.
 * @param[in,out]   pInstance       pointer to the state-struct
 * @param[in]       deviceAddress   address for the satellite that should be
 *                                  written to
 * @param[in]       regAddress      address of the register that should be written
 * @param[in]       dataLSB         LSB of the register that should be written
 * @param[in]       dataMSB         MSB of the register that should be written
 * @return      #STD_OK if an accessible register address has been selected,
 *              #STD_NOT_OK if not.
 */
static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteDevice(
    MXM_5X_INSTANCE_s *pInstance,
    uint8_t deviceAddress,
    uint8_t regAddress,
    uint8_t dataLSB,
    uint8_t dataMSB);

/**
 * @brief   clears the command buffer and writes READALL into the buffer
 * @details Fills the command buffer with a READALL command. This command
 *          retrieves the LSB and MSB of exactly one register of every device
 *          in the daisy-chain.
 * @param[in,out]   pInstance   pointer to the state-struct
 * @param[in]       regAddress  address of the register that should be read
 * @return      #STD_OK if an accessible register address has been selected,
 *              #STD_NOT_OK if not.
 */
static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferReadall(MXM_5X_INSTANCE_s *pInstance, uint8_t regAddress);

/*========== Static Function Implementations ================================*/
static void MXM_5XClearCommandBuffer(MXM_5X_INSTANCE_s *pInstance) {
    for (uint8_t i = 0; i < COMMAND_BUFFER_LENGTH; i++) {
        pInstance->commandBuffer[i] = 0x00U;
    }
    pInstance->commandBufferCurrentLength = COMMAND_BUFFER_LENGTH;
}

static STD_RETURN_TYPE_e MXM_5XIsUserAccessibleRegister(uint8_t regAddress) {
    STD_RETURN_TYPE_e retval = STD_OK;
    /* check if regAddress is in user-accessible area */
    if (regAddress <= 0x98U) {
        /* overall range is good, check for exceptions */

        /* TODO make selectable which chip is used */

        /* MAX17852 */
        if ((regAddress == 0x5DU) || (regAddress == 0x5EU)) {
            retval = STD_NOT_OK;
        }

    } else {
        /* regAddress is outside user-accessible range */
        retval = STD_NOT_OK;
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_52IsUserAccessibleRegister(uint8_t regAddress) {
    STD_RETURN_TYPE_e retval = STD_OK;
    /* check if regAddress is in user-accessible area */
    if (regAddress <= 0x98U) {
        /* overall range is good, check for exceptions */
        /* MAX17852 */
        if ((regAddress == 0x5DU) || (regAddress == 0x5EU)) {
            retval = STD_NOT_OK;
        }
    } else {
        /* regAddress is outside user-accessible range */
        retval = STD_NOT_OK;
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_53IsUserAccessibleRegister(uint8_t regAddress) {
    STD_RETURN_TYPE_e retval = STD_OK;
    /* check if regAddress is in user-accessible area */
    if (regAddress <= 0x98U) {
        /* overall range is good, check for exceptions */
        if ((regAddress == 0x46U) || ((0x2CU <= regAddress) && (regAddress <= 0x2FU)) ||
            ((0x84U <= regAddress) && (regAddress <= 0x8BU))) {
            /* regAddress is inside one of the exceptions */
            retval = STD_NOT_OK;
        }
    } else {
        /* regAddress is outside user-accessible range */
        retval = STD_NOT_OK;
    }
    return retval;
}

static void MXM_5XConstructCommandBufferHelloall(MXM_5X_INSTANCE_s *pInstance) {
    MXM_5XClearCommandBuffer(pInstance);
    pInstance->commandBuffer[0]           = BATTERY_MANAGEMENT_HELLOALL;
    pInstance->commandBuffer[1]           = 0x00;
    pInstance->commandBuffer[2]           = HELLOALL_START_SEED;
    pInstance->commandBufferCurrentLength = 3;
}

static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteall(
    MXM_5X_INSTANCE_s *pInstance,
    uint8_t regAddress,
    uint8_t dataLSB,
    uint8_t dataMSB) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    if (MXM_5XIsUserAccessibleRegister(regAddress) == STD_OK) {
        /* clear command buffer */
        MXM_5XClearCommandBuffer(pInstance);

        /* construct command buffer */
        pInstance->commandBuffer[0] = BATTERY_MANAGEMENT_WRITEALL;
        pInstance->commandBuffer[1] = regAddress;
        pInstance->commandBuffer[2] = dataLSB;
        pInstance->commandBuffer[3] = dataMSB;
        /* PEC byte */
        pInstance->commandBuffer[4] = MXM_CRC8(pInstance->commandBuffer, 4);
        /* TODO alive-counter? */
        pInstance->commandBufferCurrentLength = 5;
        retval                                = STD_OK;
    }

    return retval;
}

static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferWriteDevice(
    MXM_5X_INSTANCE_s *pInstance,
    uint8_t deviceAddress,
    uint8_t regAddress,
    uint8_t dataLSB,
    uint8_t dataMSB) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    if (MXM_5XIsUserAccessibleRegister(regAddress) == STD_OK) {
        /* clear command buffer */
        MXM_5XClearCommandBuffer(pInstance);

        /* construct command buffer */

        /* commandBuffer[0] = Device address in a daisy chain + 0b100
         * DA = deviceAddress
         * Bit     |   7   |   6   |   5   |   4   |   3   | 2 | 1 | 0
         * Content | DA[4] | DA[3] | DA[2] | DA[1] | DA[0] | 1 | 0 | 0
         */
        pInstance->commandBuffer[0] = ((deviceAddress << 3) & 0xF8u) | BATTERY_MANAGEMENT_WRITEDEVICE;
        pInstance->commandBuffer[1] = regAddress;
        pInstance->commandBuffer[2] = dataLSB;
        pInstance->commandBuffer[3] = dataMSB;
        /* PEC byte */
        pInstance->commandBuffer[4] = MXM_CRC8(pInstance->commandBuffer, 4);
        /* TODO alive-counter? */
        pInstance->commandBufferCurrentLength = 5;
        retval                                = STD_OK;
    }

    return retval;
}

static STD_RETURN_TYPE_e MXM_5XConstructCommandBufferReadall(MXM_5X_INSTANCE_s *pInstance, uint8_t regAddress) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    /* TODO test these functions */

    if (MXM_5XIsUserAccessibleRegister(regAddress) == STD_OK) {
        /* clear command buffer */
        MXM_5XClearCommandBuffer(pInstance);

        /* construct command buffer */
        pInstance->commandBuffer[0] = BATTERY_MANAGEMENT_READALL;
        pInstance->commandBuffer[1] = regAddress;
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

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e MXM_5XGetRXBuffer(
    const MXM_5X_INSTANCE_s *const kpkInstance,
    uint8_t *rxBuffer,
    uint16_t rxBufferLength) {
    STD_RETURN_TYPE_e retval = STD_OK;

    if ((rxBuffer != NULL_PTR) && (rxBufferLength != 0u)) {
        for (uint16_t i = 0; i < rxBufferLength; i++) {
            if (i < MXM_5X_RX_BUFFER_LEN) {
                rxBuffer[i] = (uint8_t)kpkInstance->rxBuffer[i];
            } else {
                rxBuffer[i] = 0;
            }
        }
    } else {
        retval = STD_NOT_OK;
    }

    return retval;
}

extern MXM_DC_BYTE_e MXM_5XGetLastDCByte(const MXM_5X_INSTANCE_s *const kpkInstance) {
    return (MXM_DC_BYTE_e)kpkInstance->lastDCByte;
}

extern uint8_t MXM_5XGetNumberOfSatellites(const MXM_5X_INSTANCE_s *const kpkInstance) {
    return kpkInstance->numberOfSatellites;
}

extern STD_RETURN_TYPE_e MXM_5XGetNumberOfSatellitesGood(const MXM_5X_INSTANCE_s *const kpkInstance) {
    return kpkInstance->numberOfSatellitesIsGood;
}

extern STD_RETURN_TYPE_e MXM_5XSetStateRequest(
    MXM_5X_INSTANCE_s *pInstance5x,
    MXM_STATEMACHINE_5X_e state,
    MXM_5X_COMMAND_PAYLOAD_s commandPayload,
    MXM_5X_STATE_REQUEST_STATUS_e *processed) {
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
    STD_RETURN_TYPE_e retval;
    switch (pInstance5x->state) {
        case MXM_STATEMACH_5X_UNINITIALIZED:
            /* statemachine waits here for initialization */
            break;
        case MXM_STATEMACH_5X_IDLE:
            /* TODO idle state */
            break;
        case MXM_STATEMACH_5X_41B_FMEA_CHECK:
            if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance5x->substate  = MXM_5X_41B_FMEA_REQUEST;
                pInstance5x->status41b = MXM_41B_STATE_UNSENT;
            }

            if (pInstance5x->substate == MXM_5X_41B_FMEA_REQUEST) {
                retval = MXM_41BSetStateRequest(
                    pInstance41b, MXM_STATEMACH_41B_CHECK_FMEA, NULL_PTR, 0, 0, NULL_PTR, 0, &pInstance5x->status41b);
                if (retval == STD_OK) {
                    pInstance5x->substate = MXM_5X_41B_FMEA_CHECK;
                } else {
                    /* stay here */
                }
            } else if (pInstance5x->substate == MXM_5X_41B_FMEA_CHECK) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    *pInstance5x->processed = MXM_5X_STATE_ERROR;
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    /* TODO continue */
                    *pInstance5x->processed = MXM_5X_STATE_PROCESSED;
                    pInstance5x->substate   = MXM_5X_ENTRY_SUBSTATE;
                    pInstance5x->status41b  = MXM_41B_STATE_UNSENT;
                    pInstance5x->state      = MXM_STATEMACH_5X_IDLE;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else {
                /* something is very broken */
                *pInstance5x->processed = MXM_5X_STATE_ERROR;
            }
            break;
        case MXM_STATEMACH_5X_INIT:
            if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance5x->substate  = MXM_5X_INIT_41B_INIT;
                pInstance5x->status41b = MXM_41B_STATE_UNSENT;
            }

            if (pInstance5x->substate == MXM_5X_INIT_41B_INIT) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BSetStateRequest(
                        pInstance41b, MXM_STATEMACH_41B_INIT, NULL_PTR, 0, 0, NULL_PTR, 0, &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    /* TODO continue */
                    pInstance5x->substate  = MXM_5X_INIT_ENABLE_KEEP_ALIVE;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_ENABLE_KEEP_ALIVE) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BWriteRegisterFunction(
                        pInstance41b, MXM_41B_REG_FUNCTION_KEEP_ALIVE, mxm_kConfig3KeepAlive160us41BRegister);
                    if (retval == STD_OK) {
                        retval = MXM_41BSetStateRequest(
                            pInstance41b,
                            MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                            NULL_PTR,
                            0,
                            0,
                            NULL_PTR,
                            0,
                            &pInstance5x->status41b);
                    }
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    /* TODO continue */
                    pInstance5x->substate  = MXM_5X_INIT_ENABLE_RX_INTERRUPT_FLAGS;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_ENABLE_RX_INTERRUPT_FLAGS) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval =
                        MXM_41BWriteRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_RX_ERROR_INT, MXM_41B_REG_TRUE);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                    retval = MXM_41BWriteRegisterFunction(
                        pInstance41b, MXM_41B_REG_FUNCTION_RX_OVERFLOW_INT, MXM_41B_REG_TRUE);
                    if (retval == STD_OK) {
                        retval = MXM_41BSetStateRequest(
                            pInstance41b,
                            MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                            NULL_PTR,
                            0,
                            0,
                            NULL_PTR,
                            0,
                            &pInstance5x->status41b);
                    }
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    /* TODO continue */
                    pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER,
                        NULL_PTR,
                        0,
                        0,
                        NULL_PTR,
                        0,
                        &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    /* TODO continue */
                    /* TODO next step */
                    pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_EN_PREAMBLES;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_EN_PREAMBLES) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval =
                        MXM_41BWriteRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_TX_PREAMBLES, MXM_41B_REG_TRUE);
                    if (retval == STD_OK) {
                        retval = MXM_41BSetStateRequest(
                            pInstance41b,
                            MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                            NULL_PTR,
                            0,
                            0,
                            NULL_PTR,
                            0,
                            &pInstance5x->status41b);
                    }
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_BUSY;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_BUSY) {
                /* TODO wait for rx status change busy */
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_READ_STATUS_REGISTER,
                        NULL_PTR,
                        0,
                        0,
                        NULL_PTR,
                        0,
                        &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    MXM_41B_REG_BIT_VALUE functionValue;
                    retval =
                        MXM_41BReadRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_RX_BUSY_STATUS, &functionValue);
                    if (retval == STD_NOT_OK) {
                        /* TODO this should not happen, did we use the right enum? */
                    } else {
                        if (functionValue == MXM_41B_REG_FALSE) {
                            /* repeat this state */
                            pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                        } else if (functionValue == MXM_41B_REG_TRUE) {
                            pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_DIS_PREAMBLES;
                            pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                        } else {
                            FAS_ASSERT(FAS_TRAP);
                        }
                    }
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_DIS_PREAMBLES) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BWriteRegisterFunction(
                        pInstance41b, MXM_41B_REG_FUNCTION_TX_PREAMBLES, MXM_41B_REG_FALSE);
                    if (retval == STD_OK) {
                        retval = MXM_41BSetStateRequest(
                            pInstance41b,
                            MXM_STATEMACH_41B_WRITE_CONF_AND_INT_REGISTER,
                            NULL_PTR,
                            0,
                            0,
                            NULL_PTR,
                            0,
                            &pInstance5x->status41b);
                    }
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_EMPTY;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_EMPTY) {
                /* TODO wait for rx status change busy */
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_READ_STATUS_REGISTER,
                        NULL_PTR,
                        0,
                        0,
                        NULL_PTR,
                        0,
                        &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    MXM_41B_REG_BIT_VALUE functionValue;
                    retval =
                        MXM_41BReadRegisterFunction(pInstance41b, MXM_41B_REG_FUNCTION_RX_EMPTY_STATUS, &functionValue);
                    if (retval == STD_NOT_OK) {
                        /* TODO this should not happen, did we use the right enum? */
                    } else {
                        if (functionValue == MXM_41B_REG_TRUE) {
                            /* repeat this state until rx_empty_status is 0 */
                            pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                        } else if (functionValue == MXM_41B_REG_FALSE) {
                            pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_TRANSMIT_BUFFER;
                            pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                        } else {
                            FAS_ASSERT(FAS_TRAP);
                        }
                    }
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_TRANSMIT_BUFFER) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_CLEAR_TRANSMIT_BUFFER,
                        NULL_PTR,
                        0,
                        0,
                        NULL_PTR,
                        0,
                        &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER_2;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER_2) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_CLEAR_RECEIVE_BUFFER,
                        NULL_PTR,
                        0,
                        0,
                        NULL_PTR,
                        0,
                        &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* TODO error handling */
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate  = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    MXM_5XConstructCommandBufferHelloall(pInstance5x);
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_UART_TRANSACTION,
                        pInstance5x->commandBuffer,
                        pInstance5x->commandBufferCurrentLength,
                        0,
                        pInstance5x->rxBuffer,
                        HELLOALL_RX_LENGTH,
                        &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* reset state-machine */
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate = MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL_VERIFY_MSG_AND_COUNT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
                /* TODO check for receive buffer errors and handle */
            } else if (pInstance5x->substate == MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL_VERIFY_MSG_AND_COUNT) {
                /* check if the commandBuffer matches with the receive buffer */
                retval = STD_OK;
                for (uint8_t i = 0u; i < (pInstance5x->commandBufferCurrentLength - 1u); i++) {
                    if (pInstance5x->commandBuffer[i] != pInstance5x->rxBuffer[i]) {
                        retval = STD_NOT_OK;
                    }
                }
                /* update number of satellites
             */
                pInstance5x->numberOfSatellites =
                    (uint8_t)(pInstance5x->rxBuffer[HELLOALL_RX_LENGTH - 1u] - HELLOALL_START_SEED);

                /*
                 * Plausibility check, compare with preset number of satellites
                 */
                if (pInstance5x->numberOfSatellites == (BS_NR_OF_MODULES * BS_NR_OF_STRINGS)) {
                    pInstance5x->numberOfSatellitesIsGood = STD_OK;
                }

                if (retval == STD_NOT_OK) {
                    /* TODO error handling */
                } else {
                    pInstance5x->substate   = MXM_5X_ENTRY_SUBSTATE;
                    pInstance5x->status41b  = MXM_41B_STATE_UNSENT;
                    pInstance5x->state      = MXM_STATEMACH_5X_IDLE;
                    *pInstance5x->processed = MXM_5X_STATE_PROCESSED;
                }
            } else {
                /* something is very broken */
                *pInstance5x->processed = MXM_5X_STATE_ERROR;
            }
            break;
        case MXM_STATEMACH_5X_WRITEALL:
            if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance5x->substate  = MXM_5X_WRITEALL_UART_TRANSACTION;
                pInstance5x->status41b = MXM_41B_STATE_UNSENT;
            }

            if (pInstance5x->substate == MXM_5X_WRITEALL_UART_TRANSACTION) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    MXM_5XConstructCommandBufferWriteall(
                        pInstance5x,
                        pInstance5x->commandPayload.regAddress,
                        pInstance5x->commandPayload.lsb,
                        pInstance5x->commandPayload.msb);
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_UART_TRANSACTION,
                        pInstance5x->commandBuffer,
                        pInstance5x->commandBufferCurrentLength,
                        0,
                        pInstance5x->rxBuffer,
                        pInstance5x->commandBufferCurrentLength,
                        &pInstance5x->status41b);
                    /* TODO check CRC */
                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* reset state-machine */
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate   = MXM_5X_ENTRY_SUBSTATE;
                    pInstance5x->status41b  = MXM_41B_STATE_UNSENT;
                    pInstance5x->state      = MXM_STATEMACH_5X_IDLE;
                    *pInstance5x->processed = MXM_5X_STATE_PROCESSED; /* TODO continue */
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            }
            break;
        case MXM_STATEMACH_5X_WRITE_DEVICE:
            if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance5x->substate  = MXM_5X_WRITE_DEVICE_UART_TRANSACTION;
                pInstance5x->status41b = MXM_41B_STATE_UNSENT;
            }

            if (pInstance5x->substate == MXM_5X_WRITE_DEVICE_UART_TRANSACTION) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    MXM_5XConstructCommandBufferWriteDevice(
                        pInstance5x,
                        pInstance5x->commandPayload.deviceAddress,
                        pInstance5x->commandPayload.regAddress,
                        pInstance5x->commandPayload.lsb,
                        pInstance5x->commandPayload.msb);
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_UART_TRANSACTION,
                        pInstance5x->commandBuffer,
                        pInstance5x->commandBufferCurrentLength,
                        0,
                        pInstance5x->rxBuffer,
                        pInstance5x->commandBufferCurrentLength,
                        &pInstance5x->status41b);
                    if (retval == STD_NOT_OK) {
                        *pInstance5x->processed = MXM_5X_STATE_ERROR;
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* reset state-machine */
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate   = MXM_5X_ENTRY_SUBSTATE;
                    pInstance5x->status41b  = MXM_41B_STATE_UNSENT;
                    pInstance5x->state      = MXM_STATEMACH_5X_IDLE;
                    *pInstance5x->processed = MXM_5X_STATE_PROCESSED; /* TODO continue and check CRC */
                } else {
                    /* This should never happen */
                    *pInstance5x->processed = MXM_5X_STATE_ERROR;
                }
            }
            break;
        case MXM_STATEMACH_5X_READALL:
            if (pInstance5x->substate == MXM_5X_ENTRY_SUBSTATE) {
                /* entry of state --> set to first substate */
                pInstance5x->substate  = MXM_5X_READALL_UART_TRANSACTION;
                pInstance5x->status41b = MXM_41B_STATE_UNSENT;
            }

            if (pInstance5x->substate == MXM_5X_READALL_UART_TRANSACTION) {
                if (pInstance5x->status41b == MXM_41B_STATE_UNSENT) {
                    MXM_5XConstructCommandBufferReadall(pInstance5x, pInstance5x->commandPayload.regAddress);
                    /* TODO parse rx buffer here into values and parse CRC before passing on*/
                    /* stretch message length in order to accommodate 2 bytes per satellite */
                    retval = MXM_41BSetStateRequest(
                        pInstance41b,
                        MXM_STATEMACH_41B_UART_TRANSACTION,
                        pInstance5x->commandBuffer,
                        pInstance5x->commandBufferCurrentLength,
                        2u * pInstance5x->numberOfSatellites,
                        pInstance5x->rxBuffer,
                        MXM_5X_RX_BUFFER_LEN,
                        &pInstance5x->status41b);

                    if (retval == STD_NOT_OK) {
                        /* TODO error handling */
                    }
                } else if (pInstance5x->status41b == MXM_41B_STATE_UNPROCESSED) {
                    /* wait for processing
                 * TODO implement timeout? */
                } else if (pInstance5x->status41b == MXM_41B_STATE_ERROR) {
                    /* reset state-machine */
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else if (pInstance5x->status41b == MXM_41B_STATE_PROCESSED) {
                    pInstance5x->substate  = MXM_5X_READALL_CHECK_CRC;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    FAS_ASSERT(FAS_TRAP);
                }
            } else if (pInstance5x->substate == MXM_5X_READALL_CHECK_CRC) {
                /* check CRC */
                if (MXM_CRC8(
                        pInstance5x->rxBuffer,
                        pInstance5x->commandBufferCurrentLength + (2u * pInstance5x->numberOfSatellites)) == 0x00u) {
                    pInstance5x->substate  = MXM_5X_READALL_GET_DC;
                    pInstance5x->status41b = MXM_41B_STATE_UNSENT;
                } else {
                    *pInstance5x->processed = MXM_5X_STATE_ERROR;
                    pInstance5x->substate   = MXM_5X_ENTRY_SUBSTATE;
                    pInstance5x->status41b  = MXM_41B_STATE_UNSENT;
                }
            } else if (pInstance5x->substate == MXM_5X_READALL_GET_DC) {
                /* get DC */ /* TODO check DC in this state */
                /* dc byte position is after data */
                uint8_t dc_byte_position = 2u + (2u * pInstance5x->numberOfSatellites);

                pInstance5x->lastDCByte = (uint8_t)pInstance5x->rxBuffer[dc_byte_position];

                pInstance5x->substate   = MXM_5X_ENTRY_SUBSTATE;
                pInstance5x->status41b  = MXM_41B_STATE_UNSENT;
                pInstance5x->state      = MXM_STATEMACH_5X_IDLE;
                *pInstance5x->processed = MXM_5X_STATE_PROCESSED; /* TODO continue */
            } else {
                /* something is very broken */
                *pInstance5x->processed = MXM_5X_STATE_ERROR;
            }
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

extern STD_RETURN_TYPE_e must_check_return MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck(void) {
    STD_RETURN_TYPE_e retval         = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check0  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check1  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check2  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check3  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check4  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check5  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check6  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check7  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check8  = STD_NOT_OK;
    STD_RETURN_TYPE_e retval_check9  = STD_OK;
    STD_RETURN_TYPE_e retval_check10 = STD_OK;
    STD_RETURN_TYPE_e retval_check11 = STD_OK;
    STD_RETURN_TYPE_e retval_check12 = STD_OK;
    STD_RETURN_TYPE_e retval_check13 = STD_OK;
    STD_RETURN_TYPE_e retval_check14 = STD_OK;
    STD_RETURN_TYPE_e retval_check15 = STD_OK;
    STD_RETURN_TYPE_e retval_check16 = STD_OK;
    STD_RETURN_TYPE_e retval_check17 = STD_OK;

    /* check:
    * - user memory is contained in range 0x00 to 0x98
    * - reserved addresses in user address space:
    *   0x2C, 0x2D, 0x2E, 0x2F, 0x46 and 0x84 through 0x8B */

    /* expected #STD_OK */
    retval_check0 = MXM_5XIsUserAccessibleRegister(0x00U);
    retval_check1 = MXM_5XIsUserAccessibleRegister(0x42U);
    retval_check2 = MXM_5XIsUserAccessibleRegister(0x98U);

    retval_check3 = MXM_52IsUserAccessibleRegister(0x00U);
    retval_check4 = MXM_52IsUserAccessibleRegister(0x42U);
    retval_check5 = MXM_52IsUserAccessibleRegister(0x98U);

    retval_check6 = MXM_53IsUserAccessibleRegister(0x00U);
    retval_check7 = MXM_53IsUserAccessibleRegister(0x42U);
    retval_check8 = MXM_53IsUserAccessibleRegister(0x98U);

    /* expected #STD_NOT_OK */
    retval_check9  = MXM_53IsUserAccessibleRegister(0x2CU);
    retval_check10 = MXM_53IsUserAccessibleRegister(0x2EU);
    retval_check11 = MXM_53IsUserAccessibleRegister(0x2FU);
    retval_check12 = MXM_53IsUserAccessibleRegister(0x46U);
    retval_check13 = MXM_53IsUserAccessibleRegister(0x84U);
    retval_check14 = MXM_53IsUserAccessibleRegister(0x8BU);

    retval_check15 = MXM_5XIsUserAccessibleRegister(0x99U);
    retval_check16 = MXM_52IsUserAccessibleRegister(0x99U);
    retval_check17 = MXM_53IsUserAccessibleRegister(0x99U);

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
