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
 * @file    test_mxm_1785x_tools.c
 * @author  foxBMS Team
 * @date    2020-07-15 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  MXM
 *
 * @brief   Test for the Maxim 1785x driver tools.
 *
 * @details def
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockfassert.h"
#include "Mockmxm_cfg.h"

#include "mxm_1785x_tools.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testMXM_ExtractValueFromRegisterTest(void) {
    TEST_ASSERT_PASS_ASSERT(TEST_ASSERT_EQUAL(STD_OK, MXM_ExtractValueFromRegisterTest()));
}

/** test #MXM_ExtractValueFromRegister() with an ADC value */
void testMXM_ExtractValueFromRegisterADCValue(void) {
    const uint8_t lsb = 0xFCu;
    const uint8_t msb = 0xFFu;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_ADC_14BIT_VALUE, &value);
    TEST_ASSERT_EQUAL(0x3FFFu, value);
}

/** test #MXM_ExtractValueFromRegister() with a single bit in lsb */
void testMXM_ExtractValueFromRegisterLsbBitSet(void) {
    const uint8_t lsb = 0x10u;
    const uint8_t msb = 0x00u;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_STATUS1_ALRTINTRFC, &value);
    TEST_ASSERT_EQUAL(1u, value);
}

/** test #MXM_ExtractValueFromRegister() without a bit in lsb */
void testMXM_ExtractValueFromRegisterLsbBitUnset(void) {
    const uint8_t lsb = 0x00u;
    const uint8_t msb = 0x00u;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_STATUS1_ALRTINTRFC, &value);
    TEST_ASSERT_EQUAL(0u, value);
}

/** test #MXM_ExtractValueFromRegister() with whole lsb */
void testMXM_ExtractValueFromRegisterLsbCompletelySet(void) {
    const uint8_t lsb = 0xFFu;
    const uint8_t msb = 0x00u;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_BM_LSB, &value);
    TEST_ASSERT_EQUAL(0xFFu, value);
}

/** test #MXM_ExtractValueFromRegister() with whole msb */
void testMXM_ExtractValueFromRegisterMsbCompletelySet(void) {
    const uint8_t lsb = 0x00u;
    const uint8_t msb = 0xFFu;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_BM_MSB, &value);
    TEST_ASSERT_EQUAL(0xFFu, value);
}

/** test #MXM_ExtractValueFromRegister() with a single bit in msb */
void testMXM_ExtractValueFromRegisterMsbBitSet(void) {
    const uint8_t lsb = 0x00u;
    const uint8_t msb = 0x40u;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_STATUS1_ALRTRST, &value);
    TEST_ASSERT_EQUAL(1u, value);
}

/** test #MXM_ExtractValueFromRegister() with a single bit in msb and with full byte */
void testMXM_ExtractValueFromRegisterMsbBitSetFullBytes(void) {
    const uint8_t lsb = 0xFFu;
    const uint8_t msb = 0xFFu;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_STATUS1_ALRTRST, &value);
    TEST_ASSERT_EQUAL(1u, value);
}

/** test #MXM_ExtractValueFromRegister() without a bit in msb */
void testMXM_ExtractValueFromRegisterMsbBitUnset(void) {
    const uint8_t lsb = 0x00u;
    const uint8_t msb = 0x00u;
    uint16_t value    = 0x00u;
    MXM_ExtractValueFromRegister(lsb, msb, MXM_REG_STATUS1_ALRTRST, &value);
    TEST_ASSERT_EQUAL(0u, value);
}

void testMXM_ConvertInvalidConversionType(void) {
    /* bogus conversion type */
    uint8_t msb      = 0;
    uint8_t lsb      = 0;
    uint16_t voltage = 0;
    TEST_ASSERT_FAIL_ASSERT(MXM_Convert(lsb, msb, &voltage, (MXM_CONVERSION_TYPE_e)42, 5000));
}

void testMXM_ConvertFullScale(void) {
    /* full scale */
    uint8_t msb      = 0xFFu;
    uint8_t lsb      = 0xFCu;
    uint16_t voltage = 0;
    TEST_ASSERT_PASS_ASSERT(MXM_Convert(lsb, msb, &voltage, MXM_CONVERSION_UNIPOLAR, 5000));
    TEST_ASSERT_EQUAL(5000u, voltage);
}

void testMXM_ConvertLowScale(void) {
    /* low scale */
    uint8_t msb      = 0x00u;
    uint8_t lsb      = 0x00u;
    uint16_t voltage = 0;
    TEST_ASSERT_PASS_ASSERT(MXM_Convert(lsb, msb, &voltage, MXM_CONVERSION_UNIPOLAR, 5000));
    TEST_ASSERT_EQUAL(0u, voltage);
}

void testMXM_ConvertHalfScale(void) {
    /* half scale */
    uint8_t msb      = 0x80u;
    uint8_t lsb      = 0x00u;
    uint16_t voltage = 0;
    TEST_ASSERT_PASS_ASSERT(MXM_Convert(lsb, msb, &voltage, MXM_CONVERSION_UNIPOLAR, 5000));
    TEST_ASSERT_EQUAL(2500u, voltage);
}

void testMXM_ConvertSelfCheck(void) {
    fas_storeAssertLoc_Ignore();
    TEST_ASSERT_PASS_ASSERT(TEST_ASSERT_EQUAL(STD_OK, MXM_ConvertTest()));
}

void testMXM_FirstSetBitTest(void) {
    TEST_ASSERT_PASS_ASSERT(TEST_ASSERT_EQUAL(STD_OK, MXM_FirstSetBitTest()));
}

void testMXM_Unipolar14BitInto16BitWithInput0(void) {
    uint8_t lsb = 255;
    uint8_t msb = 255;
    MXM_Unipolar14BitInto16Bit(0u, &lsb, &msb);
    TEST_ASSERT_EQUAL(0u, lsb);
    TEST_ASSERT_EQUAL(0u, msb);
}

void testMXM_Unipolar14BitInto16BitWithInput3FFF(void) {
    uint8_t lsb = 0;
    uint8_t msb = 0;
    MXM_Unipolar14BitInto16Bit(0x3FFFu, &lsb, &msb);
    TEST_ASSERT_EQUAL(0xFCu, lsb);
    TEST_ASSERT_EQUAL(0xFFu, msb);
}

void testMXM_VoltageIntoUnipolar14Bit(void) {
    TEST_ASSERT_EQUAL(0u, MXM_VoltageIntoUnipolar14Bit(0, 5000));
    TEST_ASSERT_EQUAL(0x3FFFu, MXM_VoltageIntoUnipolar14Bit(5000, 5000));
    /* get the half-scale value by rightshifting */
    TEST_ASSERT_EQUAL((0x3FFFu >> 1u), MXM_VoltageIntoUnipolar14Bit(2500, 5000));

    /* test with fullscale value of block-voltage */
    TEST_ASSERT_EQUAL(0x3FFFu, MXM_VoltageIntoUnipolar14Bit(65000, 65000));
    /* test with halfscale value of block-voltage */
    TEST_ASSERT_EQUAL((0x3FFFu >> 1u), MXM_VoltageIntoUnipolar14Bit(32500, 65000));
}

void testScaleAndConvert2500(void) {
    /*  halfscale
        lsb = 0xFCu;
        msb = 0x7Fu;
        2.5V
    */
    uint16_t value_14bit = MXM_VoltageIntoUnipolar14Bit(2500u, 5000u);
    TEST_ASSERT_EQUAL((0x3FFF >> 1u), value_14bit);

    uint8_t lsb = 0;
    uint8_t msb = 0;
    MXM_Unipolar14BitInto16Bit(value_14bit, &lsb, &msb);
    TEST_ASSERT_EQUAL(0xFCu, lsb);
    TEST_ASSERT_EQUAL(0x7Fu, msb);
}

void testScaleAndConvert500(void) {
    /*  known values from driver:
        lsb = 0x98u;
        msb = 0x19u;
        0.5V
    */
    uint16_t value_14bit = MXM_VoltageIntoUnipolar14Bit(500u, 5000u);
    TEST_ASSERT_EQUAL((0x1998u >> 2u), value_14bit);

    uint8_t lsb = 0;
    uint8_t msb = 0;
    MXM_Unipolar14BitInto16Bit(value_14bit, &lsb, &msb);
    TEST_ASSERT_EQUAL(0x98u, lsb);
    TEST_ASSERT_EQUAL(0x19u, msb);
}

void testConversionExtractAndInsertRoundTripFullScale(void) {
    uint16_t voltage        = 5000u;
    uint16_t voltage_result = voltage;
    uint8_t lsb             = 0;
    uint8_t msb             = 0;

    uint16_t converted_voltage = MXM_VoltageIntoUnipolar14Bit(voltage, 5000);

    MXM_Unipolar14BitInto16Bit(converted_voltage, &lsb, &msb);

    TEST_ASSERT_PASS_ASSERT(MXM_Convert(lsb, msb, &voltage_result, MXM_CONVERSION_UNIPOLAR, 5000));

    TEST_ASSERT_EQUAL(voltage, voltage_result);
}

void testConversionExtractAndInsertRoundTripNull(void) {
    uint16_t voltage        = 0u;
    uint16_t voltage_result = voltage;
    uint8_t lsb             = 0;
    uint8_t msb             = 0;

    uint16_t converted_voltage = MXM_VoltageIntoUnipolar14Bit(voltage, 5000);

    MXM_Unipolar14BitInto16Bit(converted_voltage, &lsb, &msb);

    TEST_ASSERT_PASS_ASSERT(MXM_Convert(lsb, msb, &voltage_result, MXM_CONVERSION_UNIPOLAR, 5000));

    TEST_ASSERT_EQUAL(voltage, voltage_result);
}

void testConversionExtractAndInsertRoundTripSomeValue(void) {
    uint16_t voltage        = 4200u;
    uint16_t voltage_result = voltage;
    uint8_t lsb             = 0;
    uint8_t msb             = 0;

    uint16_t converted_voltage = MXM_VoltageIntoUnipolar14Bit(voltage, 5000);
    TEST_ASSERT_EQUAL(0x35C1u, converted_voltage);

    MXM_Unipolar14BitInto16Bit(converted_voltage, &lsb, &msb);
    TEST_ASSERT_EQUAL(0x04u, lsb);
    TEST_ASSERT_EQUAL(0xD7u, msb);

    TEST_ASSERT_PASS_ASSERT(MXM_Convert(lsb, msb, &voltage_result, MXM_CONVERSION_UNIPOLAR, 5000));

    TEST_ASSERT_EQUAL(voltage, voltage_result);
}

/* WARNING: in order for the MXM_ConvertModuleToString tests to work as expected,
   make sure that both test and source have been rebuilt (clear the build) after
   changing system defines such as BS_NR_OF_MODULES */

void testMXM_ConvertModuleToStringNullPointer(void) {
    uint8_t stringNumber          = 0u;
    uint16_t moduleNumberInString = 0u;
    TEST_ASSERT_FAIL_ASSERT(MXM_ConvertModuleToString(0u, NULL_PTR, &moduleNumberInString));
    TEST_ASSERT_FAIL_ASSERT(MXM_ConvertModuleToString(0u, &stringNumber, NULL_PTR));
}

void testMXM_ConvertModuleToStringTooManyForMaxim(void) {
    uint8_t stringNumber          = 0u;
    uint16_t moduleNumberInString = 0u;
    /* module index #MXM_MAXIMUM_NR_OF_MODULES should not be ok */
    TEST_ASSERT_FAIL_ASSERT(MXM_ConvertModuleToString(MXM_MAXIMUM_NR_OF_MODULES, &stringNumber, &moduleNumberInString));
}

void testMXM_ConvertModuleToStringTooManyForSystem(void) {
    uint8_t stringNumber          = 0u;
    uint16_t moduleNumberInString = 0u;
    /* The system cannot have more modules than BS_NR_OF_STRINGS*BS_NR_OF_MODULES */
    TEST_ASSERT_FAIL_ASSERT(
        MXM_ConvertModuleToString((BS_NR_OF_STRINGS * BS_NR_OF_MODULES), &stringNumber, &moduleNumberInString));
}

void testMXM_ConvertModuleToStringFirstInFirstString(void) {
    const uint8_t expectedStringNumber          = 0u;
    uint8_t stringNumber                        = 42u;
    const uint16_t expectedModuleNumberInString = 0u;
    uint16_t moduleNumberInString               = 42u;
    TEST_ASSERT_PASS_ASSERT(
        MXM_ConvertModuleToString((expectedStringNumber * BS_NR_OF_MODULES), &stringNumber, &moduleNumberInString));

    TEST_ASSERT_EQUAL(expectedStringNumber, stringNumber);
    TEST_ASSERT_EQUAL(expectedModuleNumberInString, moduleNumberInString);
}

void testMXM_ConvertModuleToStringFirstInSecondString(void) {
    /* skip this test if the BMS configuration does not fit to this test (we need a second string) */
    if (BS_NR_OF_STRINGS < 2u) {
        TEST_PASS_MESSAGE("This test is skipped due to the configuration of the BMS.");
    }
    const uint8_t expectedStringNumber          = 1u;
    uint8_t stringNumber                        = 0u;
    const uint16_t expectedModuleNumberInString = 0u;
    uint16_t moduleNumberInString               = 42u;
    TEST_ASSERT_PASS_ASSERT(
        MXM_ConvertModuleToString((expectedStringNumber * BS_NR_OF_MODULES), &stringNumber, &moduleNumberInString));

    TEST_ASSERT_EQUAL(expectedStringNumber, stringNumber);
    TEST_ASSERT_EQUAL(expectedModuleNumberInString, moduleNumberInString);
}

void testMXM_ConvertModuleToStringSecondInFirstString(void) {
    /* skip this test if the BMS configuration does not fit to this test (we need a second module) */
    if (BS_NR_OF_MODULES < 2u) {
        TEST_PASS_MESSAGE("This test is skipped due to the configuration of the BMS.");
    }
    const uint8_t expectedStringNumber          = 0u;
    uint8_t stringNumber                        = 42u;
    const uint16_t expectedModuleNumberInString = 1u;
    uint16_t moduleNumberInString               = 42u;
    TEST_ASSERT_PASS_ASSERT(MXM_ConvertModuleToString(
        ((expectedStringNumber * BS_NR_OF_MODULES) + expectedModuleNumberInString),
        &stringNumber,
        &moduleNumberInString));

    TEST_ASSERT_EQUAL(expectedStringNumber, stringNumber);
    TEST_ASSERT_EQUAL(expectedModuleNumberInString, moduleNumberInString);
}

void testMXM_ConvertModuleToStringSecondInSecondString(void) {
    /* skip this test if the BMS configuration does not fit to this test (we need a second module) */
    if ((BS_NR_OF_MODULES < 2u) || (BS_NR_OF_STRINGS < 2u)) {
        TEST_PASS_MESSAGE("This test is skipped due to the configuration of the BMS.");
    }
    const uint8_t expectedStringNumber          = 1u;
    uint8_t stringNumber                        = 42u;
    const uint16_t expectedModuleNumberInString = 1u;
    uint16_t moduleNumberInString               = 42u;
    TEST_ASSERT_PASS_ASSERT(MXM_ConvertModuleToString(
        (expectedModuleNumberInString + (expectedStringNumber * BS_NR_OF_MODULES)),
        &stringNumber,
        &moduleNumberInString));

    TEST_ASSERT_EQUAL(expectedStringNumber, stringNumber);
    TEST_ASSERT_EQUAL(expectedModuleNumberInString, moduleNumberInString);
}
