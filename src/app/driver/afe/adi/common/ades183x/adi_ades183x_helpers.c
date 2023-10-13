/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    adi_ades183x_helpers.c
 * @author  foxBMS Team
 * @date    2022-12-06 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Helper functionalities specific to the driver for the ADI ades183x
 *
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "adi_ades183x.h"
/* clang-format on */

#include "adi_ades183x_helpers.h"

#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_pec.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Increment command counter of AFE driver.
 * @param   adiState state of the driver
 */
static void ADI_IncrementCommandCounter(ADI_STATE_s *adiState);

/**
 * @brief   Writes data in the configuration stored in AFE driver,
 *          for a specific AFE.
 * @details The driver stores the AFE configuration to write to the AFEs.
 *          This function sets the data for a specific register for a specific
 *          AFE in the daisy chain. The parameter registerSet selects one
 *          configuration register (A or B). Offset corresponds to one of the
 *          bytes in the selected register.
 * @param   module          module number
 * @param   registerSet     configuration register to address
 * @param   registerOffset  position of byte to address in configuration register
 * @param   data            data to write to byte in register
 * @param   position        position where to store the data in byte
 * @param   mask            mask corresponding to data to store
 * @param   adiState        state of the driver
 */
static void ADI_StoredConfigurationFillRegisterData(
    uint8_t module,
    ADI_CFG_REGISTER_SET_e registerSet,
    uint8_t registerOffset,
    uint8_t data,
    uint8_t position,
    uint8_t mask,
    ADI_STATE_s *adiState);

/*========== Static Function Implementations ================================*/
static void ADI_StoredConfigurationFillRegisterData(
    uint8_t module,
    ADI_CFG_REGISTER_SET_e registerSet,
    uint8_t registerOffset,
    uint8_t data,
    uint8_t position,
    uint8_t mask,
    ADI_STATE_s *adiState) {
    /* AXIVION Routine Generic-MissingParameterAssert: module: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: data: parameter accepts whole range */
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(registerSet <= ADI_CFG_REGISTER_SET_B);
    /* Only 8 bit in a register part */
    FAS_ASSERT(position <= ADI_MAX_BIT_POSITION_IN_BYTE);
    /* All 0s mask would mean do nothing */
    FAS_ASSERT(mask > 0u);
    FAS_ASSERT(registerOffset <= ADI_REGISTER_OFFSET5);

    if (registerSet == ADI_CFG_REGISTER_SET_A) {
        ADI_WriteDataBits(
            &adi_configurationRegisterAgroup[adiState->currentString][(module * ADI_WRCFGA_LEN) + registerOffset],
            data,
            position,
            mask);
    } else if (registerSet == ADI_CFG_REGISTER_SET_B) {
        ADI_WriteDataBits(
            &adi_configurationRegisterBgroup[adiState->currentString][(module * ADI_WRCFGB_LEN) + registerOffset],
            data,
            position,
            mask);
    } else {                  /* invalid register set */
        FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
    }
}

static void ADI_IncrementCommandCounter(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /**
     *  SM_SPI_CNT: SPI Frame Counter
     *  Increment driver stored value of command counter if command causes increase.
     */
    for (uint16_t i = 0u; i < ADI_N_ADI; i++) {
        if (adiState->data.commandCounter[adiState->currentString][i] < ADI_COMMAND_COUNTER_MAX_VALUE) {
            adiState->data.commandCounter[adiState->currentString][i]++;
        } else {
            adiState->data.commandCounter[adiState->currentString][i] = ADI_COMMAND_COUNTER_RESTART_VALUE;
        }
    }
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_e registerSet, ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(registerSet <= ADI_CFG_REGISTER_SET_B);

    STD_RETURN_TYPE_e returnValue = STD_OK;
    bool configurationAIsOk       = true;
    bool configurationBIsOk       = true;

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        /* Test configuration register A */
        for (uint8_t byte = 0; byte < ADI_WRCFGA_LEN; byte++) {
            if (adi_configurationRegisterAgroup[adiState->currentString][byte + (m * ADI_WRCFGA_LEN)] !=
                adi_readConfigurationRegisterAgroup[adiState->currentString][byte + (m * ADI_WRCFGA_LEN)]) {
                configurationAIsOk = false;
                returnValue        = STD_NOT_OK;
                break;
            }
        }
        if (configurationAIsOk == false) {
            adiState->data.errorTable->configurationAIsOk[adiState->currentString][m] = false;
        } else {
            adiState->data.errorTable->configurationAIsOk[adiState->currentString][m] = true;
        }
        configurationAIsOk = true;
        /* Test configuration register B */
        for (uint8_t byte = 0; byte < ADI_WRCFGA_LEN; byte++) {
            if (adi_configurationRegisterBgroup[adiState->currentString][byte + (m * ADI_WRCFGB_LEN)] !=
                adi_readConfigurationRegisterBgroup[adiState->currentString][byte + (m * ADI_WRCFGB_LEN)]) {
                configurationBIsOk = false;
                returnValue        = STD_NOT_OK;
                break;
            }
        }
        if (configurationBIsOk == false) {
            adiState->data.errorTable->configurationBIsOk[adiState->currentString][m] = false;
        } else {
            adiState->data.errorTable->configurationBIsOk[adiState->currentString][m] = true;
        }
        configurationBIsOk = true;
    }
    return returnValue;
}

extern void ADI_WriteCommandConfigurationBits(
    uint16_t *configuredCommand,
    uint8_t position,
    uint8_t length,
    uint16_t configuration) {
    FAS_ASSERT(configuredCommand != NULL_PTR);
    FAS_ASSERT(length > 0u);
    FAS_ASSERT(length <= ADI_COMMAND_CODE_LENGTH);
    FAS_ASSERT(position <= (ADI_COMMAND_CODE_LENGTH - 1u));
    FAS_ASSERT(configuration <= (~(ADI_DATA_MASK_SEED << length)));

    uint16_t command = (((uint16_t)configuredCommand[ADI_COMMAND_FIRST_BYTE_POSITION]) << ADI_BYTE_SHIFT) |
                       configuredCommand[ADI_COMMAND_SECOND_BYTE_POSITION];
    uint16_t mask = (~(ADI_COMMAND_MASK_SEED << length)) << position;
    /* Set bits to modify to 0 */
    command &= ~mask;
    /* Now set bits, works if 0 or not */
    command |= (configuration << position) & mask;
    configuredCommand[ADI_COMMAND_FIRST_BYTE_POSITION]  = (command >> ADI_BYTE_SHIFT) & ADI_ONE_BYTE_MASK;
    configuredCommand[ADI_COMMAND_SECOND_BYTE_POSITION] = command & ADI_ONE_BYTE_MASK;
}

extern STD_RETURN_TYPE_e ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_e registerSet, ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(registerSet <= ADI_CFG_REGISTER_SET_B);
    STD_RETURN_TYPE_e returnValueA = STD_OK;
    STD_RETURN_TYPE_e returnValueB = STD_OK;
    STD_RETURN_TYPE_e returnValue  = STD_OK;

    if (registerSet == ADI_CFG_REGISTER_SET_A) {
        ADI_WriteRegister(
            adi_cmdWrcfga,
            adi_configurationRegisterAgroup[adiState->currentString],
            ADI_PEC_NO_FAULT_INJECTION,
            adiState);
        ADI_CopyCommandBits(adi_cmdRdcfga, adi_command);
        ADI_ReadRegister(adi_command, adi_readConfigurationRegisterAgroup[adiState->currentString], adiState);
        returnValueA = ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_A, adiState);
    } else if (registerSet == ADI_CFG_REGISTER_SET_B) {
        ADI_WriteRegister(
            adi_cmdWrcfgb,
            adi_configurationRegisterBgroup[adiState->currentString],
            ADI_PEC_NO_FAULT_INJECTION,
            adiState);
        ADI_CopyCommandBits(adi_cmdRdcfgb, adi_command);
        ADI_ReadRegister(adi_command, adi_readConfigurationRegisterBgroup[adiState->currentString], adiState);
        returnValueB = ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_B, adiState);
    } else {                  /* invalid register set */
        FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
    }
    if ((returnValueA == STD_NOT_OK) || (returnValueB == STD_NOT_OK)) {
        returnValue = STD_NOT_OK;
    }
    return returnValue;
}

extern void ADI_CopyCommandBits(const uint16_t *sourceCommand, uint16_t *destinationCommand) {
    FAS_ASSERT(sourceCommand != NULL_PTR);
    FAS_ASSERT(destinationCommand != NULL_PTR);
    for (uint8_t i = 0u; i < ADI_COMMAND_DEFINITION_LENGTH; i++) {
        destinationCommand[i] = sourceCommand[i];
    }
}

extern void ADI_ReadDataBits(uint8_t receivedData, uint8_t *pDataToRead, uint8_t position, uint8_t mask) {
    /* AXIVION Routine Generic-MissingParameterAssert: receivedData: parameter accepts whole range */
    FAS_ASSERT(pDataToRead != NULL_PTR);
    /* Only 8 bit in a register part */
    FAS_ASSERT(position <= ADI_MAX_BIT_POSITION_IN_BYTE);
    /* All 0s mask would mean do nothing */
    FAS_ASSERT(mask > 0u);

    *pDataToRead = (receivedData & mask) >> position;
}

/* RequirementId: D7.1 V0R4 FUN-0.0.01.01 */
/* RequirementId: D7.1 V0R4 SIF-4.10.01.01 */
/* RequirementId: D7.1 V0R4 SIF-4.10.02.01 */
/* RequirementId: D7.1 V0R4 SIF-4.10.11.02 */
/* RequirementId: D7.1 V0R4 SIF-4.10.12.01 */
extern void ADI_ReadRegister(uint16_t *registerToRead, uint8_t *data, ADI_STATE_s *adiState) {
    FAS_ASSERT(registerToRead != NULL_PTR);
    FAS_ASSERT(data != NULL_PTR);
    FAS_ASSERT(adiState != NULL_PTR);

    uint8_t PEC_Check[ADI_SIZE_OF_DATA_FOR_PEC_COMPUTATION_WITH_COUNTER] = {0};
    uint16_t PEC_result                                                  = 0u;
    uint8_t PEC_RX[ADI_PEC_SIZE_IN_BYTES]                                = {0};
    uint8_t afeCommandCounter                                            = 0u;
    uint16_t registerLengthInBytes = registerToRead[ADI_COMMAND_DATA_LENGTH_POSITION];
    uint16_t byte                  = 0u; /* variable to parse data bytes */

    /* One frame = data + 2 bytes PEC */
    /* + 2u: the two additional bytes corresponding to the PEC */
    uint16_t spiFrameLength = registerLengthInBytes + ADI_PEC_SIZE_IN_BYTES;
    uint16_t dataLength     = registerLengthInBytes;

    for (uint16_t i = 0; i < ADI_N_BYTES_FOR_DATA_TRANSMISSION; i++) {
        adiState->data.txBuffer[i] = 0x0;
    }

    /**
     *  SM_SPI_PEC: SPI Packet Error Code
     *  Calculate PEC for command.
     */
    /* Compute PEC of the two command bytes */
    PEC_Check[ADI_COMMAND_FIRST_BYTE_POSITION]  = (uint8_t)registerToRead[ADI_COMMAND_FIRST_BYTE_POSITION];
    PEC_Check[ADI_COMMAND_SECOND_BYTE_POSITION] = (uint8_t)registerToRead[ADI_COMMAND_SECOND_BYTE_POSITION];
    PEC_result                                  = ADI_Pec15(ADI_COMMAND_SIZE_IN_BYTES, PEC_Check);

    adiState->data.txBuffer[ADI_COMMAND_FIRST_BYTE_POSITION]  = registerToRead[ADI_COMMAND_FIRST_BYTE_POSITION];
    adiState->data.txBuffer[ADI_COMMAND_SECOND_BYTE_POSITION] = registerToRead[ADI_COMMAND_SECOND_BYTE_POSITION];
    adiState->data.txBuffer[ADI_COMMAND_PEC_FIRST_BYTE_POSITION] =
        (uint8_t)((PEC_result >> ADI_BYTE_SHIFT) & ADI_ONE_BYTE_MASK);
    adiState->data.txBuffer[ADI_COMMAND_PEC_SECOND_BYTE_POSITION] = (uint8_t)(PEC_result & ADI_ONE_BYTE_MASK);

    /* 4u: two bytes command + two bytes command PEC */
    /* Register length + 2u: The two additional bytes correspond to the PEC */
    uint16_t frameLength = ADI_COMMAND_AND_PEC_SIZE_IN_BYTES +
                           ((registerLengthInBytes + ADI_PEC_SIZE_IN_BYTES) * ADI_N_ADI);
    ADI_SpiTransmitReceiveData(adiState, adiState->data.txBuffer, adiState->data.rxBuffer, (uint32_t)frameLength);

    for (uint16_t i = 0; i < ADI_N_ADI; i++) {
        /* Put received messages in data buffer, used outside function to access data */
        for (byte = 0u; byte < registerLengthInBytes; byte++) {
            data[byte + (i * dataLength)] =
                (uint8_t)(adiState->data.rxBuffer
                              [(ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME + byte) + (i * spiFrameLength)]);
        }
        /* PEC_Check is a local variable holding read data and used to compute the PEC */
        for (byte = 0u; byte < registerLengthInBytes; byte++) {
            PEC_Check[byte] =
                (uint8_t)(adiState->data.rxBuffer
                              [(ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME + byte) + (i * spiFrameLength)]);
        }
        /* Data PEC is also computed on command counter, so one byte with command counter is added */
        PEC_Check[byte] =
            (uint8_t)(adiState->data.rxBuffer
                          [(ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME + byte) + (i * spiFrameLength)]) &
            ADI_COMMAND_COUNTER_MASK;

        PEC_result =
            ADI_Pec10((uint8_t)(registerToRead[ADI_COMMAND_DATA_LENGTH_POSITION] & ADI_ONE_BYTE_MASK), PEC_Check, true);
        PEC_RX[ADI_DATA_PEC_FIRST_BYTE_POSITION]  = (uint8_t)((PEC_result >> ADI_BYTE_SHIFT) & ADI_ONE_BYTE_MASK);
        PEC_RX[ADI_DATA_PEC_SECOND_BYTE_POSITION] = (uint8_t)(PEC_result & ADI_ONE_BYTE_MASK);

        /* Position of the PEC (two bytes) in the first read frame */
        /* 4u: two bytes command + two bytes command PEC, followed by number of bytes in register */
        uint16_t crcByte0Position = registerLengthInBytes + ADI_COMMAND_AND_PEC_SIZE_IN_BYTES;
        uint16_t crcByte1Position = registerLengthInBytes + ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + 1u;

        /**
         *  SM_SPI_PEC: SPI Packet Error Code
         *  PEC check on read value.
         */
        /* if calculated PEC not equal to received PEC */
        if ((PEC_RX[ADI_DATA_PEC_FIRST_BYTE_POSITION] !=
             (adiState->data.rxBuffer[crcByte0Position + (i * spiFrameLength)] &
              ADI_PEC10_MSB_EXCLUDE_COMMAND_COUNTER)) ||
            (PEC_RX[ADI_DATA_PEC_SECOND_BYTE_POSITION] !=
             (adiState->data.rxBuffer[crcByte1Position + (i * spiFrameLength)] & ADI_ONE_BYTE_MASK))) {
/* update error table of the corresponding ades183x only if PEC check is activated */
#if (ADI_DISCARD_PEC == false)
            adiState->data.errorTable->crcIsOk[adiState->currentString][i] = false;
#else
            adiState->data.errorTable->crcIsOk[adiState->currentString][i] = true;
#endif
        } else {
            /* update error table of the corresponding ades183x */
            adiState->data.errorTable->crcIsOk[adiState->currentString][i] = true;
        }

        /* CRC is placed after the data bytes */
        uint16_t crcFirstBytePosition = ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME + registerLengthInBytes;
        /**
         *  SM_SPI_CNT: SPI Frame Counter
         *  Compare driver stored value of command counter with values sent by the AFEs.
         */
        /* Retrieve command counter value sent by the AFE, included in first CRC byte of answer frame */
        uint16_t commandCounterData = adiState->data.rxBuffer[crcFirstBytePosition + (i * spiFrameLength)] &
                                      ADI_COMMAND_COUNTER_MASK;
        afeCommandCounter = (uint8_t)((commandCounterData >> ADI_COMMAND_COUNTER_POSITION) & ADI_ONE_BYTE_MASK);

        if (afeCommandCounter != adiState->data.commandCounter[adiState->currentString][i]) {
            /**
             * command counter flag stays set as soon as an error was detected
             * it can only be reset within the reset function
             */
            adiState->data.errorTable->commandCounterIsOk[adiState->currentString][i] = false;
        }
    }
}

/* RequirementId: D7.1 V0R4 FUN-0.0.01.04 */
extern void ADI_SpiTransmitReceiveData(
    ADI_STATE_s *adiState,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    /* AXIVION Routine Generic-MissingParameterAssert: frameLength: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: pRxBuff: parameter accepts whole range */
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);

    STD_RETURN_TYPE_e retValDummy = STD_OK;
    STD_RETURN_TYPE_e retVal      = STD_OK;
    if (frameLength == 0u) {
        /* Length = 0: only transmit dummy byte */
        /* START SPI function to adapt for different environment */
        /* No DMA, blocking */
        retVal = SPI_TransmitDummyByte(&spi_adiInterface[adiState->currentString], 0u);
        /* END SPI function to adapt for different environment */
    } else {
        if (pRxBuff == NULL_PTR) {
            /* Transmit only (receive discarded) */
            /* START SPI function to adapt for different environment */
            /* No DMA, blocking */
            retValDummy =
                SPI_TransmitDummyByte(&spi_adiInterface[adiState->currentString], ADI_SPI_WAKEUP_WAIT_TIME_US);
            retVal = SPI_TransmitData(&spi_adiInterface[adiState->currentString], pTxBuff, frameLength);
            /* END SPI function to adapt for different environment */
        } else {
            /* Transmit and receive */
            /* START SPI function to adapt for different environment */
            /* Uses DMA and notifications */
            retValDummy =
                SPI_TransmitDummyByte(&spi_adiInterface[adiState->currentString], ADI_SPI_WAKEUP_WAIT_TIME_US);
            retVal =
                SPI_TransmitReceiveDataDma(&spi_adiInterface[adiState->currentString], pTxBuff, pRxBuff, frameLength);
            /* Now wait for notification in DMA Rx finished interrupt */
            uint32_t timeout = ADI_TRANSMISSION_TIMEOUT;
            FAS_ASSERT(timeout > 0u);
            uint32_t ulNotifiedValue = 0u;
            /* Suspend task and wait for notification, clear notification value on entry and exit  */
            (void)OS_WaitForNotification(&ulNotifiedValue, timeout);
            if (ulNotifiedValue != ADI_DMA_SPI_FINISHED_NOTIFICATION_VALUE) {
                /* if ulNotifiedValue not written by xTaskNotify() in DMA interrupt: transmission timeout */
                retVal = STD_NOT_OK;
            }
            /* END SPI function to adapt for different environment */
        }
    }

    if ((retVal == STD_NOT_OK) || (retValDummy == STD_NOT_OK)) {
        adiState->data.errorTable->spiIsOk[adiState->currentString] = false;
    } else if ((retVal == STD_OK) && (retValDummy == STD_OK)) {
        adiState->data.errorTable->spiIsOk[adiState->currentString] = true;
    } else {
        FAS_ASSERT(FAS_TRAP); /* LCOV_EXCL_LINE */
    }
}

extern void ADI_StoredConfigurationFillRegisterDataGlobal(
    ADI_CFG_REGISTER_SET_e registerSet,
    uint8_t registerOffset,
    uint8_t data,
    uint8_t position,
    uint8_t mask,
    ADI_STATE_s *adiState) {
    /* AXIVION Routine Generic-MissingParameterAssert: data: parameter accepts whole range */
    FAS_ASSERT(adiState != NULL_PTR);
    FAS_ASSERT(registerSet <= ADI_CFG_REGISTER_SET_B);
    /* Only 8 bit in a register part */
    FAS_ASSERT(position <= ADI_MAX_BIT_POSITION_IN_BYTE);
    /* All 0s mask would mean do nothing */
    FAS_ASSERT(mask > 0u);
    FAS_ASSERT(registerOffset <= ADI_REGISTER_OFFSET5);

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        ADI_StoredConfigurationFillRegisterData(m, registerSet, registerOffset, data, position, mask, adiState);
    }
}

/* RequirementId: D7.1 V0R4 FUN-0.0.01.03 */
/* RequirementId: D7.1 V0R4 SIF-4.10.01.01 */
/* RequirementId: D7.1 V0R4 SIF-4.10.11.01 */
extern void ADI_TransmitCommand(uint16_t *command, ADI_STATE_s *adiState) {
    FAS_ASSERT(command != NULL_PTR);
    FAS_ASSERT(adiState != NULL_PTR);

    uint8_t PEC_Check[ADI_MAX_REGISTER_SIZE_IN_BYTES] = {0};
    uint16_t PEC_result                               = 0u;

    /**
     *  SM_SPI_PEC: SPI Packet Error Code
     *  Calculate PEC for command.
     */
    PEC_Check[ADI_COMMAND_FIRST_BYTE_POSITION]  = (uint8_t)command[ADI_COMMAND_BYTE0_POSITION];
    PEC_Check[ADI_COMMAND_SECOND_BYTE_POSITION] = (uint8_t)command[ADI_COMMAND_BYTE1_POSITION];
    PEC_result                                  = ADI_Pec15(ADI_COMMAND_SIZE_IN_BYTES, PEC_Check);

    adiState->data.txBuffer[ADI_COMMAND_FIRST_BYTE_POSITION]  = command[ADI_COMMAND_BYTE0_POSITION];
    adiState->data.txBuffer[ADI_COMMAND_SECOND_BYTE_POSITION] = command[ADI_COMMAND_BYTE1_POSITION];
    adiState->data.txBuffer[ADI_COMMAND_PEC_FIRST_BYTE_POSITION] =
        (uint8_t)((PEC_result >> ADI_BYTE_SHIFT) & ADI_ONE_BYTE_MASK);
    adiState->data.txBuffer[ADI_COMMAND_PEC_SECOND_BYTE_POSITION] = (uint8_t)(PEC_result & ADI_ONE_BYTE_MASK);

    ADI_SpiTransmitReceiveData(adiState, adiState->data.txBuffer, NULL_PTR, ADI_COMMAND_AND_PEC_SIZE_IN_BYTES);

    /**
         *  SM_SPI_CNT: SPI Frame Counter
         *  Increment driver stored value of command counter if command causes increase.
         */
    /** If command increments AFE command counter, increment driver command counter */
    if (command[ADI_COMMAND_INC_POSITION] == 1u) {
        ADI_IncrementCommandCounter(adiState);
    }
}

extern void ADI_WriteRegisterGlobal(
    const uint16_t *registerToWrite,
    uint8_t *data,
    ADI_PEC_FAULT_INJECTION_e pecFaultInjection,
    ADI_STATE_s *adiState) {
    FAS_ASSERT(registerToWrite != NULL_PTR);
    FAS_ASSERT(data != NULL_PTR);
    FAS_ASSERT(adiState != NULL_PTR);

    for (uint16_t i = 0u; i < ADI_N_ADI; i++) {
        for (uint8_t byte = 0; byte < registerToWrite[ADI_COMMAND_DATA_LENGTH_POSITION]; byte++) {
            adi_dataTransmit[byte + (i * registerToWrite[ADI_COMMAND_DATA_LENGTH_POSITION])] = data[byte];
        }
    }
    ADI_WriteRegister(registerToWrite, adi_dataTransmit, pecFaultInjection, adiState);
}

/* RequirementId: D7.1 V0R4 SIF-4.20.01.03 */
extern void ADI_StoredConfigurationWriteToAfeGlobal(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);
    STD_RETURN_TYPE_e returnValue = STD_OK;

    returnValue = ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_A, adiState);
    if (returnValue == STD_NOT_OK) {
        /* If problem when writing configuration, retry once */
        (void)ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_A, adiState);
    }
    returnValue = ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_B, adiState);
    if (returnValue == STD_NOT_OK) {
        /* If problem when writing configuration, retry once */
        (void)ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_B, adiState);
    }
}

extern void ADI_ClearCommandCounter(ADI_STATE_s *adiState) {
    FAS_ASSERT(adiState != NULL_PTR);

    /**
     *  SM_SPI_CNT: SPI Frame Counter
     *  Reset driver stored value of command counter by sending RSTCC command.
     */
    /* Clear command counter */
    ADI_CopyCommandBits(adi_cmdRstcc, adi_command);
    ADI_TransmitCommand(adi_command, adiState);
    for (uint16_t i = 0u; i < ADI_N_ADI; i++) {
        adiState->data.errorTable->commandCounterIsOk[adiState->currentString][i] = true;
        adiState->data.commandCounter[adiState->currentString][i]                 = ADI_COMMAND_COUNTER_RESET_VALUE;
    }
}

extern void ADI_Wait(uint32_t waitTime) {
    FAS_ASSERT(waitTime > 0u);

    uint32_t currentTime = OS_GetTickCount();
    /* Block task without possibility to wake up */
    /* +1: to wait at least waitTime*/
    OS_DelayTaskUntil(&currentTime, waitTime + 1u);
}

extern void ADI_WriteDataBits(uint8_t *pSentData, uint8_t dataToWrite, uint8_t position, uint8_t mask) {
    FAS_ASSERT(pSentData != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: dataToWrite: parameter accepts whole range */
    /* Only 8 bit in a register part */
    FAS_ASSERT(position <= ADI_MAX_BIT_POSITION_IN_BYTE);
    /* All 0s mask would mean do nothing */
    FAS_ASSERT(mask > 0u);

    uint8_t tempData = (uint8_t)((((uint16_t)dataToWrite) << position) & ADI_DATA_MASK_SEED);
    *pSentData &= ~mask;
    *pSentData |= tempData & mask;
}

/* RequirementId: D7.1 V0R4 FUN-0.0.01.02 */
/* RequirementId: D7.1 V0R4 SIF-4.10.01.01 */
/* RequirementId: D7.1 V0R4 SIF-4.10.11.01 */
extern void ADI_WriteRegister(
    const uint16_t *registerToWrite,
    uint8_t *data,
    ADI_PEC_FAULT_INJECTION_e pecFaultInjection,
    ADI_STATE_s *adiState) {
    FAS_ASSERT(registerToWrite != NULL_PTR);
    FAS_ASSERT(data != NULL_PTR);
    FAS_ASSERT(adiState != NULL_PTR);

    uint16_t PEC_result                                     = 0;
    uint8_t PEC_Check[ADI_SIZE_OF_DATA_FOR_PEC_COMPUTATION] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t registerLengthInBytes                          = registerToWrite[ADI_COMMAND_DATA_LENGTH_POSITION];
    uint16_t spiFrameLength                                 = registerLengthInBytes + 2u;
    uint16_t dataLength                                     = registerLengthInBytes;

    /**
     *  SM_SPI_PEC: SPI Packet Error Code
     *  Calculate PEC for command.
     */
    /* Compute PEC of the two command bytes */
    PEC_Check[ADI_COMMAND_FIRST_BYTE_POSITION]  = (uint8_t)registerToWrite[ADI_COMMAND_FIRST_BYTE_POSITION];
    PEC_Check[ADI_COMMAND_SECOND_BYTE_POSITION] = (uint8_t)registerToWrite[ADI_COMMAND_SECOND_BYTE_POSITION];
    PEC_result                                  = ADI_Pec15(ADI_COMMAND_SIZE_IN_BYTES, PEC_Check);

    adiState->data.txBuffer[ADI_COMMAND_FIRST_BYTE_POSITION]  = registerToWrite[ADI_COMMAND_FIRST_BYTE_POSITION];
    adiState->data.txBuffer[ADI_COMMAND_SECOND_BYTE_POSITION] = registerToWrite[ADI_COMMAND_SECOND_BYTE_POSITION];
    if (pecFaultInjection == ADI_COMMAND_PEC_FAULT_INJECTION) {
        adiState->data.txBuffer[ADI_COMMAND_PEC_FIRST_BYTE_POSITION] = (PEC_result >> ADI_BYTE_SHIFT) &
                                                                       ADI_ONE_BYTE_MASK;
        adiState->data.txBuffer[ADI_COMMAND_PEC_SECOND_BYTE_POSITION] = (PEC_result & ADI_ONE_BYTE_MASK) + 1u;
    } else {
        adiState->data.txBuffer[ADI_COMMAND_PEC_FIRST_BYTE_POSITION] = (PEC_result >> ADI_BYTE_SHIFT) &
                                                                       ADI_ONE_BYTE_MASK;
        adiState->data.txBuffer[ADI_COMMAND_PEC_SECOND_BYTE_POSITION] = PEC_result & ADI_ONE_BYTE_MASK;
    }

    for (uint16_t i = 0u; i < ADI_N_ADI; i++) {
        /* data[] contains the data to write to the daisy-chain, it is used outside of the function */
        for (uint16_t byte = 0; byte < registerLengthInBytes; byte++) {
            adiState->data
                .txBuffer[(ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME + byte) + (i * spiFrameLength)] =
                data[byte + (i * dataLength)];
        }
        /**
         *  SM_SPI_PEC: SPI Packet Error Code
         *  Calculate PEC for values to be sent to the daisy-chain.
         */
        /* Calculate PEC of all data (1 PEC value for 6 bytes) */
        /* PEC_Check is a local variable holding data to write and used to compute the PEC */
        for (uint16_t byte = 0; byte < registerLengthInBytes; byte++) {
            uint16_t bufferData =
                adiState->data
                    .txBuffer[(ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME + byte) + (i * spiFrameLength)];
            PEC_Check[byte] = (uint8_t)((bufferData)&ADI_ONE_BYTE_MASK);
        }

        PEC_result = ADI_Pec10((uint8_t)(registerLengthInBytes & ADI_ONE_BYTE_MASK), PEC_Check, false);

        /* CRC is placed after the data bytes */
        uint16_t crcFirstBytePosition = ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME + registerLengthInBytes;
        if (pecFaultInjection == ADI_DATA_PEC_FAULT_INJECTION) {
            adiState->data.txBuffer[crcFirstBytePosition + (i * spiFrameLength)] =
                (PEC_result >> ADI_BYTE_SHIFT) & ADI_PEC10_FULL_EXCLUDE_COMMAND_COUNTER;
            adiState->data.txBuffer[crcFirstBytePosition + 1u + (i * spiFrameLength)] =
                (PEC_result & ADI_ONE_BYTE_MASK) + 1u;
        } else {
            adiState->data.txBuffer[crcFirstBytePosition + (i * spiFrameLength)] =
                (PEC_result >> ADI_BYTE_SHIFT) & ADI_PEC10_FULL_EXCLUDE_COMMAND_COUNTER;
            adiState->data.txBuffer[crcFirstBytePosition + 1u + (i * spiFrameLength)] = PEC_result & ADI_ONE_BYTE_MASK;
        }
    }

    uint16_t frameLength = ADI_COMMAND_AND_PEC_SIZE_IN_BYTES +
                           ((registerLengthInBytes + ADI_PEC_SIZE_IN_BYTES) * ADI_N_ADI);
    ADI_SpiTransmitReceiveData(adiState, adiState->data.txBuffer, adiState->data.rxBuffer, (uint32_t)frameLength);

    /**
     *  SM_SPI_CNT: SPI Frame Counter
     *  Increment driver stored value of command counter if command causes increase.
     */
    /* If command increments AFE command counter, increment driver command counter */
    /*  Do not increment if a PEC error was injected */
    if ((registerToWrite[ADI_COMMAND_INC_POSITION] == 1u) && (pecFaultInjection == ADI_PEC_NO_FAULT_INJECTION)) {
        ADI_IncrementCommandCounter(adiState);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ADI_StoredConfigurationFillRegisterData(
    uint8_t module,
    ADI_CFG_REGISTER_SET_e registerSet,
    uint8_t registerOffset,
    uint8_t data,
    uint8_t position,
    uint8_t mask,
    ADI_STATE_s *adiState) {
    ADI_StoredConfigurationFillRegisterData(module, registerSet, registerOffset, data, position, mask, adiState);
}
extern void TEST_ADI_IncrementCommandCounter(ADI_STATE_s *adiState) {
    ADI_IncrementCommandCounter(adiState);
}

#endif
