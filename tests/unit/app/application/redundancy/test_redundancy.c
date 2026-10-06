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
 * @file    test_redundancy.c
 * @author  foxBMS Team
 * @date    2020-07-31 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the redundancy module
 * @details Tests all externalized functions of the redundancy module.
 *
 *          Mocking strategy (mirrors the bms-values test approach):
 *          - Collaborators are replaced by CMock *_StubWithCallback callbacks
 *            (no *_Ignore / *_ExpectAndReturn on structs).
 *          - The two header-by-value helpers
 *            (DATA_DatabaseEntryUpdatedAtLeastOnce and
 *            DATA_EntryUpdatedWithinInterval) are steered via the header
 *            uniqueId, so base/redundant sources can be distinguished robustly.
 *          - The internal redundancy state (mrc_state) is primed with
 *            TEST_MRC_SetMrcState() so the "updated since last MRC" comparisons
 *            are deterministic and reset in setUp().
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockbms.h"
#include "Mockdatabase.h"
#include "Mockdatabase_helper.h"
#include "Mockdiag.h"
#include "Mockos.h"
#include "Mockredundancy-validation.h"

#include "battery_system_cfg.h"
#include "database_cfg.h"
#include "diag_cfg.h"
#include "redundancy_cfg.h"

#include "foxmath.h"
#include "redundancy.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/

/*========== Definitions and Implementations for Unit Test ==================*/
#define MRC_NR_OF_CELLS_PER_STRING   (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE)
#define MRC_NR_OF_SENSORS_PER_STRING (BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE)

/* --- control values for the MCR_* plausibility stubs --------------------- */
static STD_RETURN_TYPE_e test_mcrVoltageReturn     = STD_OK;
static int16_t test_mcrValidatedVoltage_mV         = 0;
static STD_RETURN_TYPE_e test_mcrTemperatureReturn = STD_OK;
static int16_t test_mcrValidatedTemperature_ddegC  = 0;

/* --- control values for DATA_DatabaseEntryUpdatedAtLeastOnce (redundancy use) */
static bool test_useVoltageRedundancy     = false;
static bool test_useTemperatureRedundancy = false;

/* --- control values for DATA_EntryUpdatedWithinInterval (per source) ------ */
static bool test_baseVoltageWithinInterval           = true;
static bool test_redundancyVoltageWithinInterval     = true;
static bool test_baseTemperatureWithinInterval       = true;
static bool test_redundancyTemperatureWithinInterval = true;

/*========== Unit Test Helper Functions =====================================*/
/* MCR_CheckCellVoltage: writes the validated voltage on success and returns the
 * configured result. */
static STD_RETURN_TYPE_e McrCheckCellVoltageCb(
    int16_t baseCellVoltage,
    int16_t redundancy0CellVoltage,
    int16_t *pCellVoltage,
    int cmock_num_calls) {
    (void)baseCellVoltage;
    (void)redundancy0CellVoltage;
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pCellVoltage);
    if (test_mcrVoltageReturn == STD_OK) {
        *pCellVoltage = test_mcrValidatedVoltage_mV;
    }
    return test_mcrVoltageReturn;
}

/* MCR_CheckCellTemperature: mirror of the voltage stub for temperatures. */
static STD_RETURN_TYPE_e McrCheckCellTemperatureCb(
    int16_t baseCellTemperature,
    int16_t redundancy0CellTemperature,
    int16_t *pCellTemperature,
    int cmock_num_calls) {
    (void)baseCellTemperature;
    (void)redundancy0CellTemperature;
    (void)cmock_num_calls;
    TEST_ASSERT_NOT_NULL(pCellTemperature);
    if (test_mcrTemperatureReturn == STD_OK) {
        *pCellTemperature = test_mcrValidatedTemperature_ddegC;
    }
    return test_mcrTemperatureReturn;
}

/* DIAG_ReportResultToHandler: only argument wiring is relevant, always report OK. */
static STD_RETURN_TYPE_e DiagCheckEventCb(
    STD_RETURN_TYPE_e cond,
    DIAG_ID_e diagId,
    DIAG_IMPACT_LEVEL_e impact,
    uint32_t impactNumber,
    int cmock_num_calls) {
    (void)cond;
    (void)diagId;
    (void)impact;
    (void)impactNumber;
    (void)cmock_num_calls;
    return STD_OK;
}

/* DIAG_Handler: measurement timeout bookkeeping, always report OK. */
static DIAG_RETURNTYPE_e DiagHandlerCb(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    DIAG_IMPACT_LEVEL_e impact,
    uint32_t impactNumber,
    int cmock_num_calls) {
    (void)diagId;
    (void)event;
    (void)impact;
    (void)impactNumber;
    (void)cmock_num_calls;
    return DIAG_HANDLER_RETURN_OK;
}

/* DATA_DatabaseEntryUpdatedAtLeastOnce: decides "use redundancy" per source
 * based on the header uniqueId. */
static bool DatabaseEntryUpdatedAtLeastOnceCb(DATA_BLOCK_HEADER_s dataBlockHeader, int cmock_num_calls) {
    (void)cmock_num_calls;
    bool retval = false;
    switch (dataBlockHeader.uniqueId) {
        case DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0:
            retval = test_useVoltageRedundancy;
            break;
        case DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0:
            retval = test_useTemperatureRedundancy;
            break;
        default:
            TEST_FAIL_MESSAGE("Unexpected uniqueId in DATA_DatabaseEntryUpdatedAtLeastOnce");
            break;
    }
    return retval;
}

/* DATA_EntryUpdatedWithinInterval: returns the configured "within interval"
 * result per source based on the header uniqueId. */
static bool EntryUpdatedWithinIntervalCb(
    DATA_BLOCK_HEADER_s dataBlockHeader,
    uint32_t timeInterval,
    int cmock_num_calls) {
    (void)timeInterval;
    (void)cmock_num_calls;
    bool retval = false;
    switch (dataBlockHeader.uniqueId) {
        case DATA_BLOCK_ID_CELL_VOLTAGE_BASE:
            retval = test_baseVoltageWithinInterval;
            break;
        case DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0:
            retval = test_redundancyVoltageWithinInterval;
            break;
        case DATA_BLOCK_ID_CELL_TEMPERATURE_BASE:
            retval = test_baseTemperatureWithinInterval;
            break;
        case DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0:
            retval = test_redundancyTemperatureWithinInterval;
            break;
        default:
            TEST_FAIL_MESSAGE("Unexpected uniqueId in DATA_EntryUpdatedWithinInterval");
            break;
    }
    return retval;
}

/* Installs the stubs needed by the measurement-level functions. */
static void installMeasurementStubs(void) {
    DATA_DatabaseEntryUpdatedAtLeastOnce_StubWithCallback(DatabaseEntryUpdatedAtLeastOnceCb);
    DATA_EntryUpdatedWithinInterval_StubWithCallback(EntryUpdatedWithinIntervalCb);
    DIAG_Handler_StubWithCallback(DiagHandlerCb);
    MCR_CheckCellVoltage_StubWithCallback(McrCheckCellVoltageCb);
    MCR_CheckCellTemperature_StubWithCallback(McrCheckCellTemperatureCb);
    DIAG_ReportResultToHandler_StubWithCallback(DiagCheckEventCb);
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* Reset internal redundancy state so "updated since last MRC" is deterministic. */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);

    test_mcrVoltageReturn              = STD_OK;
    test_mcrValidatedVoltage_mV        = 0;
    test_mcrTemperatureReturn          = STD_OK;
    test_mcrValidatedTemperature_ddegC = 0;

    test_useVoltageRedundancy     = false;
    test_useTemperatureRedundancy = false;

    test_baseVoltageWithinInterval           = true;
    test_redundancyVoltageWithinInterval     = true;
    test_baseTemperatureWithinInterval       = true;
    test_redundancyTemperatureWithinInterval = true;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/**
 * @brief   Test static function #MRC_ValidateCellVoltage
 * @details Argument validation:
 *            - AT1/3: NULL_PTR for pCellVoltageBase        -> assert
 *            - AT2/3: NULL_PTR for pCellVoltageRedundancy0 -> assert
 *            - AT3/3: NULL_PTR for pValidatedVoltages      -> assert
 *          Routine validation:
 *            - RT1/5: base+redundant valid, plausible   -> STD_OK, value validated
 *            - RT2/5: base+redundant valid, implausible -> STD_NOT_OK, invalid flag
 *            - RT3/5: only base valid                   -> STD_OK, base value used
 *            - RT4/5: only redundant valid              -> STD_OK, redundant value used
 *            - RT5/5: both invalid                      -> STD_NOT_OK, average stored
 */
void testMRC_ValidateCellVoltage(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_VOLTAGE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltage(NULL_PTR, &dummy, &dummy));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltage(&dummy, NULL_PTR, &dummy));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltage(&dummy, &dummy, NULL_PTR));

    /* ======= Routine tests =============================================== */
    MCR_CheckCellVoltage_StubWithCallback(McrCheckCellVoltageCb);
    DIAG_ReportResultToHandler_StubWithCallback(DiagCheckEventCb);

    DATA_BLOCK_CELL_VOLTAGE_s base = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    DATA_BLOCK_CELL_VOLTAGE_s red  = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
    DATA_BLOCK_CELL_VOLTAGE_s val  = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};

    /* ======= RT1/5: both valid, plausible ======= */
    test_mcrVoltageReturn       = STD_OK;
    test_mcrValidatedVoltage_mV = 3000;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                base.invalidCellVoltage[s][m][cb] = false;
                red.invalidCellVoltage[s][m][cb]  = false;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_ValidateCellVoltage(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(MRC_NR_OF_CELLS_PER_STRING, val.nrValidCellVoltages[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_FALSE(val.invalidCellVoltage[s][m][cb]);
                TEST_ASSERT_EQUAL_INT16(3000, val.cellVoltage_mV[s][m][cb]);
            }
        }
    }

    /* ======= RT2/5: both valid, implausible ======= */
    test_mcrVoltageReturn = STD_NOT_OK;
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_MRC_ValidateCellVoltage(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(0u, val.nrValidCellVoltages[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_TRUE(val.invalidCellVoltage[s][m][cb]);
            }
        }
    }

    /* ======= RT3/5: only base valid ======= */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                base.invalidCellVoltage[s][m][cb] = false;
                red.invalidCellVoltage[s][m][cb]  = true;
                base.cellVoltage_mV[s][m][cb]     = 3300;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_ValidateCellVoltage(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(MRC_NR_OF_CELLS_PER_STRING, val.nrValidCellVoltages[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_FALSE(val.invalidCellVoltage[s][m][cb]);
                TEST_ASSERT_EQUAL_INT16(3300, val.cellVoltage_mV[s][m][cb]);
            }
        }
    }

    /* ======= RT4/5: only redundant valid ======= */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                base.invalidCellVoltage[s][m][cb] = true;
                red.invalidCellVoltage[s][m][cb]  = false;
                red.cellVoltage_mV[s][m][cb]      = 3100;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_ValidateCellVoltage(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(MRC_NR_OF_CELLS_PER_STRING, val.nrValidCellVoltages[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_FALSE(val.invalidCellVoltage[s][m][cb]);
                TEST_ASSERT_EQUAL_INT16(3100, val.cellVoltage_mV[s][m][cb]);
            }
        }
    }

    /* ======= RT5/5: both invalid -> average stored ======= */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                base.invalidCellVoltage[s][m][cb] = true;
                red.invalidCellVoltage[s][m][cb]  = true;
                base.cellVoltage_mV[s][m][cb]     = 3000;
                red.cellVoltage_mV[s][m][cb]      = 3200;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_MRC_ValidateCellVoltage(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(0u, val.nrValidCellVoltages[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                TEST_ASSERT_TRUE(val.invalidCellVoltage[s][m][cb]);
                TEST_ASSERT_EQUAL_INT16((3000 + 3200) / 2, val.cellVoltage_mV[s][m][cb]);
            }
        }
    }
}

/**
 * @brief   Test static function #MRC_ValidateCellTemperature
 * @details Argument validation:
 *            - AT1/3..AT3/3: NULL_PTR for each argument -> assert
 *          Routine validation:
 *            - RT1/5: both valid, plausible   -> STD_OK, value validated
 *            - RT2/5: both valid, implausible -> STD_NOT_OK, invalid flag
 *            - RT3/5: only base valid         -> STD_OK, base value used
 *            - RT4/5: only redundant valid    -> STD_OK, redundant value used
 *            - RT5/5: both invalid            -> STD_NOT_OK, average stored
 */
void testMRC_ValidateCellTemperature(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_TEMPERATURE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperature(NULL_PTR, &dummy, &dummy));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperature(&dummy, NULL_PTR, &dummy));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperature(&dummy, &dummy, NULL_PTR));

    /* ======= Routine tests =============================================== */
    MCR_CheckCellTemperature_StubWithCallback(McrCheckCellTemperatureCb);
    DIAG_ReportResultToHandler_StubWithCallback(DiagCheckEventCb);

    DATA_BLOCK_CELL_TEMPERATURE_s base = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    DATA_BLOCK_CELL_TEMPERATURE_s red  = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
    DATA_BLOCK_CELL_TEMPERATURE_s val  = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};

    /* ======= RT1/5: both valid, plausible ======= */
    test_mcrTemperatureReturn          = STD_OK;
    test_mcrValidatedTemperature_ddegC = 250;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                base.invalidCellTemperature[s][m][ts] = false;
                red.invalidCellTemperature[s][m][ts]  = false;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_ValidateCellTemperature(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(MRC_NR_OF_SENSORS_PER_STRING, val.nrValidTemperatures[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_FALSE(val.invalidCellTemperature[s][m][ts]);
                TEST_ASSERT_EQUAL_INT16(250, val.cellTemperature_ddegC[s][m][ts]);
            }
        }
    }

    /* ======= RT2/5: both valid, implausible ======= */
    test_mcrTemperatureReturn = STD_NOT_OK;
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_MRC_ValidateCellTemperature(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(0u, val.nrValidTemperatures[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_TRUE(val.invalidCellTemperature[s][m][ts]);
            }
        }
    }

    /* ======= RT3/5: only base valid ======= */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                base.invalidCellTemperature[s][m][ts] = false;
                red.invalidCellTemperature[s][m][ts]  = true;
                base.cellTemperature_ddegC[s][m][ts]  = 230;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_ValidateCellTemperature(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(MRC_NR_OF_SENSORS_PER_STRING, val.nrValidTemperatures[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_FALSE(val.invalidCellTemperature[s][m][ts]);
                TEST_ASSERT_EQUAL_INT16(230, val.cellTemperature_ddegC[s][m][ts]);
            }
        }
    }

    /* ======= RT4/5: only redundant valid ======= */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                base.invalidCellTemperature[s][m][ts] = true;
                red.invalidCellTemperature[s][m][ts]  = false;
                red.cellTemperature_ddegC[s][m][ts]   = 245;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_ValidateCellTemperature(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(MRC_NR_OF_SENSORS_PER_STRING, val.nrValidTemperatures[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_FALSE(val.invalidCellTemperature[s][m][ts]);
                TEST_ASSERT_EQUAL_INT16(245, val.cellTemperature_ddegC[s][m][ts]);
            }
        }
    }

    /* ======= RT5/5: both invalid -> average stored ======= */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                base.invalidCellTemperature[s][m][ts] = true;
                red.invalidCellTemperature[s][m][ts]  = true;
                base.cellTemperature_ddegC[s][m][ts]  = 200;
                red.cellTemperature_ddegC[s][m][ts]   = 260;
            }
        }
    }
    TEST_ASSERT_EQUAL(STD_NOT_OK, TEST_MRC_ValidateCellTemperature(&base, &red, &val));
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        TEST_ASSERT_EQUAL_UINT16(0u, val.nrValidTemperatures[s]);
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                TEST_ASSERT_TRUE(val.invalidCellTemperature[s][m][ts]);
                TEST_ASSERT_EQUAL_INT16((200 + 260) / 2, val.cellTemperature_ddegC[s][m][ts]);
            }
        }
    }
}

/**
 * @brief   Test static function #MRC_UpdateCellVoltageValidation
 * @details Argument validation:
 *            - AT1/2: NULL_PTR for pCellVoltage       -> assert
 *            - AT2/2: NULL_PTR for pValidatedVoltages -> assert
 *          Routine validation:
 *            - RT1/1: payload copied, destination header preserved -> STD_OK
 */
void testMRC_UpdateCellVoltageValidation(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_VOLTAGE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellVoltageValidation(NULL_PTR, &dummy));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellVoltageValidation(&dummy, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    DATA_BLOCK_CELL_VOLTAGE_s src   = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    DATA_BLOCK_CELL_VOLTAGE_s dst   = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    src.cellVoltage_mV[0][0][0]     = 3333;
    src.invalidCellVoltage[0][0][0] = true;
    src.header.timestamp            = 42u; /* must NOT be copied */
    dst.header.timestamp            = 100u;

    /* ======= RT1/1: call function under test */
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_UpdateCellVoltageValidation(&src, &dst));

    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL_INT16(3333, dst.cellVoltage_mV[0][0][0]);
    TEST_ASSERT_TRUE(dst.invalidCellVoltage[0][0][0]);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE, dst.header.uniqueId);
    TEST_ASSERT_EQUAL_UINT32(100u, dst.header.timestamp);
}

/**
 * @brief   Test static function #MRC_UpdateCellTemperatureValidation
 * @details Argument validation:
 *            - AT1/2: NULL_PTR for pCellTemperature       -> assert
 *            - AT2/2: NULL_PTR for pValidatedTemperature  -> assert
 *          Routine validation:
 *            - RT1/1: payload copied, destination header preserved -> STD_OK
 */
void testMRC_UpdateCellTemperatureValidation(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_TEMPERATURE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellTemperatureValidation(NULL_PTR, &dummy));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_UpdateCellTemperatureValidation(&dummy, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    DATA_BLOCK_CELL_TEMPERATURE_s src   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    DATA_BLOCK_CELL_TEMPERATURE_s dst   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    src.cellTemperature_ddegC[0][0][0]  = 275;
    src.invalidCellTemperature[0][0][0] = true;
    src.header.timestamp                = 42u; /* must NOT be copied */
    dst.header.timestamp                = 100u;

    /* ======= RT1/1: call function under test */
    TEST_ASSERT_EQUAL(STD_OK, TEST_MRC_UpdateCellTemperatureValidation(&src, &dst));

    /* ======= RT1/1: test output verification */
    TEST_ASSERT_EQUAL_INT16(275, dst.cellTemperature_ddegC[0][0][0]);
    TEST_ASSERT_TRUE(dst.invalidCellTemperature[0][0][0]);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE, dst.header.uniqueId);
    TEST_ASSERT_EQUAL_UINT32(100u, dst.header.timestamp);
}

/**
 * @brief   Test static function MRC_ValidateCellVoltageMeasurement
 * @details Argument validation:
 *            - AT1/3..AT3/3: NULL_PTR for each argument -> assert
 *          Routine validation (branches):
 *            - RT1/6: redundancy never updated, base updated      -> false
 *            - RT2/6: no redundancy, base not updated             -> false
 *            - RT3/6: redundancy, both updated                    -> true (validate)
 *            - RT4/6: redundancy, only base updated + red timeout -> true (copy base)
 *            - RT5/6: redundancy, only red updated + base timeout -> true (copy redundant)
 *            - RT6/6: redundancy, nothing updated                 -> false
 */
void testMRC_ValidateCellVoltageMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_VOLTAGE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltageMeasurement(NULL_PTR, &dummy, &dummy));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltageMeasurement(&dummy, NULL_PTR, &dummy));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellVoltageMeasurement(&dummy, &dummy, NULL_PTR));

    /* ======= Routine tests =============================================== */
    installMeasurementStubs();

    DATA_BLOCK_CELL_VOLTAGE_s val;
    DATA_BLOCK_CELL_VOLTAGE_s base;
    DATA_BLOCK_CELL_VOLTAGE_s red;

    /* ======= RT1/6: redundancy never updated, base updated ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useVoltageRedundancy   = false;
    val                         = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    val.cellVoltage_mV[0][0][0] = 1234;
    base                        = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    red                   = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
    base.header.timestamp = 10u; /* differs from mrc_state (0) -> base updated */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                base.cellVoltage_mV[s][m][cb]     = 3300;
                base.invalidCellVoltage[s][m][cb] = false;
            }
        }
    }
    TEST_ASSERT_FALSE(TEST_MRC_ValidateCellVoltageMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(1234, val.cellVoltage_mV[0][0][0]);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE, val.header.uniqueId);

    /* ======= RT2/6: no redundancy, base not updated ======= */
    TEST_MRC_SetMrcState(10u, 0u, 0u, 0u);
    test_useVoltageRedundancy = false;
    val                       = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    base                      = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    red                       = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
    base.header.timestamp     = 10u; /* equals mrc_state -> not updated */
    TEST_ASSERT_FALSE(TEST_MRC_ValidateCellVoltageMeasurement(&val, &base, &red));

    /* ======= RT3/6: redundancy, both updated ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useVoltageRedundancy   = true;
    test_mcrVoltageReturn       = STD_OK;
    test_mcrValidatedVoltage_mV = 3210;
    val                         = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    base                        = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    red                   = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
    base.header.timestamp = 10u;
    red.header.timestamp  = 20u;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                base.invalidCellVoltage[s][m][cb] = false;
                red.invalidCellVoltage[s][m][cb]  = false;
            }
        }
    }
    TEST_ASSERT_TRUE(TEST_MRC_ValidateCellVoltageMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(3210, val.cellVoltage_mV[0][0][0]);

    /* ======= RT4/6: redundancy, only base updated + redundancy timeout ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useVoltageRedundancy            = true;
    test_redundancyVoltageWithinInterval = false; /* redundancy timeout reached */
    val                                  = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    base                  = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    red                   = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
    base.header.timestamp = 10u; /* updated */
    red.header.timestamp  = 0u;  /* not updated (== mrc_state) */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                base.cellVoltage_mV[s][m][cb]     = 3400;
                base.invalidCellVoltage[s][m][cb] = false;
            }
        }
    }
    TEST_ASSERT_TRUE(TEST_MRC_ValidateCellVoltageMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(3400, val.cellVoltage_mV[0][0][0]); /* base copied */

    /* ======= RT5/6: redundancy, only redundant updated + base timeout ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useVoltageRedundancy            = true;
    test_baseVoltageWithinInterval       = false; /* base timeout reached */
    test_redundancyVoltageWithinInterval = true;
    val                                  = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    base                  = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    red                   = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
    base.header.timestamp = 0u;  /* not updated */
    red.header.timestamp  = 20u; /* updated */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                red.cellVoltage_mV[s][m][cb]     = 3150;
                red.invalidCellVoltage[s][m][cb] = false;
            }
        }
    }
    TEST_ASSERT_TRUE(TEST_MRC_ValidateCellVoltageMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(3150, val.cellVoltage_mV[0][0][0]); /* redundant copied */

    /* ======= RT6/6: redundancy, nothing updated ======= */
    TEST_MRC_SetMrcState(10u, 20u, 0u, 0u);
    test_useVoltageRedundancy = true;
    val                       = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    base                      = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    red                       = (DATA_BLOCK_CELL_VOLTAGE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};
    base.header.timestamp     = 10u; /* == mrc_state -> not updated */
    red.header.timestamp      = 20u; /* == mrc_state -> not updated */
    TEST_ASSERT_FALSE(TEST_MRC_ValidateCellVoltageMeasurement(&val, &base, &red));
}

/**
 * @brief   Test static function #MRC_ValidateCellTemperatureMeasurement
 * @details Argument validation:
 *            - AT1/3..AT3/3: NULL_PTR for each argument -> assert
 *          Routine validation (branches):
 *            - RT1/6: redundancy never updated, base updated      -> false
 *            - RT2/6: no redundancy, base not updated             -> false
 *            - RT3/6: redundancy, both updated                    -> true (validate)
 *            - RT4/6: redundancy, only base updated + red timeout -> true (copy base)
 *            - RT5/6: redundancy, only red updated + base timeout -> true (copy redundant)
 *            - RT6/6: redundancy, nothing updated                 -> false
 */
void testMRC_ValidateCellTemperatureMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_TEMPERATURE_s dummy = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperatureMeasurement(NULL_PTR, &dummy, &dummy));
    /* ======= AT2/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperatureMeasurement(&dummy, NULL_PTR, &dummy));
    /* ======= AT3/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_MRC_ValidateCellTemperatureMeasurement(&dummy, &dummy, NULL_PTR));

    /* ======= Routine tests =============================================== */
    installMeasurementStubs();

    DATA_BLOCK_CELL_TEMPERATURE_s val;
    DATA_BLOCK_CELL_TEMPERATURE_s base;
    DATA_BLOCK_CELL_TEMPERATURE_s red;

    /* ======= RT1/6: redundancy never updated, base updated ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useTemperatureRedundancy = false;
    val                           = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    val.cellTemperature_ddegC[0][0][0] = 123;
    base = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    red  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
    base.header.timestamp = 10u;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                base.cellTemperature_ddegC[s][m][ts]  = 230;
                base.invalidCellTemperature[s][m][ts] = false;
            }
        }
    }
    TEST_ASSERT_FALSE(TEST_MRC_ValidateCellTemperatureMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(123, val.cellTemperature_ddegC[0][0][0]);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE, val.header.uniqueId);

    /* ======= RT2/6: no redundancy, base not updated ======= */
    TEST_MRC_SetMrcState(0u, 0u, 10u, 0u);
    test_useTemperatureRedundancy = false;
    val                           = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    base = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    red  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
    base.header.timestamp = 10u; /* == mrc_state -> not updated */
    TEST_ASSERT_FALSE(TEST_MRC_ValidateCellTemperatureMeasurement(&val, &base, &red));

    /* ======= RT3/6: redundancy, both updated ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useTemperatureRedundancy      = true;
    test_mcrTemperatureReturn          = STD_OK;
    test_mcrValidatedTemperature_ddegC = 248;
    val  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    base = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    red  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
    base.header.timestamp = 10u;
    red.header.timestamp  = 20u;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                base.invalidCellTemperature[s][m][ts] = false;
                red.invalidCellTemperature[s][m][ts]  = false;
            }
        }
    }
    TEST_ASSERT_TRUE(TEST_MRC_ValidateCellTemperatureMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(248, val.cellTemperature_ddegC[0][0][0]);

    /* ======= RT4/6: redundancy, only base updated + redundancy timeout ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useTemperatureRedundancy            = true;
    test_redundancyTemperatureWithinInterval = false;
    val  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    base = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    red  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
    base.header.timestamp = 10u;
    red.header.timestamp  = 0u;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                base.cellTemperature_ddegC[s][m][ts]  = 235;
                base.invalidCellTemperature[s][m][ts] = false;
            }
        }
    }
    TEST_ASSERT_TRUE(TEST_MRC_ValidateCellTemperatureMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(235, val.cellTemperature_ddegC[0][0][0]);

    /* ======= RT5/6: redundancy, only redundant updated + base timeout ======= */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useTemperatureRedundancy            = true;
    test_baseTemperatureWithinInterval       = false;
    test_redundancyTemperatureWithinInterval = true;
    val  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    base = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    red  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
    base.header.timestamp = 0u;
    red.header.timestamp  = 20u;
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                red.cellTemperature_ddegC[s][m][ts]  = 242;
                red.invalidCellTemperature[s][m][ts] = false;
            }
        }
    }
    TEST_ASSERT_TRUE(TEST_MRC_ValidateCellTemperatureMeasurement(&val, &base, &red));
    TEST_ASSERT_EQUAL_INT16(242, val.cellTemperature_ddegC[0][0][0]);

    /* ======= RT6/6: redundancy, nothing updated ======= */
    TEST_MRC_SetMrcState(0u, 0u, 10u, 20u);
    test_useTemperatureRedundancy = true;
    val                           = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    base = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
    red  = (DATA_BLOCK_CELL_TEMPERATURE_s){.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0};
    base.header.timestamp = 10u;
    red.header.timestamp  = 20u;
    TEST_ASSERT_FALSE(TEST_MRC_ValidateCellTemperatureMeasurement(&val, &base, &red));
}

/*========== Extern function test ==========================================*/
/* DATA_Read4DataBlocks stub for MRC_ValidateAfeMeasurement: sets fresh base
 * timestamps while redundant timestamps remain zero and verifies the
 * requested block IDs. */
static STD_RETURN_TYPE_e ValidateAfeRead4Cb(void *p0, void *p1, void *p2, void *p3, int cmock_num_calls) {
    (void)cmock_num_calls;
    DATA_BLOCK_CELL_VOLTAGE_s *vBase     = (DATA_BLOCK_CELL_VOLTAGE_s *)p0;
    DATA_BLOCK_CELL_VOLTAGE_s *vRed      = (DATA_BLOCK_CELL_VOLTAGE_s *)p1;
    DATA_BLOCK_CELL_TEMPERATURE_s *tBase = (DATA_BLOCK_CELL_TEMPERATURE_s *)p2;
    DATA_BLOCK_CELL_TEMPERATURE_s *tRed  = (DATA_BLOCK_CELL_TEMPERATURE_s *)p3;

    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE_BASE, vBase->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0, vRed->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE_BASE, tBase->header.uniqueId);
    TEST_ASSERT_EQUAL(DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0, tRed->header.uniqueId);

    /* Fresh base timestamps (mrc_state primed to 0) -> "updated since last MRC" */
    vBase->header.timestamp = 10u;
    tBase->header.timestamp = 10u;
    /* redundancy timestamps left at 0 -> DATA_DatabaseEntryUpdatedAtLeastOnce=false */

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                vBase->cellVoltage_mV[s][m][cb]     = 3300;
                vBase->invalidCellVoltage[s][m][cb] = false;
            }
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                tBase->cellTemperature_ddegC[s][m][ts]  = 240;
                tBase->invalidCellTemperature[s][m][ts] = false;
            }
        }
    }
    return STD_OK;
}

/**
 * @brief   Test extern function #MRC_ValidateAfeMeasurement
 * @details Argument validation:
 *            - AT1/2: NULL_PTR for pTableCellVoltages     -> assert
 *            - AT2/2: NULL_PTR for pTableCellTemperatures -> assert
 *          Routine validation:
 *            - RT1/1: redundancy never updated, both base measurements updated
 *                     -> neither database entry needs updating
 */
void testMRC_ValidateAfeMeasurement(void) {
    /* ======= Assertion tests ============================================= */
    DATA_BLOCK_CELL_VOLTAGE_s dummyV     = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    DATA_BLOCK_CELL_TEMPERATURE_s dummyT = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(MRC_ValidateAfeMeasurement(NULL_PTR, &dummyT));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(MRC_ValidateAfeMeasurement(&dummyV, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    TEST_MRC_SetMrcState(0u, 0u, 0u, 0u);
    test_useVoltageRedundancy     = false;
    test_useTemperatureRedundancy = false;

    installMeasurementStubs();
    DATA_Read4DataBlocks_StubWithCallback(ValidateAfeRead4Cb);

    DATA_BLOCK_CELL_VOLTAGE_s val       = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    DATA_BLOCK_CELL_TEMPERATURE_s valT  = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
    val.cellVoltage_mV[0][0][0]         = 1234;
    valT.cellTemperature_ddegC[0][0][0] = 123;

    /* ======= RT1/1: call function under test */
    MRC_REQUIRED_UPDATES_s result = MRC_ValidateAfeMeasurement(&val, &valT);

    /* ======= RT1/1: test output verification */
    TEST_ASSERT_FALSE(result.updatedCellVoltageDatabaseEntry);
    TEST_ASSERT_FALSE(result.updatedTemperatureDatabaseEntry);
    TEST_ASSERT_EQUAL_INT16(1234, val.cellVoltage_mV[0][0][0]);
    TEST_ASSERT_EQUAL_INT16(123, valT.cellTemperature_ddegC[0][0][0]);
}
