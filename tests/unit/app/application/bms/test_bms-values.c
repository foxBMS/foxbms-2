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
 * @file    test_bms-values.c
 * @author  foxBMS Team
 * @date    2026-07-28 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the BMS Values implementation
 * @details Covers initialization, the success path of BMSVL_UpdateSystemValues,
 *          the spread-check recalculation branch, and the all-invalid
 *          division-by-zero guards.
 *
 *          The BMS Values module is essentially an "orchestrator":
 *          it reads several database blocks, delegates the heavy lifting to
 *          the plausibility (PL_*) and redundancy modules, derives
 *          min/max/average values from validated cell data, and writes the
 *          results back to the database.
 *          It owns very little state itself (a couple of module-static tables
 *          and a timestamp state struct).
 *
 *          Consequences for testing:
 *          1) All collaborators (database access DATA_*, plausibility PL_*)
 *             are replaced by CMock stubs. We install *_StubWithCallback
 *             callbacks so that we can:
 *               - assert the *arguments* BMSVL passed in (call wiring /
 *                 contract),
 *               - inject deterministic *input data* into the tables BMSVL
 *                 owns,
 *               - control the *return value* of a collaborator to steer BMSVL
 *                 into a specific branch (e.g. the spread-check
 *                 recalculation).
 *          2) The relevant arithmetic lives in `static` functions. These are
 *             reached through thin `TEST_BMSVL_*` wrappers that bms-values.c
 *             exposes only when compiled with `UNITY_UNIT_TEST`.
 *          3) The module keeps its result table (min/max) as a *module-static*
 *             variable that the test cannot see directly. To inspect it after
 *             a call, the pointer that BMSVL forwards to the spread-check
 *             stub (see `test_pCapturedMinMax`).
 *
 *          Test-design rationale:
 *          - Isolate database and plausibility collaborators via CMock.
 *          - Drive deterministic data through callbacks.
 *          - Reach static internals through UNITY_UNIT_TEST wrappers.
 *          - Verify both call wiring and payload content.
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockdatabase.h"
#include "Mockdatabase_helper.h"
#include "Mockplausibility.h"

#include "database_cfg.h"

#include "bms-values.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
/* Canonical "valid measurement" values used on the success paths. They are
 * intentionally uniform across all cells so that the expected min == max ==
 * average, which keeps the assertions trivial and unambiguous. */
#define TEST_CELL_VOLTAGE_mV (3700)
#define TEST_CELL_TEMP_ddegC (250)

/* Expected number of valid entries per string when *every* cell/sensor is
 * valid. Derived from the battery-system geometry so the tests stay correct if
 * the configuration changes. */
#define TEST_NR_CELLS_PER_STRING (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE)
#define TEST_NR_TEMP_PER_STRING  (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE)

/* Distinct values used only by the "copy base -> validated" test, so a copy
 * failure cannot accidentally match the generic success values above. */
#define TEST_COPY_VOLTAGE_mV (3333)
#define TEST_COPY_TEMP_ddegC (222)

/* Two clearly different value sets (A and B) used by the derive-min/max test.
 * The test relies on A != B to distinguish "recalculation happened" from
 * "recalculation did NOT happen" (see testBMSVL_DeriveMinimumMaximumValues). */
#define TEST_DERIVE_VOLTAGE_A_mV (3600)
#define TEST_DERIVE_VOLTAGE_B_mV (3800)
#define TEST_DERIVE_TEMP_A_ddegC (200)
#define TEST_DERIVE_TEMP_B_ddegC (300)

/* Shared state used to steer the derive-min/max spread-check stubs:
 * - test_deriveSeed*     : the synthetic cell value the stub writes into the
 *                          (module-static) cell table on each call.
 * - test_derive*Return   : return value of the respective spread check; this
 *                          selects the recalculation branch (STD_NOT_OK) or
 *                          the no-recalculation branch (STD_OK) inside BMSVL.
 * - test_pCapturedMinMax : pointer to the module-static min/max result table,
 *                          captured from the stub so the test can inspect it.
 */
static int16_t test_deriveSeedVoltage_mV                = 0;
static int16_t test_deriveSeedTemp_ddegC                = 0;
static STD_RETURN_TYPE_e test_deriveVoltageSpreadReturn = STD_OK;
static STD_RETURN_TYPE_e test_deriveTempSpreadReturn    = STD_OK;
static DATA_BLOCK_MIN_MAX_s *test_pCapturedMinMax       = NULL_PTR;
/* Tracks whether the write callback ran at all.
 *
 * `cmock_num_calls` inside the callback only tells us which invocation number
 * we are currently in *after* the callback has already been entered. If the
 * production code never calls DATA_Write4DataBlocks(), the callback is never
 * executed and no assertion on `cmock_num_calls` would run. This counter closes
 * that gap by giving the test an observable post-condition outside the
 * callback. */
static uint8_t test_write4Calls = 0u;

/* --- DATA_DatabaseEntryUpdatedAtLeastOnce stub ----------------------------
 * BMSVL only copies the base measurements into the validated tables if the
 * destination entry has already been updated once. For the success paths we
 * therefore report "already updated" (true), otherwise no copy would happen.
 * Call order inside BMSVL_CopyBaseMeasurementsToValidatedMeasurements():
 *   call 0 -> cell voltage header, call 1 -> cell temperature header. */
static bool DatabaseEntryUpdatedAtLeastOnceCb(DATA_BLOCK_HEADER_s header, int cmock_num_calls) {
    if ((cmock_num_calls % 2) == 0) {
        TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE, header.uniqueId);
    } else {
        TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE, header.uniqueId);
    }
    return true;
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Unit Test Helper Functions =====================================*/

/* Overall mocking strategy:
 * - DATA_* callbacks emulate database I/O and verify table IDs (proving BMSVL
 *   asked for the correct database block).
 * - PL_* callbacks verify integration contracts (correct arguments forwarded)
 *   without re-testing the plausibility helpers themselves.
 * - Callback return values force target branches deterministically. */

/* --- PL_CheckVoltageSpread stub for the derive-min/max test ---------------
 * BMSVL calls this AFTER it has computed a first min/max from the current cell
 * table. We (ab)use the stub to:
 *   1) re-seed ALL cell voltages to `test_deriveSeedVoltage_mV` and mark them
 *      valid (this is what a following recalculation would read),
 *   2) capture the pointer to the module-static min/max table for later
 *      inspection,
 *   3) return `test_deriveVoltageSpreadReturn` to select the recalculation
 *      branch (STD_NOT_OK) or not (STD_OK). */
static STD_RETURN_TYPE_e DeriveVoltageSpreadCb(
    DATA_BLOCK_CELL_VOLTAGE_s *pV,
    const DATA_BLOCK_MIN_MAX_s *pMinMax,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pV);
    TEST_ASSERT_NOT_NULL(pMinMax);
    test_pCapturedMinMax = (DATA_BLOCK_MIN_MAX_s *)pMinMax; /* for post-call checks */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                pV->invalidCellVoltage[s][m][cb] = false;
                pV->cellVoltage_mV[s][m][cb]     = test_deriveSeedVoltage_mV;
            }
        }
    }
    return test_deriveVoltageSpreadReturn;
}

/* --- PL_CheckTemperatureSpread stub for the derive-min/max test -----------
 * Mirror of DeriveVoltageSpreadCb for temperatures. Kept independent so both
 * spread-check branches can be controlled separately. Note: it iterates over
 * TEMP sensors (not cell blocks), matching the temperature array dimension. */
static STD_RETURN_TYPE_e DeriveTemperatureSpreadCb(
    DATA_BLOCK_CELL_TEMPERATURE_s *pT,
    const DATA_BLOCK_MIN_MAX_s *pMinMax,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    (void)pMinMax;
    TEST_ASSERT_NOT_NULL(pT);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pT->invalidCellTemperature[s][m][ts] = false;
                pT->cellTemperature_ddegC[s][m][ts]  = test_deriveSeedTemp_ddegC;
            }
        }
    }
    return test_deriveTempSpreadReturn;
}

/* --- DATA_READ_DATA(&base_v, &base_t) stub for the copy test --------------
 * The function under test reads two *_BASE tables and copies their payload
 * into the caller-provided destination tables. This stub:
 *   - proves BMSVL passed the *_BASE tables (verified via their unique IDs),
 *   - fills the base tables with known values so the test can later confirm
 *     that the copy really happened. */
static STD_RETURN_TYPE_e CopyReadBaseCb(void *p0, void *p1, int cmock_num_calls) {
    (void)cmock_num_calls;
    DATA_BLOCK_CELL_VOLTAGE_s *baseV     = (DATA_BLOCK_CELL_VOLTAGE_s *)p0;
    DATA_BLOCK_CELL_TEMPERATURE_s *baseT = (DATA_BLOCK_CELL_TEMPERATURE_s *)p1;

    /* The internal base tables are initialized with the *_BASE unique IDs.   */
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE_BASE, baseV->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE_BASE, baseT->header.uniqueId);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                baseV->cellVoltage_mV[s][m][cb]     = TEST_COPY_VOLTAGE_mV;
                baseV->invalidCellVoltage[s][m][cb] = false;
            }
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                baseT->cellTemperature_ddegC[s][m][ts]  = TEST_COPY_TEMP_ddegC;
                baseT->invalidCellTemperature[s][m][ts] = false;
            }
        }
    }
    return STD_OK;
}

/* --- DATA_WRITE_DATA stub for BMSVL_Initialize (success path) --------------
 * BMSVL_Initialize has no return value; its only observable effect is the
 * database write. This stub inspects that write and confirms EVERYTHING was
 * invalidated (all cell/module/string flags true, all pack bitmasks 0x01),
 * which is exactly the post-initialization contract of the module.
 * cmock_num_calls == 0 asserts that the write happens exactly once. */
static STD_RETURN_TYPE_e DataWriteCallbackReturnStdOk(void *p0, void *p1, void *p2, int cmock_num_calls) {
    DATA_BLOCK_CELL_VOLTAGE_s *cv     = (DATA_BLOCK_CELL_VOLTAGE_s *)p0;
    DATA_BLOCK_CELL_TEMPERATURE_s *ct = (DATA_BLOCK_CELL_TEMPERATURE_s *)p1;
    DATA_BLOCK_PACK_VALUES_s *pack    = (DATA_BLOCK_PACK_VALUES_s *)p2;

    TEST_ASSERT_EQUAL(0, cmock_num_calls);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_TRUE(cv->invalidStringVoltage[s]);

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            TEST_ASSERT_TRUE(cv->invalidModuleVoltage[s][m]);

            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_TRUE(cv->invalidCellVoltage[s][m][cb]);
            }
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_TRUE(ct->invalidCellTemperature[s][m][ts]);
            }
        }

        TEST_ASSERT_EQUAL_UINT8(0x01, pack->invalidStringVoltage[s]);
        TEST_ASSERT_EQUAL_UINT8(0x01, pack->invalidStringCurrent[s]);
        TEST_ASSERT_EQUAL_UINT8(0x01, pack->invalidStringPower[s]);
    }
    TEST_ASSERT_EQUAL(0x01, pack->invalidPackCurrent);
    TEST_ASSERT_EQUAL(0x01, pack->invalidBatteryVoltage);
    TEST_ASSERT_EQUAL(0x01, pack->invalidHvBusVoltage);
    TEST_ASSERT_EQUAL(0x01, pack->invalidPackPower);
    return STD_OK;
}

/* --- DATA_WRITE_DATA stub for BMSVL_Initialize (failure path) --------------
 * Used only for the RT2/2 case: it forces the database write to fail so the
 * FAS_ASSERT(retval == STD_OK) inside BMSVL_Initialize triggers. We do not
 * care about the payload here; the point is only to exercise the error path.
 */
static STD_RETURN_TYPE_e DataWriteCallbackReturnStdNotOk(void *p0, void *p1, void *p2, int cmock_num_calls) {
    /* only use for the RT2/2 case in testBMSVL_Initialize */
    /* Do not care about the internals, only return that writing the database
     * failed */
    (void)p0;
    (void)p1;
    (void)p2;
    (void)cmock_num_calls;
    return STD_NOT_OK;
}

/*========== Shared DATA_* read callbacks (BMSVL_UpdateSystemValues) =========*/
/* BMSVL_UpdateSystemValues issues several reads before processing. The stubs
 * below both verify that the correct blocks were requested (by unique ID) and
 * supply deterministic inputs for the downstream calculation.
 * DATA_READ_DATA(&tableCurrent, &tableSystemVoltage1, &tableSystemVoltage3, &tablePower) */
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

/* DATA_READ_DATA(&bmsvl_tableCellVoltageBase, &bmsvl_tableCellTemperatureBase)
 * Success path only: mark all cells/sensors valid with the uniform canonical
 * values so the derived min/max/average are fully predictable. */
static STD_RETURN_TYPE_e Read2Cb(void *p0, void *p1, int cmock_num_calls) {
    (void)cmock_num_calls;
    DATA_BLOCK_CELL_VOLTAGE_s *v     = (DATA_BLOCK_CELL_VOLTAGE_s *)p0;
    DATA_BLOCK_CELL_TEMPERATURE_s *t = (DATA_BLOCK_CELL_TEMPERATURE_s *)p1;

    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE_BASE, v->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE_BASE, t->header.uniqueId);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        v->nrValidCellVoltages[s] = TEST_NR_CELLS_PER_STRING;
        t->nrValidTemperatures[s] = TEST_NR_TEMP_PER_STRING;
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

/*========== Plausibility validate callbacks (all return void) ==============*/
/* These PL_Validate* stubs do not compute anything; their sole purpose is to
 * assert that BMSVL forwarded non-NULL, correctly typed arguments to each
 * plausibility stage. This locks the integration contract without re-testing
 * the plausibility module (which has its own unit tests). */

/* Current validation: BMSVL_STATE_s must be non-null because the real helper
 * uses timestamps stored in that state. */
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

/* String-voltage validation argument wiring. */
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

/* Battery-voltage validation argument wiring. */
static void PlValidateBatteryVoltageCb(DATA_BLOCK_PACK_VALUES_s *pPack, int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pPack);
}

/* HV-bus validation argument wiring. */
static void PlValidateHvBusCb(
    DATA_BLOCK_PACK_VALUES_s *pPack,
    const DATA_BLOCK_SYSTEM_VOLTAGE_3_s *pSysV3,
    int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pPack);
    TEST_ASSERT_NOT_NULL(pSysV3);
}

/* Power validation: BMSVL_STATE_s must be non-null (timestamp/fallback logic). */
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

/*========== Spread checks - success path (return STD_OK) ===================*/
/* In BMSVL_UpdateSystemValues the spread checks are invoked *after* the first
 * min/max calculation. Returning STD_OK here means "no cell was newly flagged
 * invalid", so BMSVL must NOT recalculate. We also verify that the first
 * calculation already produced the full valid-count, confirming the success
 * path of the arithmetic. */
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

/*========== Write callbacks ================================================*/
/* Final DB write of BMSVL_UpdateSystemValues. Because all inputs are uniform,
 * we expect validated cell tables plus min/max data to contain only the
 * canonical values and full valid-counts for every string.
 *
 * `cmock_num_calls` is still useful here: once the callback is entered, it
 * proves that this is the first invocation of DATA_Write4DataBlocks(). It does
 * *not* prove that the callback was entered at all. Therefore we additionally
 * increment `test_write4Calls` and assert on that counter after the function
 * under test returns. */
static STD_RETURN_TYPE_e Write4Cb(void *p0, void *p1, void *p2, void *p3, int cmock_num_calls) {
    test_write4Calls++;
    TEST_ASSERT_EQUAL_UINT8(0u, cmock_num_calls);
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltages         = (DATA_BLOCK_CELL_VOLTAGE_s *)p0;
    DATA_BLOCK_CELL_TEMPERATURE_s *cellTemperatures = (DATA_BLOCK_CELL_TEMPERATURE_s *)p1;
    DATA_BLOCK_PACK_VALUES_s *pack                  = (DATA_BLOCK_PACK_VALUES_s *)p2;
    DATA_BLOCK_MIN_MAX_s *mm                        = (DATA_BLOCK_MIN_MAX_s *)p3;

    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE, cellVoltages->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE, cellTemperatures->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_PACK_VALUES, pack->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_MIN_MAX, mm->header.uniqueId);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_CELLS_PER_STRING, cellVoltages->nrValidCellVoltages[s]);
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_TEMP_PER_STRING, cellTemperatures->nrValidTemperatures[s]);

        TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, mm->minimumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, mm->maximumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, mm->averageCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_CELLS_PER_STRING, mm->validMeasuredCellVoltages[s]);

        TEST_ASSERT_EQUAL_INT16(TEST_CELL_TEMP_ddegC, mm->minimumTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_TEMP_ddegC, mm->maximumTemperature_ddegC[s]);
        /* average temperature is a float_t -> compare with FLOAT matcher. */
        TEST_ASSERT_EQUAL_FLOAT((float_t)TEST_CELL_TEMP_ddegC, mm->averageTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_TEMP_PER_STRING, mm->validMeasuredCellTemperatures[s]);
    }
    return STD_OK;
}

/*========== Test Cases =====================================================*/
/* Externalized Static Functions */

/**
 * @brief   Tests static function BMSVL_CalculateCellVoltageMinMaxAverage
 * @details The function calculates the minimum, maximum and average values of
 *          the cell voltages.
 *          The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pValidatedVoltages &rarr; assert
 *            - AT2/2: NULL_PTR for pMinMaxAverageValues &rarr; assert
 *          - Routine validation:
 *            - RT1/2: One or more strings have valid cell voltages &rarr; STD_OK
 *            - RT2/2: One or more strings have all invalid cell voltages
 *                     &rarr; STD_NOT_OK
 *
 *          RT2/2 additionally documents the "sentinel" behavior: when there is
 *          no valid cell, `min` keeps its INT16_MAX seed and `max` keeps its
 *          INT16_MIN seed, the average is forced to 0 (division-by-zero
 *          guard), and the function reports STD_NOT_OK.
 */
void testBMSVL_CalculateCellVoltageMinMaxAverage(void) {
    DATA_BLOCK_CELL_VOLTAGE_s atDummyVarVoltage = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    DATA_BLOCK_MIN_MAX_s atDummyVarMinMax       = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMSVL_CalculateCellVoltageMinMaxAverage(NULL_PTR, &atDummyVarMinMax));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMSVL_CalculateCellVoltageMinMaxAverage(&atDummyVarVoltage, NULL_PTR));

    /* ======= Routine tests =============================================== */
    DATA_BLOCK_CELL_VOLTAGE_s voltages = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    DATA_BLOCK_MIN_MAX_s minMax        = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};

    /* ======= RT1/2: Test implementation
     * All cells valid and uniform -> min == max == average == the value, and
     * the valid count equals the full number of cells per string. */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                voltages.invalidCellVoltage[s][m][cb] = false;
                voltages.cellVoltage_mV[s][m][cb]     = TEST_CELL_VOLTAGE_mV;
            }
        }
    }
    /* ======= RT1/2: call function under test */
    STD_RETURN_TYPE_e retval1 = TEST_BMSVL_CalculateCellVoltageMinMaxAverage(&voltages, &minMax);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, retval1);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_CELLS_PER_STRING, minMax.validMeasuredCellVoltages[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, minMax.averageCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, minMax.minimumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_VOLTAGE_mV, minMax.maximumCellVoltage_mV[s]);
    }

    /* ======= RT2/2: Test implementation
     * Every cell invalid; the payload value (3000) must be ignored entirely
     * because invalid cells are skipped. This exercises the div-by-zero guard.
     */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                voltages.invalidCellVoltage[s][m][cb] = true;
                voltages.cellVoltage_mV[s][m][cb]     = 3000; /* must be ignored */
            }
        }
    }

    /* ======= RT2/2: call function under test */
    STD_RETURN_TYPE_e retval2 = TEST_BMSVL_CalculateCellVoltageMinMaxAverage(&voltages, &minMax);

    /* ======= RT2/2: test output verification
     * No valid cell -> STD_NOT_OK, average forced to 0, and min/max keep their
     * INT16_MAX/INT16_MIN sentinel seeds. */
    TEST_ASSERT_EQUAL(STD_NOT_OK, retval2);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(0u, minMax.validMeasuredCellVoltages[s]);
        TEST_ASSERT_EQUAL_INT16(0, minMax.averageCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(INT16_MAX, minMax.minimumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(INT16_MIN, minMax.maximumCellVoltage_mV[s]);
    }
}

/**
 * @brief   Tests static function BMSVL_CalculateCellTemperatureMinMaxAverage
 * @details The function calculates the minimum, maximum and average values of
 *          the cell temperatures.
 *          The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for pValidatedTemperatures &rarr; assert
 *            - AT2/2: NULL_PTR for pMinMaxAverageValues &rarr; assert
 *          - Routine validation:
 *            - RT1/2: One or more strings have valid cell temperatures &rarr;
 *                     STD_OK
 *            - RT2/2: One or more strings have all invalid cell temperatures
 *                     &rarr; STD_NOT_OK
 */
void testBMSVL_CalculateCellTemperatureMinMaxAverage(void) {
    DATA_BLOCK_CELL_TEMPERATURE_s atDummyVarTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    DATA_BLOCK_MIN_MAX_s atDummyVarMinMax               = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMSVL_CalculateCellTemperatureMinMaxAverage(NULL_PTR, &atDummyVarMinMax));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMSVL_CalculateCellTemperatureMinMaxAverage(&atDummyVarTemperature, NULL_PTR));

    /* ======= Routine tests =============================================== */
    DATA_BLOCK_CELL_TEMPERATURE_s temperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    DATA_BLOCK_MIN_MAX_s minMax                = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};

    /* ======= RT1/2: Test implementation
     * All sensors valid and uniform. NOTE: iterate over TEMP sensors. */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                temperatures.invalidCellTemperature[s][m][ts] = false;
                temperatures.cellTemperature_ddegC[s][m][ts]  = TEST_CELL_TEMP_ddegC;
            }
        }
    }
    /* ======= RT1/2: call function under test */
    STD_RETURN_TYPE_e retval1 = TEST_BMSVL_CalculateCellTemperatureMinMaxAverage(&temperatures, &minMax);
    /* ======= RT1/2: test output verification */
    TEST_ASSERT_EQUAL(STD_OK, retval1);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(TEST_NR_TEMP_PER_STRING, minMax.validMeasuredCellTemperatures[s]);
        /* average is float_t -> use FLOAT matcher, not INT16. */
        TEST_ASSERT_EQUAL_FLOAT((float_t)TEST_CELL_TEMP_ddegC, minMax.averageTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_TEMP_ddegC, minMax.minimumTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_CELL_TEMP_ddegC, minMax.maximumTemperature_ddegC[s]);
    }

    /* ======= RT2/2: Test implementation
     * Every sensor invalid; payload value (3000) must be ignored. Iterate over
     * TEMP sensors so the whole temperature array is covered. */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                temperatures.invalidCellTemperature[s][m][ts] = true;
                temperatures.cellTemperature_ddegC[s][m][ts]  = 3000; /* must be ignored */
            }
        }
    }

    /* ======= RT2/2: call function under test */
    STD_RETURN_TYPE_e retval2 = TEST_BMSVL_CalculateCellTemperatureMinMaxAverage(&temperatures, &minMax);

    /* ======= RT2/2: test output verification
     * No valid sensor -> STD_NOT_OK, average forced to 0.0f, min/max keep their
     * INT16_MAX/INT16_MIN sentinel seeds. */
    TEST_ASSERT_EQUAL(STD_NOT_OK, retval2);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(0u, minMax.validMeasuredCellTemperatures[s]);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, minMax.averageTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_INT16(INT16_MAX, minMax.minimumTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_INT16(INT16_MIN, minMax.maximumTemperature_ddegC[s]);
    }
}

/**
 * @brief   Tests static function BMSVL_DeriveMinimumMaximumValues
 * @details The function derives the minimum, maximum and average values of
 *          the cell voltages and temperatures, and then re-derives them if a
 *          spread check detected a newly invalid measurement.
 *
 *          Test idea (the crucial part to understand):
 *          BMSVL_DeriveMinimumMaximumValues works on MODULE-STATIC cell tables
 *          the test cannot touch directly. Our spread-check stubs are the only
 *          hook into those tables, and they run *between* the first
 *          calculation and the (conditional) recalculation.
 *          The resulting sequence per call is:
 *            1) BMSVL computes min/max from whatever is currently in the
 *               static table (that is the value written by the PREVIOUS stub
 *               call),
 *            2) BMSVL calls our spread stub, which re-seeds the static table
 *               with the CURRENT seed value and returns OK/NOT_OK,
 *            3) if NOT_OK, BMSVL recalculates min/max, now reading the value
 *                the stub just wrote.
 *
 *          To exploit this we "prime" the static table with value A on a first
 *          throw-away call, then perform the real call seeding value B:
 *          - RT1/2 (spread returns STD_OK): no recalculation -> the result
 *            must still be A (from step 1, based on the primed table), NOT B.
 *          - RT2/2 (spread returns STD_NOT_OK): recalculation happens -> the
 *            result must be B (the value the stub wrote during step 2),
 *            proving the recalculation branch was taken.
 *
 *          - Argument validation: none
 *          - Routine validation:
 *            - RT1/2: spread checks STD_OK  -> result stays value A
 *            - RT2/2: spread checks STD_NOT_OK -> result becomes value B
 */
void testBMSVL_DeriveMinimumMaximumValues(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    PL_CheckVoltageSpread_StubWithCallback(DeriveVoltageSpreadCb);
    PL_CheckTemperatureSpread_StubWithCallback(DeriveTemperatureSpreadCb);

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    /* Prime step: seed value A and force a recalculation so the static tables
     * end up holding value A after this throw-away call. */
    test_deriveSeedVoltage_mV      = TEST_DERIVE_VOLTAGE_A_mV;
    test_deriveSeedTemp_ddegC      = TEST_DERIVE_TEMP_A_ddegC;
    test_deriveVoltageSpreadReturn = STD_NOT_OK;
    test_deriveTempSpreadReturn    = STD_NOT_OK;
    TEST_BMSVL_DeriveMinimumMaximumValues();

    /* Actual RT1/2 call: seed value B but return STD_OK from both spread
     * checks -> no recalculation. The first calculation reads the primed
     * value A, so A must remain the final result (proving B was NOT used). */
    test_deriveSeedVoltage_mV      = TEST_DERIVE_VOLTAGE_B_mV;
    test_deriveSeedTemp_ddegC      = TEST_DERIVE_TEMP_B_ddegC;
    test_deriveVoltageSpreadReturn = STD_OK;
    test_deriveTempSpreadReturn    = STD_OK;

    /* ======= RT1/2: call function under test */
    TEST_BMSVL_DeriveMinimumMaximumValues();

    /* ======= RT1/2: test output verification */
    TEST_ASSERT_NOT_NULL(test_pCapturedMinMax);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* no recalculation -> still value A, NOT the seeded value B */
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_VOLTAGE_A_mV, test_pCapturedMinMax->minimumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_VOLTAGE_A_mV, test_pCapturedMinMax->maximumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_VOLTAGE_A_mV, test_pCapturedMinMax->averageCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_TEMP_A_ddegC, test_pCapturedMinMax->minimumTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_TEMP_A_ddegC, test_pCapturedMinMax->maximumTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_FLOAT((float_t)TEST_DERIVE_TEMP_A_ddegC, test_pCapturedMinMax->averageTemperature_ddegC[s]);
    }

    /* ======= RT2/2: Test implementation */
    /* Re-prime the static tables with value A (again via a forced-recalculation call). */
    test_deriveSeedVoltage_mV      = TEST_DERIVE_VOLTAGE_A_mV;
    test_deriveSeedTemp_ddegC      = TEST_DERIVE_TEMP_A_ddegC;
    test_deriveVoltageSpreadReturn = STD_NOT_OK;
    test_deriveTempSpreadReturn    = STD_NOT_OK;
    TEST_BMSVL_DeriveMinimumMaximumValues();

    /* Actual RT2/2 call: seed value B and force recalculation (STD_NOT_OK).
     * First calc reads primed A; the stub then writes B; recalculation reads B,
     * so the final result must be B -> confirms the recalculation path. */
    test_deriveSeedVoltage_mV      = TEST_DERIVE_VOLTAGE_B_mV;
    test_deriveSeedTemp_ddegC      = TEST_DERIVE_TEMP_B_ddegC;
    test_deriveVoltageSpreadReturn = STD_NOT_OK;
    test_deriveTempSpreadReturn    = STD_NOT_OK;

    /* ======= RT2/2: call function under test */
    TEST_BMSVL_DeriveMinimumMaximumValues();

    /* ======= RT2/2: test output verification */
    TEST_ASSERT_NOT_NULL(test_pCapturedMinMax);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* recalculation happened -> final value is the recalculated value B */
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_VOLTAGE_B_mV, test_pCapturedMinMax->minimumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_VOLTAGE_B_mV, test_pCapturedMinMax->maximumCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_VOLTAGE_B_mV, test_pCapturedMinMax->averageCellVoltage_mV[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_TEMP_B_ddegC, test_pCapturedMinMax->minimumTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_INT16(TEST_DERIVE_TEMP_B_ddegC, test_pCapturedMinMax->maximumTemperature_ddegC[s]);
        TEST_ASSERT_EQUAL_FLOAT((float_t)TEST_DERIVE_TEMP_B_ddegC, test_pCapturedMinMax->averageTemperature_ddegC[s]);
    }
}

/**
 * @brief   Tests static function BMSVL_CopyBaseMeasurementsToValidatedMeasurements
 * @details The function copies the *_BASE database payload into the caller's
 *          validated-measurement tables, while preserving the caller's headers
 *          (so the destination keeps its own unique ID). This is only compiled
 *          in the non-redundant build variant.
 *          The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: NULL_PTR for destV &rarr; assert
 *            - AT2/2: NULL_PTR for destT &rarr; assert
 *          - Routine validation:
 *            - RT1/1: base payload copied AND destination headers preserved
 */
void testBMSVL_CopyBaseMeasurementsToValidatedMeasurements(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_VOLTAGE_s atDummyVarVoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    DATA_BLOCK_CELL_TEMPERATURE_s atDummyVarTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMSVL_CopyBaseMeasurementsToValidatedMeasurements(NULL_PTR, &atDummyVarTemperature));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_BMSVL_CopyBaseMeasurementsToValidatedMeasurements(&atDummyVarVoltage, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* Destination tables carry the *non-base* IDs; these headers must survive. */
    DATA_BLOCK_CELL_VOLTAGE_s destV     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    DATA_BLOCK_CELL_TEMPERATURE_s destT = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};

    /* Pre-fill destinations with values that differ from the base data, so we
     * can prove the copy actually overwrote them.                            */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                destV.cellVoltage_mV[s][m][cb]     = 0;
                destV.invalidCellVoltage[s][m][cb] = true;
            }
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                destT.cellTemperature_ddegC[s][m][ts]  = 0;
                destT.invalidCellTemperature[s][m][ts] = true;
            }
        }
    }

    /* The stub fills the internal *_BASE tables with the TEST_COPY_* values. */
    DATA_Read2DataBlocks_StubWithCallback(CopyReadBaseCb);
    /* Report that both validated entries have already been updated once, so
     * the copy is actually performed. */
    DATA_DatabaseEntryUpdatedAtLeastOnce_StubWithCallback(DatabaseEntryUpdatedAtLeastOnceCb);

    /* ======= RT1/1: call function under test */
    TEST_BMSVL_CopyBaseMeasurementsToValidatedMeasurements(&destV, &destT);

    /* ======= RT1/1: test output verification */
    /* There are two things to verify:
     * 1) Base payload was copied into the destinations.
     * 2) Destination headers were preserved (NOT replaced by *_BASE IDs).
     */
    /* 1) Data was copied into the destinations. */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_EQUAL_INT16(TEST_COPY_VOLTAGE_mV, destV.cellVoltage_mV[s][m][cb]);
                TEST_ASSERT_FALSE(destV.invalidCellVoltage[s][m][cb]);
            }
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_EQUAL_INT16(TEST_COPY_TEMP_ddegC, destT.cellTemperature_ddegC[s][m][ts]);
                TEST_ASSERT_FALSE(destT.invalidCellTemperature[s][m][ts]);
            }
        }
    }
    /* 2) Destination headers were preserved (NOT overwritten by *_BASE IDs). */
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE, destV.header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE, destT.header.uniqueId);
}

/* Extern Functions */
/**
 * @brief   Test extern function #BMSVL_Initialize
 * @details BMSVL_Initialize invalidates all BMS-relevant values and writes them
 *          to the database once. The two routine cases differ only in the
 *          database-write return value:
 *          - RT1/2 exercises the success path and fully verifies (inside the
 *            write stub) that every value was invalidated as required.
 *          - RT2/2 forces the write to fail; BMSVL_Initialize asserts on the
 *            failed write (FAS_ASSERT), which we verify via TEST_ASSERT_FAIL_ASSERT.
 *          - Argument validation: none
 *          - Routine validation:
 *            - RT1/2: correct invalidation + successful DB write
 *            - RT2/2: DB write fails -> fail-assert
 */
void testBMSVL_Initialize(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    DATA_Write3DataBlocks_StubWithCallback(DataWriteCallbackReturnStdOk);
    /* ======= RT1/2: call function under test */
    BMSVL_Initialize();
    /* ======= RT1/2: test output verification */
    /* verified inside DataWriteCallbackReturnStdOk (payload + single-call) */

    /* ======= RT2/2: Test implementation */
    DATA_Write3DataBlocks_StubWithCallback(DataWriteCallbackReturnStdNotOk);
    /* ======= RT2/2: call function under test */
    TEST_ASSERT_FAIL_ASSERT(BMSVL_Initialize());
    /* ======= RT2/2: test output verification */
    /* verified via fail-assert on the failed database write */
}

/**
 * @brief   Test extern function #BMSVL_UpdateSystemValues
 * @details End-to-end success path of the orchestration function.
 *          All collaborators are stubbed:
 *          - the DATA_Read* stubs verify BMSVL requested the right blocks and
 *            inject uniform, valid cell/temperature data,
 *          - the PL_Validate* stubs verify argument wiring only,
 *          - the spread-check stubs return STD_OK (no recalculation) and check
 *            the intermediate valid-counts,
 *          - the final DATA_Write4 stub (Write4Cb) verifies the derived
 *            min/max/average payload written back to the database,
 *          - `test_write4Calls` verifies that DATA_Write4DataBlocks() was
 *            actually called exactly once. This complements
 *            `cmock_num_calls`, which is only available if the callback was
 *            entered in the first place.
 *          - Argument validation: none
 *          - Routine validation:
 *            - RT1/1: values updated correctly and written to the database
 */
void testBMSVL_UpdateSystemValues(void) {
    /* ======= Assertion tests ============================================= */
    /* none */

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    DATA_Read4DataBlocks_StubWithCallback(Read4Cb);
    DATA_Read1DataBlock_StubWithCallback(Read1Cb);
    DATA_Read2DataBlocks_StubWithCallback(Read2Cb);
    DATA_Read2DataBlocks_StubWithCallback(Read2Cb);
    DATA_DatabaseEntryUpdatedAtLeastOnce_StubWithCallback(DatabaseEntryUpdatedAtLeastOnceCb);
    test_write4Calls = 0u;
    DATA_Write4DataBlocks_StubWithCallback(Write4Cb);

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
    TEST_ASSERT_EQUAL_UINT8(1u, test_write4Calls);
    /* verified inside the stubs, final payload verified in Write4Cb */
}
