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
 * @file    adi_ades183x_mux.c
 * @author  foxBMS Team
 * @date    2026-02-06 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Implementations for the multiplexer driver of the ADI ADES18x family of
 *          analog front-ends.
 * @details Declares the high-level functions for the ADI ADES18x family driver
 *          The #ADI_ActivateInterfaceBoard function is specific to the
 *          hardware that communicates with the daisy-chain, i.e., for foxBMS 2
 *          it defines the setup of the port expander.
 */

/*========== Includes =======================================================*/
#include "adi_ades183x_mux.h"

#include "adi_ades183x.h"
#include "adi_ades183x_buffers.h"
#include "adi_ades183x_commands.h"
#include "adi_ades183x_helpers.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void ADI_IncrementMuxIndex(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);

    pAdiState->currentMux[pAdiState->currentString]++;
    if (pAdiState->currentMux[pAdiState->currentString] >= ADI_MUX_SEQUENCE_LENGTH) {
        pAdiState->currentMux[pAdiState->currentString] = 0u;
    }
    pAdiState->pMuxSequence[pAdiState->currentString] = adi_muxSequence +
                                                        pAdiState->currentMux[pAdiState->currentString];
}

extern void ADI_ResetAllMuxIndices(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pAdiState->currentMux[s]   = 0u;
        pAdiState->pMuxSequence[s] = adi_muxSequence;
    }
}

extern STD_RETURN_TYPE_e ADI_SetMuxChannel(ADI_STATE_s *pAdiState) {
    FAS_ASSERT(pAdiState != NULL_PTR);
    /* Magic number 4u is the the Maximum ID + 1*/
    FAS_ASSERT(pAdiState->pMuxSequence[pAdiState->currentString]->muxId < 4u);
    FAS_ASSERT(pAdiState->pMuxSequence[pAdiState->currentString]->muxChannel <= ADI_MUX_DISABLE_VALUE);

    uint8_t dataI2c          = 0u;
    uint8_t addressI2c_write = ADI_ADG728_ADDRESS_UPPER_BITS;
    uint8_t addressI2c_read  = ADI_ADG728_ADDRESS_UPPER_BITS;
    uint16_t readDataValue   = 0u;
    uint16_t readACKValue    = 0u;
    uint16_t tries           = 0u;
    STD_RETURN_TYPE_e retVal = STD_OK;

    uint8_t ifcomm0                                        = 0u;
    uint8_t d0                                             = 0u;
    uint8_t ifcomm1                                        = 0u;
    uint8_t d1                                             = 0u;
    uint8_t ifcomm2                                        = 0u;
    uint8_t d2                                             = 0u;
    uint8_t COMM_writeData[ADI_MAX_REGISTER_SIZE_IN_BYTES] = {ifcomm0, d0, ifcomm1, d1, ifcomm2, d2};
    /* Padding of 6 bytes needed for STCOMM command for transmitting 2 bytes of data */
    uint8_t paddingData[ADI_I2C_STCOMM_PADLEN];

    /* First set channel */

    /* Set bit1 and bit0 with mux address, write to mux */
    addressI2c_write |= ((pAdiState->pMuxSequence[pAdiState->currentString]->muxId) << 1u) | ADI_I2C_WRITE;
    /* Set bit1 and bit0 with mux address, read from mux */
    addressI2c_read |= ((pAdiState->pMuxSequence[pAdiState->currentString]->muxId) << 1u) | ADI_I2C_READ;

    /**
     * Set data to send, contains channel bit (8 channels)
     * 1 means channel active, 0 means channel inactive
     */
    if (pAdiState->pMuxSequence[pAdiState->currentString]->muxChannel == ADI_MUX_DISABLE_VALUE) {
        /* 0xFF in mux sequence means disable all channels */
        dataI2c = 0u;
    } else {
        dataI2c = (uint8_t)(1u << (pAdiState->pMuxSequence[pAdiState->currentString]->muxChannel));
    }

    /**
     * I2C protocoll:
     * Init Registers:
     * set ICOM0 to I2C; generate Start signal; no Controller ACK
     * send R/W and address bits as I2C data0
     * set ICOM1 to I2C; blank; generate Stop signal; no Controller ACK and stop
     * send mux data as I2C data1
     * set ICOM2 to I2C; no transmit
     * no data
     *
     * Sent command to Start communication
     *
     * Read if target has acknowledged data
     */

    /* I2C mux pin write setup; Only 2 bytes needed */
    ifcomm0 = ((ADI_ICOM_START & ADI_I2C_ICOM_MASK) << ADI_I2C_ICOM_OFFSET) | (ADI_I2C_FCOM_NO_ACK);
    d0      = addressI2c_write;
    ifcomm1 = ((ADI_ICOM_BLANK & ADI_I2C_ICOM_MASK) << ADI_I2C_ICOM_OFFSET) | (ADI_I2C_FCOM_NO_ACK_STOP);
    d1      = dataI2c;
    ifcomm2 = ((ADI_ICOM_NO_TRANSMIT & ADI_I2C_ICOM_MASK) << ADI_I2C_ICOM_OFFSET) | (ADI_I2C_FCOM_NO_ACK);
    d2      = 0u;

    COMM_writeData[ADI_REGISTER_OFFSET0] = ifcomm0;
    COMM_writeData[ADI_REGISTER_OFFSET1] = d0;
    COMM_writeData[ADI_REGISTER_OFFSET2] = ifcomm1;
    COMM_writeData[ADI_REGISTER_OFFSET3] = d1;
    COMM_writeData[ADI_REGISTER_OFFSET4] = ifcomm2;
    COMM_writeData[ADI_REGISTER_OFFSET5] = d2;

    /* Nulling is technically not needed but it is necessary for our unit tests */
    for (uint8_t i = 0; i < ADI_I2C_STCOMM_PADLEN; i++) {
        paddingData[i] = 0u;
    }

    /* Read Multiplexer to check if correct value was set */
    tries = ADI_I2C_ACK_TRIES;
    do {
        /* Write I2C data */
        ADI_WriteRegisterGlobal(adi_cmdWrcomm, COMM_writeData, ADI_PEC_NO_FAULT_INJECTION, pAdiState);
        /* Start I2C communication */
        ADI_WriteRegisterGlobal(adi_cmdStcomm, paddingData, ADI_PEC_NO_FAULT_INJECTION, pAdiState);

        /* Check for successful writes by reading the mux values */
        ADI_CopyCommandBytes(adi_cmdRdcomm, adi_command);
        ADI_ReadRegister(adi_command, adi_dataReceive, pAdiState);
        readACKValue  = adi_dataReceive[ADI_REGISTER_OFFSET0] & ADI_I2C_ICOM_MASK;
        readDataValue = adi_dataReceive[ADI_REGISTER_OFFSET3];
        /* Needed to read every answer on the Daisy Chain */
        for (uint8_t m = 1u; m < ADI_N_ADI; m++) {
            readACKValue &= adi_dataReceive[(m * ADI_MAX_REGISTER_SIZE_IN_BYTES) + ADI_REGISTER_OFFSET0] &
                            ADI_I2C_ICOM_MASK;
            readDataValue &= adi_dataReceive[(m * ADI_MAX_REGISTER_SIZE_IN_BYTES) + ADI_REGISTER_OFFSET3];
        }

        tries--;
        ADI_Wait(2u);
    } while ((readACKValue != ADI_I2C_FCOM_TARGET_ACK) && (tries > 0u));

    if ((tries > 0u) && (readDataValue == dataI2c)) {
        retVal = STD_OK;
    } else {
        retVal = STD_NOT_OK;
    }
    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
