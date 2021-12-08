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
 * @file    mxm_crc8.c
 * @author  foxBMS Team
 * @date    2019-02-05 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   CRC8 calculation for Maxim Integrated Monitoring devices
 *
 * @details This module supports the calculation of a CRC8 based on the
 *          polynomial described in the Maxim data sheets.
 *          The polynomial is 0xA6.
 *
 */

/*========== Includes =======================================================*/
#include "mxm_crc8.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Compute CRC8 (0xA6) with initial value.
 * @details Computes the CRC8 for the given data with the polynomial 0xA6.
 * @param[in]   pData   array-pointer with data to be cyclic redundancy checked
 * @param[in]   lenData length of array
 * @param[in]   crcInit initial CRC-value
 * @return      uint8_t containing the computed CRC
 */
static uint8_t MXM_CRC8WithInitValue(uint16_t *pData, int32_t lenData, uint8_t crcInit);

/*========== Static Function Implementations ================================*/
static uint8_t MXM_CRC8WithInitValue(uint16_t *pData, int32_t lenData, uint8_t crcInit) {
    FAS_ASSERT(pData != NULL_PTR);

    /**
     * @brief   Precomputed CRC8-table for polynomial 0xA6
     * @details This array contains the precomputed results for the
     *          look-up-table-based computation of a CRC8 with polynomial 0xA6.
     *          The polynomial-number represents the coefficients of the polynomial
     *          like this:
     *          8  7  6  5  4  3  2  1  0
     *          1  0  1  0  0  1  1  0  1
     *          --> 14D
     *
     *          This polynomial can also be
     *          called 0x14D according to the notation (0xA6 << 1).
     */
    const uint8_t mxm_crc8Table[256] = {
        0x00u, 0x3Eu, 0x7Cu, 0x42u, 0xF8u, 0xC6u, 0x84u, 0xBAu, 0x95u, 0xABu, 0xE9u, 0xD7u, 0x6Du, 0x53u, 0x11u, 0x2Fu,
        0x4Fu, 0x71u, 0x33u, 0x0Du, 0xB7u, 0x89u, 0xCBu, 0xF5u, 0xDAu, 0xE4u, 0xA6u, 0x98u, 0x22u, 0x1Cu, 0x5Eu, 0x60u,
        0x9Eu, 0xA0u, 0xE2u, 0xDCu, 0x66u, 0x58u, 0x1Au, 0x24u, 0x0Bu, 0x35u, 0x77u, 0x49u, 0xF3u, 0xCDu, 0x8Fu, 0xB1u,
        0xD1u, 0xEFu, 0xADu, 0x93u, 0x29u, 0x17u, 0x55u, 0x6Bu, 0x44u, 0x7Au, 0x38u, 0x06u, 0xBCu, 0x82u, 0xC0u, 0xFEu,
        0x59u, 0x67u, 0x25u, 0x1Bu, 0xA1u, 0x9Fu, 0xDDu, 0xE3u, 0xCCu, 0xF2u, 0xB0u, 0x8Eu, 0x34u, 0x0Au, 0x48u, 0x76u,
        0x16u, 0x28u, 0x6Au, 0x54u, 0xEEu, 0xD0u, 0x92u, 0xACu, 0x83u, 0xBDu, 0xFFu, 0xC1u, 0x7Bu, 0x45u, 0x07u, 0x39u,
        0xC7u, 0xF9u, 0xBBu, 0x85u, 0x3Fu, 0x01u, 0x43u, 0x7Du, 0x52u, 0x6Cu, 0x2Eu, 0x10u, 0xAAu, 0x94u, 0xD6u, 0xE8u,
        0x88u, 0xB6u, 0xF4u, 0xCAu, 0x70u, 0x4Eu, 0x0Cu, 0x32u, 0x1Du, 0x23u, 0x61u, 0x5Fu, 0xE5u, 0xDBu, 0x99u, 0xA7u,
        0xB2u, 0x8Cu, 0xCEu, 0xF0u, 0x4Au, 0x74u, 0x36u, 0x08u, 0x27u, 0x19u, 0x5Bu, 0x65u, 0xDFu, 0xE1u, 0xA3u, 0x9Du,
        0xFDu, 0xC3u, 0x81u, 0xBFu, 0x05u, 0x3Bu, 0x79u, 0x47u, 0x68u, 0x56u, 0x14u, 0x2Au, 0x90u, 0xAEu, 0xECu, 0xD2u,
        0x2Cu, 0x12u, 0x50u, 0x6Eu, 0xD4u, 0xEAu, 0xA8u, 0x96u, 0xB9u, 0x87u, 0xC5u, 0xFBu, 0x41u, 0x7Fu, 0x3Du, 0x03u,
        0x63u, 0x5Du, 0x1Fu, 0x21u, 0x9Bu, 0xA5u, 0xE7u, 0xD9u, 0xF6u, 0xC8u, 0x8Au, 0xB4u, 0x0Eu, 0x30u, 0x72u, 0x4Cu,
        0xEBu, 0xD5u, 0x97u, 0xA9u, 0x13u, 0x2Du, 0x6Fu, 0x51u, 0x7Eu, 0x40u, 0x02u, 0x3Cu, 0x86u, 0xB8u, 0xFAu, 0xC4u,
        0xA4u, 0x9Au, 0xD8u, 0xE6u, 0x5Cu, 0x62u, 0x20u, 0x1Eu, 0x31u, 0x0Fu, 0x4Du, 0x73u, 0xC9u, 0xF7u, 0xB5u, 0x8Bu,
        0x75u, 0x4Bu, 0x09u, 0x37u, 0x8Du, 0xB3u, 0xF1u, 0xCFu, 0xE0u, 0xDEu, 0x9Cu, 0xA2u, 0x18u, 0x26u, 0x64u, 0x5Au,
        0x3Au, 0x04u, 0x46u, 0x78u, 0xC2u, 0xFCu, 0xBEu, 0x80u, 0xAFu, 0x91u, 0xD3u, 0xEDu, 0x57u, 0x69u, 0x2Bu, 0x15u,
    };

    uint8_t crc    = crcInit;
    int32_t len    = lenData;
    uint16_t *data = pData;
    while (len > 0) {
        /* The lookup table on this algorithm is not intended for values larger
        than uint8_t, SPI transmissions have size uint16_t due to the HAL */
        FAS_ASSERT(*data <= (uint8_t)UINT8_MAX);
        crc = mxm_crc8Table[*data ^ crc];
        data++;
        len--;
    }
    return crc;
}

/*========== Extern Function Implementations ================================*/

extern uint8_t MXM_CRC8(uint16_t *pData, int32_t lenData) {
    FAS_ASSERT(pData != NULL_PTR);
    return MXM_CRC8WithInitValue(pData, lenData, 0);
}

extern STD_RETURN_TYPE_e must_check_return MXM_CRC8SelfTest(void) {
    /* AXIVION Disable Style Generic-NoMagicNumbers: This test function uses magic numbers to test predefined values. */
    uint16_t testSequence1[4] = {0x02u, 0x12u, 0xB1u, 0xB2u};
    FAS_ASSERT(MXM_CRC8(testSequence1, 4) == 0xC4u);

    uint16_t testSequence2[3] = {0x03u, 0x12u, 0x00u};
    FAS_ASSERT(MXM_CRC8(testSequence2, 3) == 0xCBu);

    uint16_t testSequence3[10] = {0x02u, 0x5Bu, 0x12u, 0x42u, 0xFFu, 0xD3u, 0x13u, 0x77u, 0xA1u, 0x31u};
    FAS_ASSERT(MXM_CRC8(testSequence3, 10) == 0x7Eu);

    uint16_t testSequence4[3] = {0x03u, 0x66u, 0x00u};
    FAS_ASSERT(MXM_CRC8(testSequence4, 3) == 0x43u);

    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    return STD_OK;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
