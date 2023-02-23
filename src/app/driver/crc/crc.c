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
 * @file    crc.c
 * @author  foxBMS Team
 * @date    2022-02-22 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  CRC
 *
 * @brief   crc module implementation
 *
 * @details Uses the system CRC hardware for data integrity calculation
 */

/*========== Includes =======================================================*/
#include "crc.h"

#include "fassert.h"
#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e CRC_CalculateCrc(uint64_t *pCrc, uint8_t *pData, uint32_t lengthInBytes) {
    static uint16_t crcCalls         = 0u;
    CRC_REGISTER_SIDE_e registerSide = CRC_REGISTER_LOW;
    uint32_t dataBufferLow           = 0u;
    uint32_t dataBufferHigh          = 0u;
    uint32_t remainingBytes          = lengthInBytes;
    uint32_t remainingData           = 0u;
    STD_RETURN_TYPE_e retVal         = STD_OK;

    FAS_ASSERT(pCrc != NULL_PTR);
    FAS_ASSERT(pData != NULL_PTR);

    uint8_t *pRead = pData;

    if (crcCalls == 0u) {
        crcCalls++;

        /* Set mode to Data Capture Mode, otherwise writing the seed
       starts the computation */
        crcREG1->CTRL2 &= CRC_DATA_CAPTURE_MODE_CLEAR_MASK;
        /* Set seed*/
        crcREG1->PSA_SIGREGH1 = CRC_SEED_HIGH;
        crcREG1->PSA_SIGREGL1 = CRC_SEED_LOW;
        /* Set mode to Full-CPU Mode to start the computation when writing the data*/
        crcREG1->CTRL2 |= CRC_FULL_CPU_MODE_SET_MASK;

        /* AXIVION Next Codeline Style MisraC2012-11.3: 64 bit access needed, partial 32 bit access starts computation */
        /* Pointer to access the two signature registers, where input data will be written */
        volatile uint64_t *pCrcRegister = (volatile uint64 *)(&crcREG1->PSA_SIGREGL1);

        /* Treat packets of 64 bit data */
        while (remainingBytes >= CRC_REGISTER_SIZE_IN_BYTES) {
            /* Invert two 32 bit chunks before 64 bit write, due to big endian */
            if (registerSide == CRC_REGISTER_LOW) {
                dataBufferLow = 0u;
                for (uint8_t i = 0u; i < CRC_REGISTER_SIZE_IN_BYTES; i++) {
                    uint8_t dataBuffer = *(pRead + i);
                    dataBufferLow |= ((uint32_t)dataBuffer) << ((CRC_REVERSE_BYTES_ORDER - i) * CRC_BYTE_SIZE_IN_BITS);
                }
                registerSide = CRC_REGISTER_HIGH;
            } else { /* registerSide is CRC_REGISTER_HIGH */
                dataBufferHigh = 0u;
                for (uint8_t i = 0u; i < CRC_REGISTER_SIZE_IN_BYTES; i++) {
                    uint8_t dataBuffer = *(pRead + i);
                    dataBufferHigh |= ((uint32_t)dataBuffer) << ((CRC_REVERSE_BYTES_ORDER - i) * CRC_BYTE_SIZE_IN_BITS);
                }
                /* Signature low and high available, write to hardware register */
                uint64_t crcData = (((uint64_t)dataBufferHigh) << CRC_REGISTER_SIZE_IN_BITS) | dataBufferLow;
                *pCrcRegister    = crcData;
                registerSide     = CRC_REGISTER_LOW;
            }
            pRead = (pRead + CRC_REGISTER_SIZE_IN_BYTES);
            remainingBytes -= CRC_REGISTER_SIZE_IN_BYTES;
        }

        if (remainingBytes > 0u) {
            /* Now treat last packet that is less than 32 bits if existing */
            /* Get data in a 32 bit variable, pad with 0 */
            while (remainingBytes > 0u) {
                uint8_t dataBuffer = *pRead;
                remainingData |= ((uint32_t)(dataBuffer)) << (CRC_BYTE_SIZE_IN_BITS * remainingBytes);
                pRead++;
                remainingBytes--;
            }
            if (registerSide == CRC_REGISTER_LOW) {
                dataBufferLow = remainingData;
                registerSide  = CRC_REGISTER_HIGH;
            } else { /* registerSide is CRC_REGISTER_HIGH */
                dataBufferHigh = remainingData;
                /* Signature low and high available, write to hardware register */
                uint64_t crcData = (((uint64_t)dataBufferHigh) << CRC_REGISTER_SIZE_IN_BITS) | dataBufferLow;
                *pCrcRegister    = crcData;
                registerSide     = CRC_REGISTER_LOW;
            }
        }

        /* No data remaining but only low register data available: compute CRC */
        if (registerSide == CRC_REGISTER_HIGH) {
            crcREG1->PSA_SIGREGL1 = dataBufferLow;
        }

        *pCrc = crcREG1->PSA_SIGREGL1;
        *pCrc |= ((uint64_t)crcREG1->PSA_SIGREGH1) << CRC_REGISTER_SIZE_IN_BITS;
        crcCalls--;
    } else {
        *pCrc  = 0u;
        retVal = STD_NOT_OK;
    }

    return retVal;
}
/*================== Static functions ======================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
