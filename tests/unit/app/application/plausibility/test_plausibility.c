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
 * @file    test_plausibility.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2020-04-01 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for plausibility checks for cell voltage and cell temperatures
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdiag.h"

#include "plausibility.h"
#include "test_assert_helper.h"

/*========== Definitions and Implementations for Unit Test ==================*/
/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s cellVoltage     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CURRENT_SENSOR_s currentSensor = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
/**@}*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testCheckPackVoltagecellVoltageandcurrentSensorBothZero(void) {
    int32_t packVoltage_mV = 0;
    int32_t highVoltage_mV = 0;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorBothMaximum(void) {
    int32_t packVoltage_mV = INT32_MAX;
    int32_t highVoltage_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceEqualsTolerance(void) {
    int32_t packVoltage_mV = 0;
    int32_t highVoltage_mV = PL_STRING_VOLTAGE_TOLERANCE_mV;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_NOT_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceEqualsToleranceMinusOne(void) {
    int32_t packVoltage_mV = 0;
    int32_t highVoltage_mV = PL_STRING_VOLTAGE_TOLERANCE_mV - 1;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceEqualsTolerancePlusOne(void) {
    int32_t packVoltage_mV = 0;
    int32_t highVoltage_mV = PL_STRING_VOLTAGE_TOLERANCE_mV + 1;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_NOT_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceEqualValue(void) {
    int32_t packVoltage_mV = 4242;
    int32_t highVoltage_mV = 4242;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceINT32MaxMinus(void) {
    int32_t packVoltage_mV = 0;
    int32_t highVoltage_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_NOT_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceINT32MaxPlus(void) {
    int32_t packVoltage_mV = INT32_MAX - (PL_STRING_VOLTAGE_TOLERANCE_mV + 100);
    int32_t highVoltage_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_NOT_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceINT32MaxPlusTooMuch(void) {
    int32_t packVoltage_mV = INT32_MAX - (PL_STRING_VOLTAGE_TOLERANCE_mV + 1);
    int32_t highVoltage_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_NOT_OK);
}

void testCheckPackVoltagecellVoltageandcurrentSensorDifferenceINT32Max(void) {
    int32_t packVoltage_mV = INT32_MAX;
    int32_t highVoltage_mV = INT32_MAX;
    TEST_ASSERT_EQUAL(PL_CheckStringVoltage(packVoltage_mV, highVoltage_mV), STD_OK);
}
