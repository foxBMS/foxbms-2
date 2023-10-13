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
 * @file    adi_ades183x_helpers.h
 * @author  foxBMS Team
 * @date    2022-12-06 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Headers for the diagnostic driver for the ADI analog front-end.
 *
 */

#ifndef FOXBMS__ADI_ADES183X_HELPERS_H_
#define FOXBMS__ADI_ADES183X_HELPERS_H_

/*========== Includes =======================================================*/
/* clang-format off */
#include "adi_ades183x_cfg.h"
/* clang-format on */

#include "adi_ades183x_defs.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   send command to the ades183x daisy-chain (e.g., start voltage
 *          measurement).
 * @param   command     command to be sent
 * @param   adiState    state of the driver
 */
extern void ADI_TransmitCommand(uint16_t *command, ADI_STATE_s *adiState);

/**
 * @brief   copy command bits in variables.
 * @details The commands are defined as const. As some of them have
 *          configuration bits, they are first copied in a non-const variable.
 *          They can then be configured with
 *          ADI_WriteCommandConfigurationBits().
 * @param   sourceCommand       command definition from command list
 * @param   destinationCommand  copied command bits from command definition
 */
extern void ADI_CopyCommandBits(const uint16_t *sourceCommand, uint16_t *destinationCommand);

/**
 * @brief   write configuration bits in commands (e.g., channel in ADAX command).
 * @param   configuredCommand   command in which bits are configured
 * @param   position            start bit of configuration field to write
 * @param   length              length of configuration field to write
 * @param   configuration       configuration to write to the configuration field
 */
extern void ADI_WriteCommandConfigurationBits(
    uint16_t *configuredCommand,
    uint8_t position,
    uint8_t length,
    uint16_t configuration);

/**
 * @brief   Helper function to read specific bits from received data.
 * @param   receivedData    data received from the AFEs
 * @param   pDataToRead     specific bits read from received data
 * @param   position        position where to read the bits in received data
 * @param   mask            mask corresponding to bits to read
 */
extern void ADI_ReadDataBits(uint8_t receivedData, uint8_t *pDataToRead, uint8_t position, uint8_t mask);

/**
 * @brief   send command to the ades183x daisy-chain to read a register.
 * @details This is the core function to receive data from the ades183x
 *          daisy-chain.
 *          A 2 byte command is sent with the corresponding PEC.
 *          Example: Read configuration register (RDCFG).
 *          The data send is:
 *              2 bytes (COMMAND) 2 bytes (PEC)
 *          The data received is:
 *              6 bytes (IC1) 2 bytes (PEC) + 6 bytes (IC2) 2 bytes (PEC)
 *              + 6 bytes (IC3) 2 bytes (PEC)
 *              + ...
 *              + 6 bytes (IC{ADI_N_ADI}) 2 bytes (PEC)
 *          The function does not check the PECs. This has to be done
 *          elsewhere.
 * @param   registerToRead  register to be read
 * @param   data            read data
 * @param   adiState        state of the driver
 */
extern void ADI_ReadRegister(uint16_t *registerToRead, uint8_t *data, ADI_STATE_s *adiState);

/**
 * @brief   Sends data in the configuration stored in driver to all AFEs in the
 *          daisy chain, for all configuration registers.
 * @param   adiState          state of the driver
 */
extern void ADI_StoredConfigurationWriteToAfeGlobal(ADI_STATE_s *adiState);

/**
 * @brief   Compares the configuration stored in the AFE driver with
 *          the configuration read from the AFEs.
 * @details If the stored and read configuration match, the corresponding flag
 *          is set to true in the error table, otherwise it is set to false.
 * @param   registerSet configuration register to check
 * @param   adiState    state of the driver
 * @return  STD_OK if check was OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e ADI_CheckConfigurationRegister(ADI_CFG_REGISTER_SET_e registerSet, ADI_STATE_s *adiState);

/**
 * @brief   Sends data in the configuration stored in driver to all AFEs in the
 *          daisy chain, for a specific configuration register.
 * @param   registerSet configuration register to write
 * @param   adiState    state of the driver
 * @return  STD_OK if check made after writing was OK, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e ADI_StoredConfigurationWriteToAfe(ADI_CFG_REGISTER_SET_e registerSet, ADI_STATE_s *adiState);

/**
 * @brief   Write the same 6 bytes to all devices in the daisy-chain.
 * @param   registerToWrite     register to write
 * @param   data                data to write in register
 * @param   pecFaultInjection   used for diagnostics to inject faults in PEC
 * @param   adiState state of the driver
 */
extern void ADI_WriteRegisterGlobal(
    const uint16_t *registerToWrite,
    uint8_t *data,
    ADI_PEC_FAULT_INJECTION_e pecFaultInjection,
    ADI_STATE_s *adiState);

/**
 * @brief   Send RSTCC to AFEs in daisy-chain and clear command counter of AFE driver.
 * @param   adiState state of the driver
 */
extern void ADI_ClearCommandCounter(ADI_STATE_s *adiState);

/**
 * @brief   Transmits and receives data on SPI.
 * @param   adiState    state of the ADI driver
 * @param   pTxBuff pointer to data that is transmitted by the SPI interface
 * @param   pRxBuff pointer to data that is received by the SPI interface
 * @param   frameLength number of bytes to be transmitted by the SPI interface
 */
extern void ADI_SpiTransmitReceiveData(
    ADI_STATE_s *adiState,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);

/**
 * @brief   Writes data in the configuration stored in AFE driver,
 *          for all AFEs in the daisy-chain.
 * @details Does the same as ADI_StoredConfigurationFillRegisterData(), but
 *          for all AFEs in the daisy chain.
 * @param   registerSet     configuration register to address
 * @param   registerOffset  position of byte to address in configuration register
 * @param   data            data to write to byte in register
 * @param   position        position where to store the data in byte
 * @param   mask            mask corresponding to data to store
 * @param   adiState        state of the driver
 */
extern void ADI_StoredConfigurationFillRegisterDataGlobal(
    ADI_CFG_REGISTER_SET_e registerSet,
    uint8_t registerOffset,
    uint8_t data,
    uint8_t position,
    uint8_t mask,
    ADI_STATE_s *adiState);

/**
 * @brief   wait for a certain number of milliseconds
 * @details This function is used to wait for a definite amount of time.
 *          In foxBMS, block task running ades183x.
 *          The task cannot be woken up before the time has elapsed.
 * @param   waitTime time in ms during which task will be blocked
 */
extern void ADI_Wait(uint32_t waitTime);

/**
 * @brief   Helper function to write specific bits in data to be sent.
 * @param   pSentData   data to send to the AFEs
 * @param   dataToWrite specific bits to write in data to be sent
 * @param   position    position where to write the bits in data to send
 * @param   mask        mask corresponding to bits to write
 */
extern void ADI_WriteDataBits(uint8_t *pSentData, uint8_t dataToWrite, uint8_t position, uint8_t mask);

/**
 * @brief   sends command to the ades183x daisy-chain to write data.
 * @details This is the core function to transmit data to the ades183x
 *          daisy-chain.
 *          The data sent is:
 *              COMMAND + 6 bytes (IC1) + 6 bytes (IC2)
 *              + 6 bytes (IC3)
 *              + ...
 *              + 6 bytes (IC{ADI_N_ADI})
 *          A 2 byte command is sent with the corresponding PEC.
 *          Example: write configuration register (WRCFG).
 *          The command has to be set and then the function calculates the PEC
 *          automatically.
 *          The function calculates the needed PEC to send the data to the
 *          daisy-chain. The sent data has the format:
 *              2 byte-COMMAND (2 bytes PEC) + 6 bytes (IC1) (2 bytes PEC)
 *              + 6 bytes (IC2) (2 bytes PEC)
 *              + 6 bytes (IC3) (2 bytes PEC)
 *              + ...
 *              + 6 bytes (IC{ADI_N_ADI}) (2 bytes PEC)
 *          The pecFaultInjection parameters is used to inject faults in the
 *          command PEC or in the data PEC. It is used for diagnostics.
 *          The default value is ADI_PEC_NO_FAULT_INJECTION, in this case the
 *          correct PECs are transmitted.
 * @param   registerToWrite     register to write
 * @param   data                data to write in register
 * @param   adiState            state of the driver
 * @param   pecFaultInjection   used for diagnostics to inject faults in PEC
 */
extern void ADI_WriteRegister(
    const uint16_t *registerToWrite,
    uint8_t *data,
    ADI_PEC_FAULT_INJECTION_e pecFaultInjection,
    ADI_STATE_s *adiState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ADI_StoredConfigurationFillRegisterData(
    uint8_t module,
    ADI_CFG_REGISTER_SET_e registerSet,
    uint8_t registerOffset,
    uint8_t data,
    uint8_t position,
    uint8_t mask,
    ADI_STATE_s *adiState);
extern void TEST_ADI_IncrementCommandCounter(ADI_STATE_s *adiState);
#endif

#endif /* FOXBMS__ADI_ADES183X_HELPERS_H_ */
