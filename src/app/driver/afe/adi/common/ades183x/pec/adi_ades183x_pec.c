/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    adi_ades183x_pec.c
 * @author  foxBMS Team
 * @date    2019-12-12 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   PEC computations
 * @details The CRC polynomials used are defined in the data sheets of the ICs.
 */

/*========== Includes =======================================================*/
#include "adi_ades183x_pec.h"

#include "adi_ades183x.h"
#include "fassert.h"
#include "fstd_types.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern uint16_t ADI_Pec15(uint8_t length, uint8_t *data) {
    FAS_ASSERT(length > 0u);
    FAS_ASSERT(data != NULL_PTR);

    /* Pre-computed CRC15 Table for polynomial 0xC599 */
    static const uint16_t adi_crc15Table[ADI_PEC_PRECOMPUTED_TABLE_SIZE] = {
        0x0000u, 0x4599u, 0x4EABu, 0x0B32u, 0x58CFu, 0x1D56u, 0x1664u, 0x53FDu, 0x7407u, 0x319Eu, 0x3AACu, 0x7F35u,
        0x2CC8u, 0x6951u, 0x6263u, 0x27FAu, 0x2D97u, 0x680Eu, 0x633Cu, 0x26A5u, 0x7558u, 0x30C1u, 0x3BF3u, 0x7E6Au,
        0x5990u, 0x1C09u, 0x173Bu, 0x52A2u, 0x015Fu, 0x44C6u, 0x4FF4u, 0x0A6Du, 0x5B2Eu, 0x1EB7u, 0x1585u, 0x501Cu,
        0x03E1u, 0x4678u, 0x4D4Au, 0x08D3u, 0x2F29u, 0x6AB0u, 0x6182u, 0x241Bu, 0x77E6u, 0x327Fu, 0x394Du, 0x7CD4u,
        0x76B9u, 0x3320u, 0x3812u, 0x7D8Bu, 0x2E76u, 0x6BEFu, 0x60DDu, 0x2544u, 0x02BEu, 0x4727u, 0x4C15u, 0x098Cu,
        0x5A71u, 0x1FE8u, 0x14DAu, 0x5143u, 0x73C5u, 0x365Cu, 0x3D6Eu, 0x78F7u, 0x2B0Au, 0x6E93u, 0x65A1u, 0x2038u,
        0x07C2u, 0x425Bu, 0x4969u, 0x0CF0u, 0x5F0Du, 0x1A94u, 0x11A6u, 0x543Fu, 0x5E52u, 0x1BCBu, 0x10F9u, 0x5560u,
        0x069Du, 0x4304u, 0x4836u, 0x0DAFu, 0x2A55u, 0x6FCCu, 0x64FEu, 0x2167u, 0x729Au, 0x3703u, 0x3C31u, 0x79A8u,
        0x28EBu, 0x6D72u, 0x6640u, 0x23D9u, 0x7024u, 0x35BDu, 0x3E8Fu, 0x7B16u, 0x5CECu, 0x1975u, 0x1247u, 0x57DEu,
        0x0423u, 0x41BAu, 0x4A88u, 0x0F11u, 0x057Cu, 0x40E5u, 0x4BD7u, 0x0E4Eu, 0x5DB3u, 0x182Au, 0x1318u, 0x5681u,
        0x717Bu, 0x34E2u, 0x3FD0u, 0x7A49u, 0x29B4u, 0x6C2Du, 0x671Fu, 0x2286u, 0x2213u, 0x678Au, 0x6CB8u, 0x2921u,
        0x7ADCu, 0x3F45u, 0x3477u, 0x71EEu, 0x5614u, 0x138Du, 0x18BFu, 0x5D26u, 0x0EDBu, 0x4B42u, 0x4070u, 0x05E9u,
        0x0F84u, 0x4A1Du, 0x412Fu, 0x04B6u, 0x574Bu, 0x12D2u, 0x19E0u, 0x5C79u, 0x7B83u, 0x3E1Au, 0x3528u, 0x70B1u,
        0x234Cu, 0x66D5u, 0x6DE7u, 0x287Eu, 0x793Du, 0x3CA4u, 0x3796u, 0x720Fu, 0x21F2u, 0x646Bu, 0x6F59u, 0x2AC0u,
        0x0D3Au, 0x48A3u, 0x4391u, 0x0608u, 0x55F5u, 0x106Cu, 0x1B5Eu, 0x5EC7u, 0x54AAu, 0x1133u, 0x1A01u, 0x5F98u,
        0x0C65u, 0x49FCu, 0x42CEu, 0x0757u, 0x20ADu, 0x6534u, 0x6E06u, 0x2B9Fu, 0x7862u, 0x3DFBu, 0x36C9u, 0x7350u,
        0x51D6u, 0x144Fu, 0x1F7Du, 0x5AE4u, 0x0919u, 0x4C80u, 0x47B2u, 0x022Bu, 0x25D1u, 0x6048u, 0x6B7Au, 0x2EE3u,
        0x7D1Eu, 0x3887u, 0x33B5u, 0x762Cu, 0x7C41u, 0x39D8u, 0x32EAu, 0x7773u, 0x248Eu, 0x6117u, 0x6A25u, 0x2FBCu,
        0x0846u, 0x4DDFu, 0x46EDu, 0x0374u, 0x5089u, 0x1510u, 0x1E22u, 0x5BBBu, 0x0AF8u, 0x4F61u, 0x4453u, 0x01CAu,
        0x5237u, 0x17AEu, 0x1C9Cu, 0x5905u, 0x7EFFu, 0x3B66u, 0x3054u, 0x75CDu, 0x2630u, 0x63A9u, 0x689Bu, 0x2D02u,
        0x276Fu, 0x62F6u, 0x69C4u, 0x2C5Du, 0x7FA0u, 0x3A39u, 0x310Bu, 0x7492u, 0x5368u, 0x16F1u, 0x1DC3u, 0x585Au,
        0x0BA7u, 0x4E3Eu, 0x450Cu, 0x0095u,
    };

    uint16_t pec = ADI_PEC15_SEED;

    for (uint8_t byte = 0u; byte < length; byte++) {
        uint8_t position =
            (uint8_t)(((pec >> (ADI_PEC15_POLYNOMIAL_SIZE - ADI_PEC_BYTE_SIZE)) ^ data[byte]) & ADI_PEC_ONE_BYTE_MASK);
        pec = ((uint16_t)(pec << ADI_PEC_BYTE_SIZE)) ^ (uint16_t)(adi_crc15Table[position]);
    }

    /* Shift one bit to the left because in AFE, PEC is stored in 16 bit register with one trailing 0 */
    return ((pec & ADI_PEC15_MASK) << 1u);
}

uint16_t ADI_Pec10(uint8_t length, uint8_t *data, bool receive) {
    FAS_ASSERT(length > 0u);
    FAS_ASSERT(data != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: receive: 'receive' is guaranteed to be be bool (true/false)
       by the compiler  */

    /* Pre-computed CRC10 Table for polynomial 0x48F */
    static const uint16_t adi_crc10Table[ADI_PEC_PRECOMPUTED_TABLE_SIZE] = {
        0x000u, 0x08Fu, 0x11Eu, 0x191u, 0x23Cu, 0x2B3u, 0x322u, 0x3ADu, 0x0F7u, 0x078u, 0x1E9u, 0x166u, 0x2CBu, 0x244u,
        0x3D5u, 0x35Au, 0x1EEu, 0x161u, 0x0F0u, 0x07Fu, 0x3D2u, 0x35Du, 0x2CCu, 0x243u, 0x119u, 0x196u, 0x007u, 0x088u,
        0x325u, 0x3AAu, 0x23Bu, 0x2B4u, 0x3DCu, 0x353u, 0x2C2u, 0x24Du, 0x1E0u, 0x16Fu, 0x0FEu, 0x071u, 0x32Bu, 0x3A4u,
        0x235u, 0x2BAu, 0x117u, 0x198u, 0x009u, 0x086u, 0x232u, 0x2BDu, 0x32Cu, 0x3A3u, 0x00Eu, 0x081u, 0x110u, 0x19Fu,
        0x2C5u, 0x24Au, 0x3DBu, 0x354u, 0x0F9u, 0x076u, 0x1E7u, 0x168u, 0x337u, 0x3B8u, 0x229u, 0x2A6u, 0x10Bu, 0x184u,
        0x015u, 0x09Au, 0x3C0u, 0x34Fu, 0x2DEu, 0x251u, 0x1FCu, 0x173u, 0x0E2u, 0x06Du, 0x2D9u, 0x256u, 0x3C7u, 0x348u,
        0x0E5u, 0x06Au, 0x1FBu, 0x174u, 0x22Eu, 0x2A1u, 0x330u, 0x3BFu, 0x012u, 0x09Du, 0x10Cu, 0x183u, 0x0EBu, 0x064u,
        0x1F5u, 0x17Au, 0x2D7u, 0x258u, 0x3C9u, 0x346u, 0x01Cu, 0x093u, 0x102u, 0x18Du, 0x220u, 0x2AFu, 0x33Eu, 0x3B1u,
        0x105u, 0x18Au, 0x01Bu, 0x094u, 0x339u, 0x3B6u, 0x227u, 0x2A8u, 0x1F2u, 0x17Du, 0x0ECu, 0x063u, 0x3CEu, 0x341u,
        0x2D0u, 0x25Fu, 0x2E1u, 0x26Eu, 0x3FFu, 0x370u, 0x0DDu, 0x052u, 0x1C3u, 0x14Cu, 0x216u, 0x299u, 0x308u, 0x387u,
        0x02Au, 0x0A5u, 0x134u, 0x1BBu, 0x30Fu, 0x380u, 0x211u, 0x29Eu, 0x133u, 0x1BCu, 0x02Du, 0x0A2u, 0x3F8u, 0x377u,
        0x2E6u, 0x269u, 0x1C4u, 0x14Bu, 0x0DAu, 0x055u, 0x13Du, 0x1B2u, 0x023u, 0x0ACu, 0x301u, 0x38Eu, 0x21Fu, 0x290u,
        0x1CAu, 0x145u, 0x0D4u, 0x05Bu, 0x3F6u, 0x379u, 0x2E8u, 0x267u, 0x0D3u, 0x05Cu, 0x1CDu, 0x142u, 0x2EFu, 0x260u,
        0x3F1u, 0x37Eu, 0x024u, 0x0ABu, 0x13Au, 0x1B5u, 0x218u, 0x297u, 0x306u, 0x389u, 0x1D6u, 0x159u, 0x0C8u, 0x047u,
        0x3EAu, 0x365u, 0x2F4u, 0x27Bu, 0x121u, 0x1AEu, 0x03Fu, 0x0B0u, 0x31Du, 0x392u, 0x203u, 0x28Cu, 0x038u, 0x0B7u,
        0x126u, 0x1A9u, 0x204u, 0x28Bu, 0x31Au, 0x395u, 0x0CFu, 0x040u, 0x1D1u, 0x15Eu, 0x2F3u, 0x27Cu, 0x3EDu, 0x362u,
        0x20Au, 0x285u, 0x314u, 0x39Bu, 0x036u, 0x0B9u, 0x128u, 0x1A7u, 0x2FDu, 0x272u, 0x3E3u, 0x36Cu, 0x0C1u, 0x04Eu,
        0x1DFu, 0x150u, 0x3E4u, 0x36Bu, 0x2FAu, 0x275u, 0x1D8u, 0x157u, 0x0C6u, 0x049u, 0x313u, 0x39Cu, 0x20Du, 0x282u,
        0x12Fu, 0x1A0u, 0x031u, 0x0BEu,
    };

    uint16_t pec = ADI_PEC10_SEED;

    /* Compute CRC for the six bytes of data */
    for (uint8_t byte = 0u; byte < length; byte++) {
        uint8_t position =
            (uint8_t)(((pec >> (ADI_PEC10_POLYNOMIAL_SIZE - ADI_PEC_BYTE_SIZE)) ^ data[byte]) & ADI_PEC_ONE_BYTE_MASK);
        pec = ((uint16_t)(pec << ADI_PEC_BYTE_SIZE)) ^ (uint16_t)(adi_crc10Table[position]);
    }

    /* Add the 6 bits of command counter to CRC computation */

    /* In sent data, command counter bits are set to 0 */
    uint8_t commandCounter = 0u;
    if (receive == true) {
        /* In receive data, compute CRC with the 6 received command counter bits */
        commandCounter = data[length] & ADI_PEC10_COMMAND_COUNTER_MASK;
    }
    pec ^= (uint16_t)(commandCounter << (ADI_PEC10_POLYNOMIAL_SIZE - ADI_PEC_BYTE_SIZE));
    for (uint8_t i = 0u; i < ADI_PEC10_COMMAND_COUNTER_SIZE_IN_BITS; i++) {
        if ((pec & ADI_PEC10_MSB_MASK) != 0u) /* test for MSB = bit 10 */
        {
            pec = (uint16_t)((pec << 1u) ^ ADI_PEC10_POLYNOMIAL);
        } else {
            pec <<= 1u;
        }
    }
    return (pec & ADI_PEC10_MASK);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
