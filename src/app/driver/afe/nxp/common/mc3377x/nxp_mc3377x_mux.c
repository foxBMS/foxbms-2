/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    nxp_mc3377x_mux.c
 * @author  foxBMS Team
 * @date    2025-02-21 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Multiplexer functions of the MC3377X analog front-end driver.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc3377x_mux.h"
/* clang-format off */
#include "nxp_mc3377x-ll.h"
/* clang-format on */
#include "nxp_mc3377x.h"
#include "nxp_mc3377x_helpers.h"
#include "nxp_mc3377x_reg_def.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void N77x_IncrementMuxIndex(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentMux[pState->currentString]++;
    if (pState->currentMux[pState->currentString] >= N77X_MUX_SEQUENCE_LENGTH) {
        pState->currentMux[pState->currentString] = 0u;
    }
    pState->pMuxSequence[pState->currentString] = pState->pMuxSequenceStart[pState->currentString] +
                                                  pState->currentMux[pState->currentString];
}

extern void N77x_ResetMuxIndex(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    pState->currentMux[pState->currentString]   = 0u;
    pState->pMuxSequence[pState->currentString] = pState->pMuxSequenceStart[pState->currentString];
}

extern STD_RETURN_TYPE_e N77x_SetMuxChannel(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    FAS_ASSERT(pState->pMuxSequence[pState->currentString]->muxId < 4u);
    FAS_ASSERT(pState->pMuxSequence[pState->currentString]->muxChannel <= 0xFFu);

    uint16_t readValue                        = 0u;
    uint8_t dataI2c                           = 0u;
    uint8_t addressI2c_write                  = N77X_ADG728_ADDRESS_UPPER_BITS;
    uint8_t addressI2c_read                   = N77X_ADG728_ADDRESS_UPPER_BITS;
    uint16_t tries                            = 0u;
    STD_RETURN_TYPE_e retVal                  = STD_OK;
    N77X_COMMUNICATION_STATUS_e returnedValue = N77X_COMMUNICATION_OK;

    /* First set channel */

    /* Set bit1 and bit0 with mux address, write to mux */
    addressI2c_write |= ((pState->pMuxSequence[pState->currentString]->muxId) << 1u) | N77X_I2C_WRITE;
    /* Set bit1 and bit0 with mux address, read from mux */
    addressI2c_read |= ((pState->pMuxSequence[pState->currentString]->muxId) << 1u) | N77X_I2C_READ;

    /**
     * Set data to send, contains channel bit (8 channels)
     * 1 means channel active, 0 means channel inactive
     */
    if (pState->pMuxSequence[pState->currentString]->muxChannel == 0xFF) {
        /* 0xFF in mux sequence means disable all channels */
        dataI2c = 0u;
    } else {
        dataI2c = (uint8_t)(1u << (pState->pMuxSequence[pState->currentString]->muxChannel));
    }

    /* Write data to send on I2C bus in registers */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_I2C_DATA0_OFFSET,
        (addressI2c_write << MC3377X_I2C_DATA0_BYTE0_POS) | (dataI2c << MC3377X_I2C_DATA0_BYTE1_POS),
        pState->pSpiTxSequence);

    /* Read with a repeated start directly after write */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_I2C_DATA1_OFFSET,
        (addressI2c_read << MC3377X_I2C_DATA1_BYTE2_POS) | (N77X_I2C_DUMMY_BYTE << MC3377X_I2C_DATA1_BYTE3_POS),
        pState->pSpiTxSequence);

    /* Write into the control register to start transaction */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_I2C_CTRL_OFFSET,
        (N77X_I2C_NR_BYTES_FOR_MUX_WRITE << MC3377X_I2C_CTRL_START_POS) |
            ((MC3377X_I2C_CTRL_STPAFTER_STOP_ENUM_VAL << MC3377X_I2C_CTRL_STPAFTER_POS) +
             (N77X_I2C_NR_BYTES_TO_SWITCH_TO_READ_FOR_UX_READ << MC3377X_I2C_CTRL_RDAFTER_POS)),
        pState->pSpiTxSequence);

    /**
     * Wait util transaction ends, test on last device in daisy-chain
     * So device address = number of modules
     */
    tries = N77X_FLAG_READY_TRIES;
    do {
        returnedValue =
            N77x_CommunicationRead(BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_STAT_OFFSET, &readValue, pState);
        tries--;
        N77x_Wait(2u);
    } while ((readValue & MC3377X_I2C_STAT_PENDING_MSK) && (returnedValue == N77X_COMMUNICATION_OK) && (tries > 0u));

    if ((returnedValue == N77X_COMMUNICATION_OK) && (tries > 0u)) {
        /**
         *  Get I2C read data, on last device in daisy-chain
         *  Use result to set error state for all slaves to avoid
         *  reading all slaves in daisy-chain.
         */
        returnedValue =
            N77x_CommunicationRead(BS_NR_OF_MODULES_PER_STRING, MC3377X_I2C_DATA1_OFFSET, &readValue, pState);
        readValue = readValue >> MC3377X_I2C_DATA1_BYTE3_POS;

        if (returnedValue == N77X_COMMUNICATION_OK) {
            if (N77X_CHECK_MUX_STATE == true) {
                if (readValue == dataI2c) {
                    /* OK */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        pState->n77xData.errorTable->mux0IsOk[pState->currentString][m] = true;
                    }
                } else {
                    /* Not OK */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        pState->n77xData.errorTable->mux0IsOk[pState->currentString][m] = false;
                    }
                }
            } else {
                retVal = STD_NOT_OK;
            }
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
