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
 * @file    test_bms-values-redundancy.c
 * @author  foxBMS Team
 * @date    2026-07-28 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the BMS Values implementation
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mockplausibility.h"
#include "Mockredundancy.h"

#include "bms-values.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
#define TEST_CELL_VOLTAGE_mV     (3700)
#define TEST_CELL_TEMP_ddegC     (250)
#define TEST_NR_CELLS_PER_STRING (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE)
#define TEST_NR_TEMP_PER_STRING  (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE)

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

static STD_RETURN_TYPE_e Read4Cb(void *p0, void *p1, void *p2, void *p3, int cmock_num_calls) {
    (void)cmock_num_calls;
    DATA_BLOCK_CURRENT_s *cur          = (DATA_BLOCK_CURRENT_s *)p0;
    DATA_BLOCK_SYSTEM_VOLTAGE_1_s *sv1 = (DATA_BLOCK_SYSTEM_VOLTAGE_1_s *)p1;
    DATA_BLOCK_SYSTEM_VOLTAGE_3_s *sv3 = (DATA_BLOCK_SYSTEM_VOLTAGE_3_s *)p2;
    DATA_BLOCK_POWER_s *pwr            = (DATA_BLOCK_POWER_s *)p3;

    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CURRENT, cur->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_SYSTEM_VOLTAGE_1, sv1->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_SYSTEM_VOLTAGE_3, sv3->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_POWER, pwr->header.uniqueId);
    return STD_OK;
}

/* DATA_READ_DATA(&bmsvl_tableMinimumMaximumValues) */
static STD_RETURN_TYPE_e Read1Cb(void *p0, int cmock_num_calls) {
    (void)cmock_num_calls;
    DATA_BLOCK_MIN_MAX_s *mm = (DATA_BLOCK_MIN_MAX_s *)p0;
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_MIN_MAX, mm->header.uniqueId);
    return STD_OK;
}

/* DATA_READ_DATA(&bmsvl_tableCellVoltageBase, &bmsvl_tableCellTemperatureBase) */
/* Success path: all cells/sensors valid and uniform.                        */
static STD_RETURN_TYPE_e Read2Cb(void *p0, void *p1, int cmock_num_calls) {
    (void)cmock_num_calls;
    DATA_BLOCK_CELL_VOLTAGE_s *v     = (DATA_BLOCK_CELL_VOLTAGE_s *)p0;
    DATA_BLOCK_CELL_TEMPERATURE_s *t = (DATA_BLOCK_CELL_TEMPERATURE_s *)p1;

    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE_BASE, v->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE_BASE, t->header.uniqueId);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                v->invalidCellVoltage[s][m][cb] = false;
                v->cellVoltage_mV[s][m][cb]     = TEST_CELL_VOLTAGE_mV;
            }
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                t->invalidCellTemperature[s][m][ts] = false;
                t->cellTemperature_ddegC[s][m][ts]  = TEST_CELL_TEMP_ddegC;
            }
        }
    }
    return STD_OK;
}

static STD_RETURN_TYPE_e Write2Cb(void *p0, void *p1, int cmock_num_calls) {
    if (cmock_num_calls == 0) {
        DATA_BLOCK_CELL_VOLTAGE_s *cellVoltages         = (DATA_BLOCK_CELL_VOLTAGE_s *)p0;
        DATA_BLOCK_CELL_TEMPERATURE_s *cellTemperatures = (DATA_BLOCK_CELL_TEMPERATURE_s *)p1;

        TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE, cellVoltages->header.uniqueId);
        TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE, cellTemperatures->header.uniqueId);

        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            TEST_ASSERT_EQUAL_UINT16(TEST_NR_CELLS_PER_STRING, cellVoltages->nrValidCellVoltages[s]);
            TEST_ASSERT_EQUAL_UINT16(TEST_NR_TEMP_PER_STRING, cellTemperatures->nrValidTemperatures[s]);
        }
    } else if (cmock_num_calls == 1) {
        DATA_BLOCK_PACK_VALUES_s *pack = (DATA_BLOCK_PACK_VALUES_s *)p0;
        DATA_BLOCK_MIN_MAX_s *mm       = (DATA_BLOCK_MIN_MAX_s *)p1;

        TEST_ASSERT_EQUAL(DATA_BLOCK_ID_PACK_VALUES, pack->header.uniqueId);
        TEST_ASSERT_EQUAL(DATA_BLOCK_ID_MIN_MAX, mm->header.uniqueId);

        for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
            TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, mm->minimumCellVoltage_mV[s]);
            TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, mm->maximumCellVoltage_mV[s]);
            TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, mm->averageCellVoltage_mV[s]);
            TEST_ASSERT_EQUAL_UINT16(TEST_NR_CELLS_PER_STRING, mm->validMeasuredCellVoltages[s]);

            TEST_ASSERT_EQUAL_INT16(TEST_CELL_TEMP_ddegC, mm->minimumTemperature_ddegC[s]);
            TEST_ASSERT_EQUAL_INT16(TEST_CELL_TEMP_ddegC, mm->maximumTemperature_ddegC[s]);
            TEST_ASSERT_EQUAL_FLOAT((float_t)TEST_CELL_TEMP_ddegC, mm->averageTemperature_ddegC[s]);
            TEST_ASSERT_EQUAL_UINT16(TEST_NR_TEMP_PER_STRING, mm->validMeasuredCellTemperatures[s]);
        }
    } else {
        TEST_FAIL_MESSAGE("Unexpected DATA_WRITE_DATA call count");
    }
    return STD_OK;
}

static void PlValidateCurrentCb(
    BMSVL_STATE_s *pBmsvlState,
    DATA_BLOCK_PACK_VALUES_s *pPack,
    const DATA_BLOCK_CURRENT_s *pCurrent,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pBmsvlState);
    TEST_ASSERT_NOT_NULL(pPack);
    TEST_ASSERT_NOT_NULL(pCurrent);
}

static void PlValidateStringVoltageCb(
    DATA_BLOCK_PACK_VALUES_s *pPack,
    const DATA_BLOCK_MIN_MAX_s *pMinMax,
    const DATA_BLOCK_SYSTEM_VOLTAGE_1_s *pSysV1,
    const DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltages,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pPack);
    TEST_ASSERT_NOT_NULL(pMinMax);
    TEST_ASSERT_NOT_NULL(pSysV1);
    TEST_ASSERT_NOT_NULL(pCellVoltages);
}

static void PlValidateBatteryVoltageCb(DATA_BLOCK_PACK_VALUES_s *pPack, int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pPack);
}

static void PlValidateHvBusCb(
    DATA_BLOCK_PACK_VALUES_s *pPack,
    const DATA_BLOCK_SYSTEM_VOLTAGE_3_s *pSysV3,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pPack);
    TEST_ASSERT_NOT_NULL(pSysV3);
}

static void PlValidatePowerCb(
    BMSVL_STATE_s *pBmsvlState,
    DATA_BLOCK_PACK_VALUES_s *pPack,
    DATA_BLOCK_POWER_s *pPower,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pBmsvlState);
    TEST_ASSERT_NOT_NULL(pPack);
    TEST_ASSERT_NOT_NULL(pPower);
}

static STD_RETURN_TYPE_e PlCheckVoltageSpreadCb_Ok(
    DATA_BLOCK_CELL_VOLTAGE_s *pV,
    const DATA_BLOCK_MIN_MAX_s *pMinMax,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    (void)pV;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_CELLS_PER_STRING, pMinMax->validMeasuredCellVoltages[s]);
    }
    return STD_OK; /* no recalculation */
}

static STD_RETURN_TYPE_e PlCheckTemperatureSpreadCb_Ok(
    DATA_BLOCK_CELL_TEMPERATURE_s *pT,
    const DATA_BLOCK_MIN_MAX_s *pMinMax,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    (void)pT;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_TEMP_PER_STRING, pMinMax->validMeasuredCellTemperatures[s]);
    }
    return STD_OK; /* no recalculation */
}

static MRC_REQUIRED_UPDATES_s MrcValidateAfeMeasurementsCb(
    DATA_BLOCK_CELL_VOLTAGE_s *pCellVoltages,
    DATA_BLOCK_CELL_TEMPERATURE_s *pCellTemperatures,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    MRC_REQUIRED_UPDATES_s requiredUpdates = {
        .updatedCellVoltageDatabaseEntry = true,
        .updatedTemperatureDatabaseEntry = true,
    };
    /* Must receive valid, non-NULL blocks. */
    TEST_ASSERT_NOT_NULL(pCellVoltages);
    TEST_ASSERT_NOT_NULL(pCellTemperatures);
    /* These are the *validated* (non-base) tables, so expect the non-base IDs. */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pCellVoltages->nrValidCellVoltages[s]     = TEST_NR_CELLS_PER_STRING;
        pCellTemperatures->nrValidTemperatures[s] = TEST_NR_TEMP_PER_STRING;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                pCellVoltages->invalidCellVoltage[s][m][cb] = false;
                pCellVoltages->cellVoltage_mV[s][m][cb]     = TEST_CELL_VOLTAGE_mV;
            }
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pCellTemperatures->invalidCellTemperature[s][m][ts] = false;
                pCellTemperatures->cellTemperature_ddegC[s][m][ts]  = TEST_CELL_TEMP_ddegC;
            }
        }
    }
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE, pCellVoltages->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE, pCellTemperatures->header.uniqueId);
    return requiredUpdates;
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Test extern function #BMSVL_UpdateSystemValues
 * @details Test that the BMSVL_UpdateSystemValues function updates the
 *          BMS-relevant database values correctly and successfully writes to
 *          the database.
 *          The following cases need to be tested:
 *          - Argument validation:
 *            - none
 *          - Routine validation:
 *            - RT1/1: function shall update BMS values correctly and
 *                     successfully writes to database
 */
void testBMSVL_UpdateSystemValues(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    DATA_Read4DataBlocks_StubWithCallback(Read4Cb);
    DATA_Read1DataBlock_StubWithCallback(Read1Cb);
    DATA_Read2DataBlocks_StubWithCallback(Read2Cb);
    DATA_Write2DataBlocks_StubWithCallback(Write2Cb);

    /* Do not simply copy the data, but validate it through the second
     * measurement path */
    MRC_ValidateAfeMeasurement_StubWithCallback(MrcValidateAfeMeasurementsCb);

    PL_ValidateCurrentMeasurement_StubWithCallback(PlValidateCurrentCb);
    PL_ValidateStringVoltageMeasurement_StubWithCallback(PlValidateStringVoltageCb);
    PL_ValidateBatteryVoltageMeasurement_StubWithCallback(PlValidateBatteryVoltageCb);
    PL_ValidateHighVoltageBusMeasurement_StubWithCallback(PlValidateHvBusCb);
    PL_ValidatePowerMeasurement_StubWithCallback(PlValidatePowerCb);

    PL_CheckVoltageSpread_StubWithCallback(PlCheckVoltageSpreadCb_Ok);
    PL_CheckTemperatureSpread_StubWithCallback(PlCheckTemperatureSpreadCb_Ok);

    /* ======= RT1/1: call function under test */
    BMSVL_UpdateSystemValues();

    /* ======= RT1/1: test output verification */
    /* verified in the Stubs */
}
