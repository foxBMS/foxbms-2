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
 * @file    can_helper.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2021-07-29 (date of last update)
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Helper functions for the CAN module
 *
 *
 */

/*========== Includes =======================================================*/
#include "can_helper.h"

#include "database.h"
#include "foxmath.h"

/*========== Macros and Definitions =========================================*/
/** Plausibility checking can signals lengths */
#define CAN_SIGNAL_MAX_SIZE (64u)

/** bitmask for extraction of one byte */
#define CAN_MESSAGE_BIT_MASK_ONE_BYTE (0xFFu)

/** length of one CAN byte in bit */
#define CAN_BYTE_LENGTH (8u)

/*========== Static Constant and Variable Definitions =======================*/

/** To convert big endian startbit to usual little endian representation (from 0 as LSB to 63 as MSB) */
static const uint8_t can_bigEndianTable[CAN_SIGNAL_MAX_SIZE] = {
    56u, 57u, 58u, 59u, 60u, 61u, 62u, 63u, 48u, 49u, 50u, 51u, 52u, 53u, 54u, 55u, 40u, 41u, 42u, 43u, 44u, 45u,
    46u, 47u, 32u, 33u, 34u, 35u, 36u, 37u, 38u, 39u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 16u, 17u, 18u, 19u,
    20u, 21u, 22u, 23u, 8u,  9u,  10u, 11u, 12u, 13u, 14u, 15u, 0u,  1u,  2u,  3u,  4u,  5u,  6u,  7u};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Convert bit start in big endian case.
 *          In the big endian case for CAN, the bit start is set
 *          to the MSB and the bit numbering is not directly usable.
 *          This functions converts the bit start to set it to
 *          the LSB of the signal and with the usual bit numbering.
 * @param   bitStart    bit start in big endian format
 * @param   bitLength   signal length
 * @return  bit start position converted to little endian format
 */
static uint64_t CAN_ConvertBitStartBigEndian(uint64_t bitStart, uint64_t bitLength);

/*========== Static Function Implementations ================================*/

static uint64_t CAN_ConvertBitStartBigEndian(uint64_t bitStart, uint64_t bitLength) {
    /* A valid message has to start before the end of the message */
    FAS_ASSERT(bitStart < CAN_SIGNAL_MAX_SIZE);
    /* The longest message may be CAN_SIGNAL_MAX_SIZE long */
    FAS_ASSERT(bitLength <= CAN_SIGNAL_MAX_SIZE);
    /* A signal must contain at least one bit */
    FAS_ASSERT(bitLength > 0u);

    /**
     * Example: big endian, bitStart = 53, bitLength = 13
     * For big endian, bitStart corresponds to MSB.
     * First convert |07 06 05 04 03 02 01 00| to |63 62 61 60 59 58 57 56|
     *               |15 14 13 12 11 10 09 08|    |55 54 53 52 51 50 49 48|
     *               |23 22 21 20 19 18 17 16|    |47 46 45 44 43 42 41 40|
     *               |31 30 29 28 27 26 25 24|    |39 38 37 36 35 34 33 32|
     *               |39 38 37 36 35 34 33 32|    |31 30 29 28 27 26 25 24|
     *               |47 46 45 44 43 42 41 40|    |23 22 21 20 19 18 17 16|
     *               |55 54 53 52 51 50 49 48|    |15 14 13 12 11 10 09 08|
     *               |63 62 61 60 59 58 57 56|    |07 06 05 04 03 02 01 00|
     * to get MSB position in the usual bit representation (from 0 as LSB to 63 as MSB).
     * In the example, 53 must be converted to 13.
     */
    uint64_t position = can_bigEndianTable[bitStart];

    /**
     * Usual bit position of MSB of signal is now available.
     * Now subtract signal length to get LSB position.
     * In the example, bitStart is converted from 53 to 0 for the length of 13 bits.
     * This corresponds to the usual data access in the little endian case, from bit 0 to bit 12,
     * where bitStart corresponds to LSB.
     */
    position = position - (bitLength - 1u);

    /* a valid message has to start before the end of the message */
    FAS_ASSERT(position < CAN_SIGNAL_MAX_SIZE);
    /* Check for a plausible message length (sum of start bit and length shall
       not be larger than 64, otherwise it will not fit into the message) */
    FAS_ASSERT((position + bitLength) <= CAN_SIGNAL_MAX_SIZE);

    return position;
}

/*========== Extern Function Implementations ================================*/

extern void CAN_TxPrepareSignalData(float *pSignal, CAN_SIGNAL_TYPE_s signalProperties) {
    FAS_ASSERT(pSignal != NULL_PTR);

    /* Check min/max limits */
    if (*pSignal > signalProperties.max) {
        *pSignal = signalProperties.max;
    } else if (*pSignal < signalProperties.min) {
        *pSignal = signalProperties.min;
    } else {
        ; /* no action on *pSignal required */
    }

    /* Apply offset */
    *pSignal = *pSignal + signalProperties.offset;

    /* Apply factor */
    *pSignal = *pSignal / signalProperties.factor;
}

extern void CAN_RxConvertRawSignalData(float *pSignalConverted, float signalRaw, CAN_SIGNAL_TYPE_s signalProperties) {
    FAS_ASSERT(pSignalConverted != NULL_PTR);
    /* Apply offset and factor */
    *pSignalConverted = (signalRaw * signalProperties.factor) - signalProperties.offset;
}

extern void CAN_TxSetMessageDataWithSignalData(
    uint64_t *pMessage,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t canSignal,
    CAN_ENDIANNESS_e endianness) {
    FAS_ASSERT(pMessage != NULL_PTR);
    FAS_ASSERT((endianness == CAN_BIG_ENDIAN) || (endianness == CAN_LITTLE_ENDIAN));
    /* The longest message may be CAN_SIGNAL_MAX_SIZE long */
    FAS_ASSERT(bitLength <= CAN_SIGNAL_MAX_SIZE);
    /* A signal must contain at least one bit */
    FAS_ASSERT(bitLength > 0u);

    uint64_t position = bitStart;

    if (endianness == CAN_BIG_ENDIAN) {
        position = CAN_ConvertBitStartBigEndian(bitStart, bitLength);
    }

    /* A valid message has to start before the end of the message */
    FAS_ASSERT(position < CAN_SIGNAL_MAX_SIZE);
    /* Check for a plausible message length (sum of start bit and length shall
       not be larger than 64, otherwise it will not fit into the message) */
    FAS_ASSERT((position + bitLength) <= CAN_SIGNAL_MAX_SIZE);

    /* Prepare a mask and assemble message */
    uint64_t mask = UINT64_MAX;

    if (bitLength == CAN_SIGNAL_MAX_SIZE) {
        /* Since a bit-shift of 64 on a 64bit integer is undefined, set to desired 0 */
        mask = 0u;
    } else {
        mask <<= bitLength;
    }
    mask = ~mask;

    *pMessage |= (canSignal & mask) << position;
}

extern void CAN_TxSetCanDataWithMessageData(uint64_t message, uint8_t *pCanData, CAN_ENDIANNESS_e endianness) {
    FAS_ASSERT(pCanData != NULL_PTR);
    /* Swap byte order if necessary */
    if (endianness == CAN_BIG_ENDIAN) {
        pCanData[CAN_BYTE_0_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_7_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_1_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_6_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_2_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_5_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_3_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_4_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_4_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_3_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_5_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_2_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_6_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_1_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_7_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_0_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
    } else if (endianness == CAN_LITTLE_ENDIAN) {
        pCanData[CAN_BYTE_0_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_0_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_1_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_1_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_2_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_2_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_3_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_3_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_4_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_4_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_5_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_5_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_6_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_6_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
        pCanData[CAN_BYTE_7_POSITION] =
            (uint8_t)(((message) >> (CAN_BYTE_7_POSITION * CAN_BYTE_LENGTH)) & CAN_MESSAGE_BIT_MASK_ONE_BYTE);
    } else {
        /* Endianness must be big or little */
        FAS_ASSERT(FAS_TRAP);
    }
}

extern void CAN_RxGetSignalDataFromMessageData(
    uint64_t message,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t *pCanSignal,
    CAN_ENDIANNESS_e endianness) {
    FAS_ASSERT(pCanSignal != NULL_PTR);
    FAS_ASSERT((endianness == CAN_BIG_ENDIAN) || (endianness == CAN_LITTLE_ENDIAN));
    /* The longest message may be CAN_SIGNAL_MAX_SIZE long */
    FAS_ASSERT(bitLength <= CAN_SIGNAL_MAX_SIZE);
    /* A signal must contain at least one bit */
    FAS_ASSERT(bitLength > 0u);
    /* Signal start can not be outside of message data */
    FAS_ASSERT(bitStart < CAN_SIGNAL_MAX_SIZE);
    uint64_t position = bitStart;

    if (endianness == CAN_BIG_ENDIAN) {
        position = CAN_ConvertBitStartBigEndian(bitStart, bitLength);
    }

    /* A valid message has to start before the end of the message */
    FAS_ASSERT(position < CAN_SIGNAL_MAX_SIZE);
    /* Check for a plausible message length (sum of start bit and length shall
       not be larger than 64, otherwise it will not fit into the message) */
    FAS_ASSERT((position + bitLength) <= CAN_SIGNAL_MAX_SIZE);

    /* Prepare a mask and assemble message */
    uint64_t mask = UINT64_MAX;

    if (bitLength == CAN_SIGNAL_MAX_SIZE) {
        /* Since a bit-shift of 64 on a 64bit integer is undefined, set to desired 0 */
        mask = 0u;
    } else {
        mask <<= bitLength;
    }
    mask = ~mask;

    *pCanSignal = (message >> position) & mask;
}

extern void CAN_RxGetMessageDataFromCanData(
    uint64_t *pMessage,
    const uint8_t *const kpkCanData,
    CAN_ENDIANNESS_e endianness) {
    FAS_ASSERT(pMessage != NULL_PTR);
    FAS_ASSERT(kpkCanData != NULL_PTR);
    /* Swap byte order if necessary */
    if (endianness == CAN_BIG_ENDIAN) {
        *pMessage = ((((uint64_t)kpkCanData[CAN_BYTE_0_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_7_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_1_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_6_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_2_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_5_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_3_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_4_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_4_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_3_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_5_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_2_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_6_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_1_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_7_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_0_POSITION * CAN_BYTE_LENGTH));
    } else if (endianness == CAN_LITTLE_ENDIAN) {
        *pMessage = ((((uint64_t)kpkCanData[CAN_BYTE_0_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_0_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_1_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_1_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_2_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_2_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_3_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_3_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_4_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_4_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_5_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_5_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_6_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_6_POSITION * CAN_BYTE_LENGTH)) |
                    ((((uint64_t)kpkCanData[CAN_BYTE_7_POSITION]) & CAN_MESSAGE_BIT_MASK_ONE_BYTE)
                     << (CAN_BYTE_7_POSITION * CAN_BYTE_LENGTH));
    } else {
        /* Endianness must be big or little */
        FAS_ASSERT(FAS_TRAP);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
