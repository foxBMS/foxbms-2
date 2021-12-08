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
 * @file    mxm_1785x_tools.c
 * @author  foxBMS Team
 * @date    2020-07-15 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   This is a collection of helper functions for the MAX1785x ICs
 *
 * @details This collection of helper functions for the MAX1785x ICs helps to
 *          calculate the lsb and msb for register values and similar tasks.
 *
 */

/*========== Includes =======================================================*/
#include "mxm_1785x_tools.h"

#include "mxm_register_map.h"

/*========== Macros and Definitions =========================================*/
/** length of a byte */
#define MXM_TOOLS_LENGTH_BYTE (8u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Find Position of first set bit in bitmask
 * @details Searches a bitmask starting from the lowest bit and returns the
 *          position of the first set bit.
 * @param[in]   bitmask     bitmask that should be searched
 * @return      position of first set bit
 */
static uint8_t MXM_FirstSetBit(uint16_t bitmask);

/*========== Static Function Implementations ================================*/
static uint8_t MXM_FirstSetBit(uint16_t bitmask) {
    uint8_t retval = 0;
    while (((bitmask >> retval) & 1u) == 0u) {
        retval++;
        /* exit if every entry is zero */
        if (retval >= 16u) {
            break;
        }
    }
    return retval;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e must_check_return MXM_FirstSetBitTest(void) {
    /* AXIVION Disable Style Generic-NoMagicNumbers: This test function uses magic numbers to test predefined values. */

    /* bitmasks containing only zeros should return first bit set 16 */
    FAS_ASSERT(MXM_FirstSetBit(MXM_BM_NULL) == 16u);

    FAS_ASSERT(MXM_FirstSetBit(MXM_BM_LSB) == 0u);

    FAS_ASSERT(MXM_FirstSetBit(MXM_BM_MSB) == 8u);

    FAS_ASSERT(MXM_FirstSetBit(MXM_REG_VERSION_MOD) == 4u);

    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    return STD_OK;
}

extern void MXM_Convert(
    uint8_t lsb,
    uint8_t msb,
    uint16_t *pTarget,
    MXM_CONVERSION_TYPE_e convType,
    uint32_t fullScaleReference_mV) {
    FAS_ASSERT(pTarget != NULL_PTR);
    uint16_t temporaryVoltage = 0;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_ADC_14BIT_VALUE, &temporaryVoltage);

    temporaryVoltage                        = temporaryVoltage + (uint16_t)1u;
    const uint32_t scaledVoltageUnipolar_mV = (temporaryVoltage * fullScaleReference_mV) / 0x3FFFu;

    switch (convType) {
        case MXM_CONVERSION_BIPOLAR:
            /* not yet supported */
            FAS_ASSERT(FAS_TRAP);
            break;
        case MXM_CONVERSION_BLOCK_VOLTAGE:
        case MXM_CONVERSION_UNIPOLAR:
            if (scaledVoltageUnipolar_mV > (uint16_t)UINT16_MAX) {
                *pTarget = UINT16_MAX;
            } else {
                *pTarget = (uint16_t)scaledVoltageUnipolar_mV;
            }
            break;
        default:
            /* we should not be here */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

extern STD_RETURN_TYPE_e must_check_return MXM_ConvertTest(void) {
    /* AXIVION Disable Style Generic-NoMagicNumbers: This test function uses magic numbers to test predefined values. */

    MXM_CONVERSION_TYPE_e conversionType = MXM_CONVERSION_UNIPOLAR;

    /* low scale */
    uint8_t msb      = 0x00u;
    uint8_t lsb      = 0x00u;
    uint16_t voltage = 1;
    MXM_Convert(lsb, msb, &voltage, conversionType, 5000);
    FAS_ASSERT(voltage == 0u);

    /* half scale */
    msb     = 0x80u;
    lsb     = 0x00u;
    voltage = 0;
    MXM_Convert(lsb, msb, &voltage, conversionType, 5000);
    FAS_ASSERT(voltage == 2500u);

    /* full scale */
    msb     = 0xFFu;
    lsb     = 0xFCu;
    voltage = 0;
    MXM_Convert(lsb, msb, &voltage, conversionType, 5000);
    FAS_ASSERT(voltage == 5000u);

    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    return STD_OK;
}

extern void MXM_ExtractValueFromRegister(uint8_t lsb, uint8_t msb, MXM_REG_BM bitmask, uint16_t *pValue) {
    /* input sanitation */
    FAS_ASSERT(pValue != NULL_PTR);

    /* find lowest bit that is 1 in bitmask */
    uint8_t start = MXM_FirstSetBit(bitmask);

    /* apply bitmask to MSB */
    uint16_t msbBitmask = (bitmask & MXM_BM_MSB);
    uint8_t msbMasked   = msb & ((uint8_t)(msbBitmask >> MXM_TOOLS_LENGTH_BYTE));

    /* shift LSB into right position if lsb is used */
    if (start < MXM_TOOLS_LENGTH_BYTE) {
        /* apply bitmask to LSB */
        uint8_t lsbMasked = lsb & ((uint8_t)(bitmask & MXM_BM_LSB));
        *pValue           = ((uint16_t)lsbMasked >> start);
        /* add MSB at right position */
        *pValue = (((uint16_t)msbMasked << (MXM_TOOLS_LENGTH_BYTE - start)) | *pValue);
    } else if (start == MXM_TOOLS_LENGTH_BYTE) {
        *pValue = (uint16_t)msbMasked;
    } else {
        *pValue = ((uint16_t)msbMasked >> (start - MXM_TOOLS_LENGTH_BYTE));
    }
}

extern STD_RETURN_TYPE_e must_check_return MXM_ExtractValueFromRegisterTest(void) {
    /* AXIVION Disable Style Generic-NoMagicNumbers: This test function uses magic numbers to test predefined values. */
    uint8_t lsb    = 0x31u;
    uint8_t msb    = 0x85u;
    uint16_t value = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_VERSION_MOD, &value);
    FAS_ASSERT(value == 0x853u);

    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_VERSION_VER, &value);
    FAS_ASSERT(value == 0x01u);

    lsb   = 0xFCu;
    msb   = 0xFFu;
    value = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_ADC_14BIT_VALUE, &value);
    FAS_ASSERT(value == 0x3FFFu);

    lsb   = 0xFEu;
    msb   = 0x7Fu;
    value = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_ADC_14BIT_VALUE, &value);
    FAS_ASSERT(value == 0x1FFFu);

    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    return STD_OK;
}

extern void MXM_Unipolar14BitInto16Bit(uint16_t inputValue, uint8_t *lsb, uint8_t *msb) {
    FAS_ASSERT(lsb != NULL_PTR);
    FAS_ASSERT(msb != NULL_PTR);
    uint16_t workingCopy = inputValue;
    /* left shift into 16bit position */
    workingCopy = workingCopy << 2u;

    /* bitmask LSB */
    *lsb = (uint8_t)(workingCopy & MXM_BM_LSB);
    /* shift MSB into lower byte (workingCopy is 16bit) */
    *msb = (uint8_t)(workingCopy >> MXM_TOOLS_LENGTH_BYTE);
}

extern uint16_t MXM_VoltageIntoUnipolar14Bit(uint16_t voltage_mV, uint16_t fullscaleReference_mV) {
    uint32_t temporaryVoltage = voltage_mV;
    /* multiply by the 14bit fullscale */
    temporaryVoltage = temporaryVoltage * 0x3FFFu;
    /* return divided by fullscale_reference */
    return (uint16_t)(temporaryVoltage / fullscaleReference_mV);
}

extern void MXM_ConvertModuleToString(
    const uint16_t moduleNumber,
    uint8_t *pStringNumber,
    uint16_t *pModuleNumberInString) {
    FAS_ASSERT(pStringNumber != NULL_PTR);
    FAS_ASSERT(pModuleNumberInString != NULL_PTR);
    /* the module number cannot be higher than the highest module in the daisy-chain */
    FAS_ASSERT(moduleNumber < MXM_MAXIMUM_NR_OF_MODULES);
    /* the module number cannot be higher than number of maximum modules in the string */
    FAS_ASSERT(moduleNumber < (BS_NR_OF_STRINGS * BS_NR_OF_MODULES));

    /* calculate string number */
    *pStringNumber = (uint8_t)(moduleNumber / BS_NR_OF_MODULES);
    FAS_ASSERT(*pStringNumber <= BS_NR_OF_STRINGS);
    /* calculate module number and handle edge-case BS_NR_OF_MODULES == 1 */
    if (1u == BS_NR_OF_MODULES) {
        *pModuleNumberInString = 0u;
    } else {
        *pModuleNumberInString = moduleNumber % BS_NR_OF_MODULES;
    }
    FAS_ASSERT(*pModuleNumberInString <= BS_NR_OF_MODULES);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
