/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_can_cbs_rx_debug.c
 * @author  foxBMS Team
 * @date    2021-04-22 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the CAN driver callbacks
 * @details Tests for the rx-debug CAN driver and helper functions
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe.h"
#include "Mockalgorithm.h"
#include "Mockbal.h"
#include "Mockbms.h"
#include "Mockcan.h"
#include "Mockcan_cbs_tx_debug-build-configuration.h"
#include "Mockcan_cbs_tx_debug-response.h"
#include "Mockcan_cbs_tx_debug-unsupported-multiplexer-values.h"
#include "Mockcontactor.h"
#include "Mockdatabase.h"
#include "Mockdiag.h"
#include "Mockfoxmath.h"
#include "Mockfram.h"
#include "Mockimd.h"
#include "Mockinterlock.h"
#include "Mockmcu.h"
#include "Mockmeas.h"
#include "Mockos.h"
#include "Mockreset.h"
#include "Mockrtc.h"
#include "Mocksbc.h"
#include "Mocksof_trapezoid.h"
#include "Mockstate_estimation.h"
#include "Mocksys_cfg.h"

#include "database_cfg.h"
#include "fram_cfg.h"

#include "can_cbs_rx.h"
#include "can_cfg_rx-message-definitions.h"
#include "can_helper.h"
#include "sys.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("can_cbs_rx_debug.c")

TEST_INCLUDE_PATH("../../src/app/application/algorithm")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/config")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation")
TEST_INCLUDE_PATH("../../src/app/application/algorithm/state_estimation/sof/trapezoid")
TEST_INCLUDE_PATH("../../src/app/application/bal")
TEST_INCLUDE_PATH("../../src/app/application/bms")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/can")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/rx")
TEST_INCLUDE_PATH("../../src/app/driver/can/cbs/tx-async")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/i2c")
TEST_INCLUDE_PATH("../../src/app/driver/imd")
TEST_INCLUDE_PATH("../../src/app/driver/interlock")
TEST_INCLUDE_PATH("../../src/app/driver/mcu")
TEST_INCLUDE_PATH("../../src/app/driver/meas")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/sbc")
TEST_INCLUDE_PATH("../../src/app/driver/sbc/fs8x_driver")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/sps")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/engine/sys")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

#define MULTIPLEXER_VALUE_VERSION_INFORMATION (0u)
#define MULTIPLEXER_VALUE_RTC                 (1u)
#define MULTIPLEXER_VALUE_SOFTWARE_RESET      (2u)
#define MULTIPLEXER_VALUE_FRAM_INITIALIZATION (3u)
#define MULTIPLEXER_VALUE_TIME_INFO           (4u)
#define MULTIPLEXER_VALUE_UPTIME_INFO         (5u)
#define INVALID_MULTIPLEXER_VALUE             (99u)

#define SYS_STATE_VALID_CANRX_RETURN_VALUE   (0u)
#define SYS_STATE_INVALID_CANRX_RETURN_VALUE (1u)

/* Needed so that the test can be compiled */
SBC_STATE_s sbc_stateMcuSupervisor = {
    .timer                   = 0u,
    .stateRequest            = SBC_STATE_NO_REQUEST,
    .state                   = SBC_STATEMACHINE_UNINITIALIZED,
    .substate                = SBC_ENTRY,
    .lastState               = SBC_STATEMACHINE_UNINITIALIZED,
    .lastSubstate            = SBC_ENTRY,
    .illegalRequestsCounter  = 0u,
    .retryCounter            = 0u,
    .requestWatchdogTrigger  = 0u,
    .triggerEntry            = 0u,
    .pFs85xxInstance         = NULL_PTR,
    .watchdogState           = SBC_PERIODIC_WATCHDOG_DEACTIVATED,
    .watchdogPeriod_10ms     = 10u,
    .useIgnitionForPowerDown = true,
};

CAN_MESSAGE_PROPERTIES_s validRxDebugTestMessage = {
    .id         = CANRX_DEBUG_ID,
    .idType     = CANRX_DEBUG_ID_TYPE,
    .dlc        = CAN_DEFAULT_DLC,
    .endianness = CANRX_DEBUG_ENDIANNESS,
};

/* dummy data array filled with zero */
const uint8_t testCanDataZeroArray[CAN_MAX_DLC] = {0};
const uint64_t testMessageDataZero              = 0u;
const CAN_ENDIANNESS_e invalidEndianness        = CAN_LITTLE_ENDIAN;
const CAN_ENDIANNESS_e validEndianness          = CAN_BIG_ENDIAN;

static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages        = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_CELL_TEMPERATURE_s can_tableTemperatures    = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor     = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATE_REQUEST_s can_tableStateRequest       = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};
static DATA_BLOCK_PACK_VALUES_s can_tablePackValues           = {.header.uniqueId = DATA_BLOCK_ID_PACK_VALUES};
static DATA_BLOCK_SOF_s can_tableSof                          = {.header.uniqueId = DATA_BLOCK_ID_SOF};
static DATA_BLOCK_SOC_s can_tableSoc                          = {.header.uniqueId = DATA_BLOCK_ID_SOC};
static DATA_BLOCK_SOE_s can_tableSoe                          = {.header.uniqueId = DATA_BLOCK_ID_SOE};
static DATA_BLOCK_ERROR_STATE_s can_tableErrorState           = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
static DATA_BLOCK_INSULATION_MONITORING_s can_tableInsulation = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};
static DATA_BLOCK_MSL_FLAG_s can_tableMslFlags            = {.header.uniqueId = DATA_BLOCK_ID_MSL_FLAG};
static DATA_BLOCK_RSL_FLAG_s can_tableRslFlags            = {.header.uniqueId = DATA_BLOCK_ID_RSL_FLAG};
static DATA_BLOCK_MOL_FLAG_s can_tableMolFlags            = {.header.uniqueId = DATA_BLOCK_ID_MOL_FLAG};
static DATA_BLOCK_AEROSOL_SENSOR_s can_tableAerosolSensor = {.header.uniqueId = DATA_BLOCK_ID_AEROSOL_SENSOR};

OS_QUEUE imd_canDataQueue     = NULL_PTR;
OS_QUEUE ftsk_rtcSetTimeQueue = NULL_PTR;

static SYS_FSM_STATES_e stored_sys_state;

const CAN_SHIM_s can_kShim = {
    .pQueueImd             = &imd_canDataQueue,
    .pTableCellVoltage     = &can_tableCellVoltages,
    .pTableCellTemperature = &can_tableTemperatures,
    .pTableMinMax          = &can_tableMinimumMaximumValues,
    .pTableCurrentSensor   = &can_tableCurrentSensor,
    .pTableOpenWire        = &can_tableOpenWire,
    .pTableStateRequest    = &can_tableStateRequest,
    .pTablePackValues      = &can_tablePackValues,
    .pTableSof             = &can_tableSof,
    .pTableSoc             = &can_tableSoc,
    .pTableSoe             = &can_tableSoe,
    .pTableErrorState      = &can_tableErrorState,
    .pTableInsulation      = &can_tableInsulation,
    .pTableMsl             = &can_tableMslFlags,
    .pTableRsl             = &can_tableRslFlags,
    .pTableMol             = &can_tableMolFlags,
    .pTableAerosolSensor   = &can_tableAerosolSensor,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    stored_sys_state       = sys_state.currentState;
    sys_state.currentState = SYS_FSM_SUBSTATE_INITIALIZATION_CAN + 1;
}

void tearDown(void) {
    sys_state.currentState = stored_sys_state;
}

/*========== Test Cases =====================================================*/

/*********************************************************************************************************************/
/* Assertion tests of the public interface of the module, i.e., 'CANRX_Debug' */
void testCANRX_DebugAssertionsParamMessage(void) {
    /* testing assertions, therefore only dummy CAN data required in this test */
    /* test assert for wrong message ID */
    CAN_MESSAGE_PROPERTIES_s testMessage = {
        .id         = CAN_MAX_11BIT_ID,               /* invalid ID */
        .idType     = CAN_EXTENDED_IDENTIFIER_29_BIT, /* invalid ID-type */
        .dlc        = 0u,                             /* invalid data length */
        .endianness = CANRX_DEBUG_ENDIANNESS,         /* valid endianness */
    };
    TEST_ASSERT_FAIL_ASSERT(CANRX_Debug(testMessage, testCanDataZeroArray, &can_kShim));

    /* fix message ID, therfore the invalid ID-type must lead to an assertion*/
    testMessage.id = CANRX_DEBUG_ID;
    TEST_ASSERT_FAIL_ASSERT(CANRX_Debug(testMessage, testCanDataZeroArray, &can_kShim));

    /* fix message ID-type, therefore only DLC length is wrong and must lead to an assertion.*/
    testMessage.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
    TEST_ASSERT_FAIL_ASSERT(CANRX_Debug(testMessage, testCanDataZeroArray, &can_kShim));

    /* fix the message, as we now need to test the rest of the arguments assertions */
    testMessage.id = CAN_DEFAULT_DLC;

    /* invalid pointer for the CAN data */
    TEST_ASSERT_FAIL_ASSERT(CANRX_Debug(testMessage, NULL_PTR, &can_kShim));
    /* invalid pointer for the database shim */
    TEST_ASSERT_FAIL_ASSERT(CANRX_Debug(testMessage, testCanDataZeroArray, NULL_PTR));
}

void testCANRX_DebugAssertionsParamCanData(void) {
    /* invalid pointer for the CAN data */
    TEST_ASSERT_FAIL_ASSERT(CANRX_Debug(validRxDebugTestMessage, NULL_PTR, &can_kShim));
}

void testCANRX_DebugAssertionsParamDatabaseShim(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};
    TEST_ASSERT_FAIL_ASSERT(CANRX_Debug(validRxDebugTestMessage, testCanData, NULL_PTR));
}

/*********************************************************************************************************************/
/* tests of the public interface of the module, i.e., 'CANRX_Debug' when valid input is provided but the sys state does not yet allow to send messages */
void testCANRX_DebugWrongSysState(void) {
    SYS_FSM_STATES_e stored_state = sys_state.currentState;
    sys_state.currentState        = SYS_FSM_STATE_UNINITIALIZED;

    uint8_t testCanData[CAN_MAX_DLC] = {0};

    testCanData[0] = MULTIPLEXER_VALUE_VERSION_INFORMATION; /* version information multiplexer message */
    uint16_t ret   = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_INVALID_CANRX_RETURN_VALUE, ret);

    sys_state.currentState = stored_state;
}

/*********************************************************************************************************************/
/* tests of the public interface of the module, i.e., 'CANRX_Debug' when valid input is provided */
/* the provided parameters are valid, but the multiplexer value is not known to the BMS, therefore we need to answer
   that we do not know how to process this value */
void testCANRX_DebugInvalidMultiplexerValues(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};

    testCanData[0] = INVALID_MULTIPLEXER_VALUE; /* invalid multiplexer value */
    CANTX_DebugUnsupportedMultiplexerVal_Expect(CANRX_DEBUG_ID, INVALID_MULTIPLEXER_VALUE);
    uint16_t ret = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_VALID_CANRX_RETURN_VALUE, ret);
}

/* provide a valid multiplexer value (version information) */
void testCANRX_DebugVersionInformationMultiplexerValue(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};

    testCanData[0] = MULTIPLEXER_VALUE_VERSION_INFORMATION; /* version information multiplexer message */
    uint16_t ret   = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_VALID_CANRX_RETURN_VALUE, ret);
}

/* provide a valid multiplexer value (set the RTC time) */
void testCANRX_DebugRtcMultiplexerValue(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};
    RTC_TIME_DATA_s timeData         = {0};

    RTC_SetRtcRequestFlag_Expect(1u);
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_rtcSetTimeQueue, (void *)&timeData, 0u, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME, STD_OK);
    testCanData[0] = MULTIPLEXER_VALUE_RTC; /* RTC multiplexer message */
    uint16_t ret   = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_VALID_CANRX_RETURN_VALUE, ret);
}

/* provide a valid multiplexer value (request a software reset) */
void testCANRX_DebugSoftwareResetMultiplexerValue(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};

    testCanData[0] = MULTIPLEXER_VALUE_SOFTWARE_RESET; /* Software reset multiplexer message */
    uint16_t ret   = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_VALID_CANRX_RETURN_VALUE, ret);
}

/* provide a valid multiplexer value (request the initialization of the FRAM) */
void testCANRX_DebugFramInitializationMultiplexerValue(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};

    testCanData[0] = MULTIPLEXER_VALUE_FRAM_INITIALIZATION; /* (re-)initialization of the FRAM multiplexer message */
    uint16_t ret   = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_VALID_CANRX_RETURN_VALUE, ret);
}

/* provide a valid multiplexer value (time information) */
void testCANRX_DebugTimeInfoMultiplexerValue(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};

    testCanData[0] = MULTIPLEXER_VALUE_TIME_INFO; /* BMS time information multiplexer message */
    uint16_t ret   = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_VALID_CANRX_RETURN_VALUE, ret);
}

/* provide a valid multiplexer value (uptime information) */
void testCANRX_DebugUptimeInfoMultiplexerValue(void) {
    uint8_t testCanData[CAN_MAX_DLC] = {0};

    testCanData[0] = MULTIPLEXER_VALUE_UPTIME_INFO; /* BMS time information multiplexer message */
    uint16_t ret   = CANRX_Debug(validRxDebugTestMessage, testCanData, &can_kShim);
    TEST_ASSERT_EQUAL(SYS_STATE_VALID_CANRX_RETURN_VALUE, ret);
}

/*********************************************************************************************************************/
/* test RTC helper functions */
void testCANRX_GetHundredthOfSeconds(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetHundredthOfSeconds(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

void testCANRX_GetSeconds(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetSeconds(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

void testCANRX_GetMinutes(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetMinutes(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

void testCANRX_GetHours(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetHours(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

void testCANRX_GetWeekday(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetWeekday(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

void testCANRX_GetDay(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetDay(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

void testCANRX_GetMonth(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetMonth(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

void testCANRX_GetYear(void) {
    /* implemented as big endian */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_GetYear(0u, CAN_LITTLE_ENDIAN));
    /* test message decoding */
}

/*********************************************************************************************************************/
void testCANRX_TriggerBmsSoftwareVersionMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO, STD_OK);
    TEST_CANRX_TriggerBmsSoftwareVersionMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerBmsSoftwareVersionMessage());
}

void testCANRX_TriggerMcuUniqueDieIdMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID, STD_OK);
    TEST_CANRX_TriggerMcuUniqueDieIdMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerMcuUniqueDieIdMessage());
}

void testCANRX_TriggerMcuLotNumberMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER, STD_OK);
    TEST_CANRX_TriggerMcuLotNumberMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerMcuLotNumberMessage());
}

void testCANRX_TriggerMcuWaferInformationMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION, STD_OK);
    TEST_CANRX_TriggerMcuWaferInformationMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerMcuWaferInformationMessage());
}

void testCANRX_TriggerCommitHashMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH, STD_OK);
    TEST_CANRX_TriggerCommitHashMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerCommitHashMessage());
}

void testCANRX_TriggerTimeInfoMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME, STD_OK);
    TEST_CANRX_TriggerTimeInfoMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerTimeInfoMessage());
}

void testCANRX_TriggerUptimeInfoMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_UPTIME, STD_OK);
    TEST_CANRX_TriggerUptimeInfoMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_UPTIME, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerUptimeInfoMessage());
}

void testCANRX_TriggerBootTimestampMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_TIMESTAMP, STD_OK);
    TEST_CANRX_TriggerBootTimestampMessage();

    /* sending response message does not work as expected */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_TIMESTAMP, STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerBootTimestampMessage());
}

void testCANRX_TriggerBuildConfigurationMessage(void) {
    /* sending response message works as expected */
    CANTX_DebugBuildConfiguration_ExpectAndReturn(STD_OK);
    TEST_CANRX_TriggerBuildConfigurationMessage();

    /* sending response message does not work as expected */
    CANTX_DebugBuildConfiguration_ExpectAndReturn(STD_NOT_OK);
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_TriggerBuildConfigurationMessage());
}

/*********************************************************************************************************************/
void testCANRX_CheckIfBmsSoftwareVersionIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfBmsSoftwareVersionIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    uint64_t testMessageData = ((uint64_t)1u) << 48u; /* set bit to indicate that the software version is requested */
    bool isRequested         = TEST_CANRX_CheckIfBmsSoftwareVersionIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfMcuUniqueDieIdIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfMcuUniqueDieIdIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    uint64_t testMessageData = ((uint64_t)1u) << 49u; /* set bit to indicate that the MCU die Id version is requested */
    bool isRequested         = TEST_CANRX_CheckIfMcuUniqueDieIdIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfMcuLotNumberIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfMcuLotNumberIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    uint64_t testMessageData = ((uint64_t)1u) << 50u; /* set bit to indicate that the MCU lot numer is requested */
    bool isRequested         = TEST_CANRX_CheckIfMcuLotNumberIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfMcuWaferInformationIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfMcuWaferInformationIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    /* set bit to indicate that the MCU wafer information is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 51u;
    bool isRequested         = TEST_CANRX_CheckIfMcuWaferInformationIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfCommitHashIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfCommitHashIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    /* set bit to indicate that the MCU wafer information is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 52u;
    bool isRequested         = TEST_CANRX_CheckIfCommitHashIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfSoftwareResetIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfSoftwareResetIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    uint64_t testMessageData = ((uint64_t)1u) << 31u; /* set bit to indicate that the software reset is requested */
    bool isRequested         = TEST_CANRX_CheckIfSoftwareResetIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfFramInitializationIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfFramInitializationIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    /* set bit to indicate that the FRAM (re-)initialization is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 31u;
    bool isRequested         = TEST_CANRX_CheckIfSoftwareResetIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfTimeInfoIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfTimeInfoIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    /* set bit to indicate that the BMS time information is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 48u;
    bool isRequested         = TEST_CANRX_CheckIfTimeInfoIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfUptimeInfoIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfUptimeInfoIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    /* set bit to indicate that the BMS time information is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 40u;
    bool isRequested         = TEST_CANRX_CheckIfUptimeInfoIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfBootTimestampIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfBootTimestampIsRequested(testMessageDataZero, invalidEndianness));

    /* test correct message -> return true */
    /* set bit to indicate that the BMS time information is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 49u;
    bool isRequested         = TEST_CANRX_CheckIfBootTimestampIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

void testCANRX_CheckIfBuildConfigurationIsRequested(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_CheckIfBuildConfigurationIsRequested(testMessageDataZero, invalidEndianness))

    /* test correct message -> return true */
    /* set bit to indicate that the BMS time information is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 53u;
    bool isRequested         = TEST_CANRX_CheckIfBuildConfigurationIsRequested(testMessageData, validEndianness);
    TEST_ASSERT_TRUE(isRequested);
}

/*********************************************************************************************************************/
/* test if all functions that process the multiplexer values (pattern: CANRX_Process.*Mux) */
void testCANRX_ProcessVersionInformationMux(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ProcessVersionInformationMux(testMessageDataZero, invalidEndianness));

    uint64_t testMessageData = ((uint64_t)1u) << 48u; /* set bit to indicate that the software version is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BMS_VERSION_INFO, STD_OK);
    TEST_CANRX_ProcessVersionInformationMux(testMessageData, validEndianness);

    testMessageData = ((uint64_t)1u) << 49u; /* set bit to indicate that the MCU die Id version is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_UNIQUE_DIE_ID, STD_OK);
    TEST_CANRX_ProcessVersionInformationMux(testMessageData, validEndianness);

    testMessageData = ((uint64_t)1u) << 50u; /* set bit to indicate that the MCU lot numer is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_LOT_NUMBER, STD_OK);
    TEST_CANRX_ProcessVersionInformationMux(testMessageData, validEndianness);

    testMessageData = ((uint64_t)1u) << 51u; /* set bit to indicate that the MCU wafer information is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_MCU_WAFER_INFORMATION, STD_OK);
    TEST_CANRX_ProcessVersionInformationMux(testMessageData, validEndianness);

    testMessageData = ((uint64_t)1u) << 52u; /* set bit to indicate that the MCU wafer information is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_COMMIT_HASH, STD_OK);
    TEST_CANRX_ProcessVersionInformationMux(testMessageData, validEndianness);

    testMessageData = ((uint64_t)1u) << 53u; /* set bit to indicate that the MCU wafer information is requested */
    CANTX_DebugBuildConfiguration_ExpectAndReturn(STD_OK);
    TEST_CANRX_ProcessVersionInformationMux(testMessageData, validEndianness);
}

void testCANRX_ProcessRtcMux(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ProcessRtcMux(testMessageDataZero, invalidEndianness));

    /* Test setting a valid rtc time */
    RTC_TIME_DATA_s timeData = {0};
    RTC_SetRtcRequestFlag_Expect(1u);
    OS_SendToBackOfQueue_ExpectAndReturn(ftsk_rtcSetTimeQueue, (void *)&timeData, 0u, STD_OK);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME, STD_OK);
    TEST_CANRX_ProcessRtcMux(testMessageDataZero, validEndianness);

    /* Test setting an invalid rtc time */
    uint64_t testMessageData = UINT64_MAX; /* Initialize message data */
    RTC_SetRtcRequestFlag_Expect(2u);
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME, STD_OK);
    TEST_CANRX_ProcessRtcMux(testMessageData, validEndianness);
}

void testCANRX_ProcessSoftwareResetMux(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ProcessSoftwareResetMux(testMessageDataZero, invalidEndianness));

    uint64_t testMessageData = ((uint64_t)1u) << 31u; /* set bit to indicate that the software reset is requested */
    SYS_TriggerSoftwareReset_Expect();
    TEST_CANRX_ProcessSoftwareResetMux(testMessageData, validEndianness);
}

void testCANRX_ProcessFramInitializationMux(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ProcessFramInitializationMux(testMessageDataZero, invalidEndianness));

    /* set bit to indicate that the FRAM (re-)initialization is requested */
    uint64_t testMessageData = ((uint64_t)1u) << 35u;
    FRAM_ReinitializeAllEntries_ExpectAndReturn(STD_OK);
    TEST_CANRX_ProcessFramInitializationMux(testMessageData, validEndianness);
}

void testCANRX_ProcessTimeInfoMux(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ProcessTimeInfoMux(testMessageDataZero, invalidEndianness));

    /* first test for CANRX_CheckIfTimeInfoIsRequested */
    uint64_t testMessageData = ((uint64_t)1u) << 48u; /* set bit to indicate that the time information is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_RTC_TIME, STD_OK);
    TEST_CANRX_ProcessTimeInfoMux(testMessageData, validEndianness);

    /* second test for CANRX_CheckIfBootTimestampIsRequested */
    testMessageData = ((uint64_t)1u) << 49u; /* set bit to indicate that the time information is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_BOOT_TIMESTAMP, STD_OK);
    TEST_CANRX_ProcessTimeInfoMux(testMessageData, validEndianness);
}

void testCANRX_ProcessUptimeInfoMux(void) {
    /* test endianness assertion */
    TEST_ASSERT_FAIL_ASSERT(TEST_CANRX_ProcessUptimeInfoMux(testMessageDataZero, invalidEndianness));

    uint64_t testMessageData = ((uint64_t)1u) << 40u; /* set bit to indicate that the time information is requested */
    CANTX_DebugResponse_ExpectAndReturn(CANTX_DEBUG_RESPONSE_TRANSMIT_UPTIME, STD_OK);
    TEST_CANRX_ProcessUptimeInfoMux(testMessageData, validEndianness);
}
