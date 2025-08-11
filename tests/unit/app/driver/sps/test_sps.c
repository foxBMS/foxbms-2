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
 * @file    test_sps.c
 * @author  foxBMS Team
 * @date    2020-10-28 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the SPS driver
 * @details TODO
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockcontactor.h"
#include "Mockcontactor_cfg.h"
#include "Mockdatabase.h"
#include "Mockio.h"
#include "Mockmcu.h"
#include "Mockos.h"
#include "Mockpex.h"
#include "Mockpex_cfg.h"
#include "Mockspi.h"
#include "Mockspi_cfg.h"
#include "Mocksps_cfg.h"
#include "Mocksps_types.h"

#include "sps.h"
#include "test_assert_helper.h"

#include <stdbool.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/contactor")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/sps")

/*========== Definitions and Implementations for Unit Test ==================*/
/** GIO defines for pin to drive reset line of SPS @{ */
#define SPS_RESET_GIO_PORT     (hetREG2->DOUT)
#define SPS_RESET_GIO_PORT_DIR (hetREG2->DIR)
#define SPS_RESET_PIN          (16u)
/**@}*/

/** SPI data configuration struct for SPS communication */
static spiDAT1_t spi_kSpsDataConfig = {
    /* struct is implemented in the TI HAL and uses uppercase true and false */
    .CS_HOLD = FALSE,     /*!< The HW chip select signal is deactivated */
    .WDEL    = TRUE,      /*!< No delay will be inserted */
    .DFSEL   = SPI_FMT_0, /*!< Data word format select: data format 0 (SPI3) */
    .CSNR    = 0x0,       /*!< Chip select (CS) number; 0x01h for CS[0] */
};

/** SPI interface configuration for SPS communication */
SPI_INTERFACE_CONFIG_s spi_spsInterface = {
    .pConfig  = &spi_kSpsDataConfig,
    .pNode    = spiREG3,
    .pGioPort = &(spiREG3->PC3),
    .csPin    = 2u,
};

/** channel states */
SPS_CHANNEL_STATE_s sps_channelStatus[SPS_NR_OF_AVAILABLE_SPS_CHANNELS] = {
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_GENERAL_IO, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_GENERAL_IO, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_GENERAL_IO, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
};

const SPS_CHANNEL_FEEDBACK_MAPPING_s sps_kChannelFeedbackMapping[SPS_NR_OF_AVAILABLE_SPS_CHANNELS] = {
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0},
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_1},
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_2},
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_3},
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_4},
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_5},
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_6},
    {PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_7},
};

static uint16_t sps_spiTxRegisterBuffer[SPS_SPI_BUFFERSIZE] = {0};
static uint16_t sps_spiRxRegisterBuffer[SPS_SPI_BUFFERSIZE] = {0};

static uint16_t sps_spiTxWriteToChannelChannelControlRegister[SPS_SPI_BUFFERSIZE];
static uint16_t sps_spiRxReadAnswerDuringChannelControl[SPS_SPI_BUFFERSIZE];

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    /* make sure variables are in a known state */

    for (uint8_t channel = 0u; channel < SPS_NR_OF_AVAILABLE_SPS_CHANNELS; channel++) {
        sps_channelStatus[channel].channelRequested = SPS_CHANNEL_OFF;
        sps_channelStatus[channel].channel          = SPS_CHANNEL_OFF;
        sps_channelStatus[channel].current_mA       = 0.0f;
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testSPS_RequestChannelStateInvalidIndex(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_SPS_RequestChannelState(SPS_NR_OF_AVAILABLE_SPS_CHANNELS + 1u, SPS_CHANNEL_OFF));
}

void testSPS_RequestChannelStateInvalidFunction(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_SPS_RequestChannelState(0u, 42u));
}

void testSPS_RequestChannelStateSwitchOn(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[0u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(0u, SPS_CHANNEL_ON));

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[0u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(0u, SPS_CHANNEL_OFF));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[0u].channelRequested);
}

void testSPS_GetChannelFeedbackInvalidChannelIndex(void) {
    TEST_ASSERT_FAIL_ASSERT(SPS_GetChannelCurrentFeedback(42u));
}

void testSPS_GetChannelFeedbackChannelLow(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    sps_channelStatus[0].current_mA = 0;

    TEST_ASSERT_EQUAL(CONT_SWITCH_OFF, SPS_GetChannelCurrentFeedback(0u));
}

void testSPS_GetChannelFeedbackChannelHigh(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();

    sps_channelStatus[0].current_mA = 600.f;

    TEST_ASSERT_EQUAL(CONT_SWITCH_ON, SPS_GetChannelCurrentFeedback(0u));
}

void testSPS_GetChannelAffiliationInvalidIndex(void) {
    TEST_ASSERT_FAIL_ASSERT(SPS_GetChannelAffiliation((SPS_CHANNEL_INDEX)42u));
}

void testSPS_GetChannelAffiliationValidIndex(void) {
    for (SPS_CHANNEL_INDEX index = 0u; index < SPS_NR_OF_AVAILABLE_SPS_CHANNELS; index++) {
        TEST_ASSERT_EQUAL(sps_channelStatus[index].affiliation, SPS_GetChannelAffiliation(index));
    }
}

void testSPS_RequestContactorStateWrongAffiliation(void) {
    /* watch out, the channel here has to have an affiliation that is not SPS_AFF_CONTACTOR */
    TEST_ASSERT_FAIL_ASSERT(SPS_RequestContactorState(11u, SPS_CHANNEL_OFF));
}

void testSPS_RequestContactorStateWrongIndex(void) {
    /* a wrong index should also lead here to an assertion */
    TEST_ASSERT_FAIL_ASSERT(SPS_RequestContactorState(42u, SPS_CHANNEL_OFF));
}

void testSPS_RequestContactorStateCorrectAffiliation(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* watch out, the channel here has to have an affiliation that is SPS_AFF_CONTACTOR */
    TEST_ASSERT_PASS_ASSERT(SPS_RequestContactorState(0u, SPS_CHANNEL_ON));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[0u].channelRequested);
}

void testSPS_RequestGeneralIOStateWrongAffiliation(void) {
    /* watch out, the channel here has to have an affiliation that is not SPS_AFF_GENERAL_IO */
    TEST_ASSERT_FAIL_ASSERT(SPS_RequestGeneralIoState(0u, SPS_CHANNEL_OFF));
}

void testSPS_RequestGeneralIOStateWrongIndex(void) {
    /* a wrong index should also lead here to an assertion */
    TEST_ASSERT_FAIL_ASSERT(SPS_RequestGeneralIoState(42u, SPS_CHANNEL_OFF));
}

void testSPS_RequestGIOStateCorrectAffiliation(void) {
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    /* watch out, the channel here has to have an affiliation that is SPS_AFF_GENERAL_IO */
    TEST_ASSERT_PASS_ASSERT(SPS_RequestGeneralIoState(7u, SPS_CHANNEL_ON));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[7u].channelRequested);
}

void testSPS_CtrlStartupProcedure(void) {
    /* default wait time */
    const uint8_t defaultTimer = 5u;

    /* check that we init in SPS_START otherwise this test has to be adapted */
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
    TEST_ASSERT_EQUAL(0u, TEST_SPS_GetSpsTimer());

    /* transition through state-machine: SPS_START */
    /* interface should become switched to low speed */
    SPI_SpsInterfaceSwitchToLowSpeed_Expect(&spi_spsInterface);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_RESET_LOW, TEST_SPS_GetSpsState());

    /* transition through state-machine: SPS_RESET_LOW */
    /* check that reset pin is reset */
    IO_PinReset_Expect(&SPS_RESET_GIO_PORT, SPS_RESET_PIN);
    SPS_Ctrl();
    /* timer should now be set to 5 */
    TEST_ASSERT_EQUAL(defaultTimer, TEST_SPS_GetSpsTimer());
    /* state should now be SPS_RESET_HIGH */
    TEST_ASSERT_EQUAL(SPS_RESET_HIGH, TEST_SPS_GetSpsState());
    /* test that timer decreases and nothing else happens */
    SPS_Ctrl();
    TEST_ASSERT_EQUAL((defaultTimer - 1u), TEST_SPS_GetSpsTimer());
    TEST_ASSERT_EQUAL(SPS_RESET_HIGH, TEST_SPS_GetSpsState());
    /* now that we know that it works: overwrite the timer so that
       the next step can proceed */
    TEST_SPS_SetSpsTimer(0u);

    /* transition through state-machine: SPS_RESET_HIGH */
    /* check that reset pin is set
       (we don't care for the actual address as long as the pin is correct) */
    IO_PinSet_Expect(&SPS_RESET_GIO_PORT, SPS_RESET_PIN);
    SPS_Ctrl();
    /* timer should now be set to 5 */
    TEST_ASSERT_EQUAL(defaultTimer, TEST_SPS_GetSpsTimer());
    /* state should now be SPS_CONFIGURE_CONTROL_REGISTER */
    TEST_ASSERT_EQUAL(SPS_CONFIGURE_CONTROL_REGISTER, TEST_SPS_GetSpsState());
    /* reset the timer so that we can continue */
    TEST_SPS_SetSpsTimer(0u);

    /* transition through state-machine: SPS_CONFIGURE_CONTROL_REGISTER */
    /* set txBuffer for spi register correctly */
    TEST_SPS_GlobalRegisterWrite(
        SPS_GLOBAL_CONTROL_REGISTER_ADDRESS,
        (SPS_NORMAL_MODE << SPS_MODE_BIT_START) | (SPS_STRONG_DRIVE << SPS_DRIVE_STRENGTH_BIT_START),
        sps_spiTxRegisterBuffer);
    /* set txBuffer for channel control correctly */
    for (SPS_CHANNEL_INDEX channel = 0u; channel < SPS_NR_OF_AVAILABLE_SPS_CHANNELS; channel++) {
        TEST_SPS_SingleDeviceRegisterWrite(
            channel / SPS_NR_CONTACTOR_PER_IC,
            SPS_OUTPUT_CONTROL_REGISTER_ADDRESS,
            (~(1u << (channel % SPS_NR_CONTACTOR_PER_IC)) & 0xFFu), /* data to write, output control */
            SPS_andWithCurrentValue, /* AND because we want to set additional bits to 0 */
            sps_spiTxWriteToChannelChannelControlRegister);
    }
    /* check that we transmit the first set of commands */
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    /* Since the transaction is successful, we should switch to high-speed */
    SPI_SpsInterfaceSwitchToHighSpeed_Expect(&spi_spsInterface);
    SPS_Ctrl();
    /* state should be now SPS_TRIGGER_CURRENT_MEASUREMENT */
    TEST_ASSERT_EQUAL(SPS_TRIGGER_CURRENT_MEASUREMENT, TEST_SPS_GetSpsState());
}

void testSPS_CtrlResetOnFailedFirstTransaction(void) {
    /* go to state SPS_TRIGGER_CURRENT_MEASUREMENT */
    TEST_SPS_SetSpsState(SPS_TRIGGER_CURRENT_MEASUREMENT);

    /* run state and let first transaction "fail" */
    TEST_SPS_GlobalRegisterWrite(
        SPS_C_CONTROL_REGISTER_ADDRESS,
        0x000F, /* Measure all four outputs for current and voltage on demand*/
        sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
}

void testSPS_CtrlResetOnFailedSecondTransaction(void) {
    /* go to state SPS_TRIGGER_CURRENT_MEASUREMENT */
    TEST_SPS_SetSpsState(SPS_TRIGGER_CURRENT_MEASUREMENT);

    /* write the txBuffer for this state */
    TEST_SPS_GlobalRegisterWrite(
        SPS_C_CONTROL_REGISTER_ADDRESS,
        0x000F, /* Measure all four outputs for current and voltage on demand*/
        sps_spiTxRegisterBuffer);
    /* run state and let second transaction "fail" */
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);

    /* write the txBuffer for this state */
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
}

void testSPS_CtrlResetOnFailedBothTransactions(void) {
    /* go to state SPS_TRIGGER_CURRENT_MEASUREMENT */
    TEST_SPS_SetSpsState(SPS_TRIGGER_CURRENT_MEASUREMENT);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterWrite(
        SPS_C_CONTROL_REGISTER_ADDRESS,
        0x000F, /* Measure all four outputs for current and voltage on demand*/
        sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
}

void testSPS_CtrlResetFromStateSPS_CONFIGURE_CONTROL_REGISTER(void) {
    /* go to state SPS_CONFIGURE_CONTROL_REGISTER */
    TEST_SPS_SetSpsState(SPS_CONFIGURE_CONTROL_REGISTER);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterWrite(
        SPS_GLOBAL_CONTROL_REGISTER_ADDRESS,
        /* transition to Normal mode and set drive strength to strong (This is required for high speed!) */
        (SPS_NORMAL_MODE << SPS_MODE_BIT_START) | (SPS_STRONG_DRIVE << SPS_DRIVE_STRENGTH_BIT_START),
        sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
}

void testSPS_CtrlResetFromStateSPS_TRIGGER_CURRENT_MEASUREMENT(void) {
    /* go to state SPS_TRIGGER_CURRENT_MEASUREMENT */
    TEST_SPS_SetSpsState(SPS_TRIGGER_CURRENT_MEASUREMENT);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterWrite(
        SPS_C_CONTROL_REGISTER_ADDRESS,
        0x000F, /* Measure all four outputs for current and voltage on demand*/
        sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
}

void testSPS_CtrlResetFromStateSPS_READ_EN_IRQ_PIN(void) {
    /* go to state SPS_READ_EN_IRQ_PIN */
    TEST_SPS_SetSpsState(SPS_READ_EN_IRQ_PIN);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterRead(
        SPS_ISR_IRQ_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
}

void testSPS_CtrlResetFromStateSPS_READ_MEASURED_CURRENT1(void) {
    /* go to state SPS_READ_MEASURED_CURRENT1 */
    TEST_SPS_SetSpsState(SPS_READ_MEASURED_CURRENT1);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT1_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
}

void testSPS_CtrlResetFromStateSPS_READ_MEASURED_CURRENT2(void) {
    /* go to state SPS_READ_MEASURED_CURRENT2 */
    TEST_SPS_SetSpsState(SPS_READ_MEASURED_CURRENT2);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT2_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
}

void testSPS_CtrlResetFromStateSPS_READ_MEASURED_CURRENT3(void) {
    /* go to state SPS_READ_MEASURED_CURRENT3 */
    TEST_SPS_SetSpsState(SPS_READ_MEASURED_CURRENT3);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT3_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
}

void testSPS_CtrlResetFromStateSPS_READ_MEASURED_CURRENT4(void) {
    /* go to state SPS_READ_MEASURED_CURRENT4 */
    TEST_SPS_SetSpsState(SPS_READ_MEASURED_CURRENT4);

    /* run state and let both transaction "fail" */
    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT4_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_NOT_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_START, TEST_SPS_GetSpsState());
}

void testSPS_CtrlNormalOperationCycle(void) {
    /* go to state SPS_TRIGGER_CURRENT_MEASUREMENT */
    TEST_SPS_SetSpsState(SPS_TRIGGER_CURRENT_MEASUREMENT);

    /* let transactions always succeed */
    /* run state chain */
    TEST_SPS_GlobalRegisterWrite(
        SPS_C_CONTROL_REGISTER_ADDRESS,
        0x000F, /* Measure all four outputs for current and voltage on demand*/
        sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_READ_EN_IRQ_PIN, TEST_SPS_GetSpsState());

    TEST_SPS_GlobalRegisterRead(
        SPS_ISR_IRQ_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_READ_MEASURED_CURRENT1, TEST_SPS_GetSpsState());

    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT1_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_READ_MEASURED_CURRENT2, TEST_SPS_GetSpsState());

    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT2_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_READ_MEASURED_CURRENT3, TEST_SPS_GetSpsState());

    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT3_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_READ_MEASURED_CURRENT4, TEST_SPS_GetSpsState());

    TEST_SPS_GlobalRegisterRead(
        SPS_OD_IOUT4_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();
    TEST_ASSERT_EQUAL(SPS_TRIGGER_CURRENT_MEASUREMENT, TEST_SPS_GetSpsState());
}

void testSPS_CtrlAssertOnIllegalState(void) {
    /* go to illegal state 128 */
    TEST_SPS_SetSpsState(128u);

    TEST_ASSERT_FAIL_ASSERT(SPS_Ctrl());
}

void testContactorSwitchOnAndOff(void) {
    /* switch on a first, second, third and fourth channel */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[0u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(0u, SPS_CHANNEL_ON));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[0u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[0u].channel);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[1u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(1u, SPS_CHANNEL_ON));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[1u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[1u].channel);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[6u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(6u, SPS_CHANNEL_ON));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[6u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[6u].channel);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[7u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(7u, SPS_CHANNEL_ON));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[7u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[7u].channel);

    /* cycle over one state that handles channels */
    TEST_SPS_SetSpsState(SPS_TRIGGER_CURRENT_MEASUREMENT);
    TEST_SPS_GlobalRegisterWrite(
        SPS_C_CONTROL_REGISTER_ADDRESS,
        0x000F, /* Measure all four outputs for current and voltage on demand*/
        sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_OK);
    MCU_Delay_us_Expect(1u);
    for (SPS_CHANNEL_INDEX channel = 0u; channel < SPS_NR_OF_AVAILABLE_SPS_CHANNELS; channel++) {
        uint8_t spsDevicePositionInDaisyChain = channel / SPS_NR_CONTACTOR_PER_IC;
        TEST_SPS_SingleDeviceRegisterWrite(
            spsDevicePositionInDaisyChain,
            SPS_OUTPUT_CONTROL_REGISTER_ADDRESS,
            (~(1u << (channel % SPS_NR_CONTACTOR_PER_IC)) & 0x0Cu), /* data to write, output control */
            SPS_orWithCurrentValue,                                 /* OR because we want to set additional bits to 1 */
            sps_spiTxWriteToChannelChannelControlRegister);
    }
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();

    /* check that channels have been marked as on */
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[0u].channel);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[1u].channel);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[6u].channel);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[7u].channel);

    /* switch off a first, second, third and fourth channel */
    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[0u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(0u, SPS_CHANNEL_OFF));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[0u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[0u].channel);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[1u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(1u, SPS_CHANNEL_OFF));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[1u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[1u].channel);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[6u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(6u, SPS_CHANNEL_OFF));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[6u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[6u].channel);

    OS_EnterTaskCritical_Expect();
    OS_ExitTaskCritical_Expect();
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[7u].channelRequested);
    TEST_ASSERT_PASS_ASSERT(TEST_SPS_RequestChannelState(7u, SPS_CHANNEL_OFF));
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[7u].channelRequested);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_ON, sps_channelStatus[7u].channel);

    /* cycle over one state that handles channels */
    TEST_SPS_SetSpsState(SPS_TRIGGER_CURRENT_MEASUREMENT);
    TEST_SPS_GlobalRegisterWrite(
        SPS_C_CONTROL_REGISTER_ADDRESS,
        0x000F, /* Measure all four outputs for current and voltage on demand*/
        sps_spiTxRegisterBuffer);
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE, STD_NOT_OK);
    MCU_Delay_us_Expect(1u);
    for (SPS_CHANNEL_INDEX channel = 0u; channel < SPS_NR_OF_AVAILABLE_SPS_CHANNELS; channel++) {
        uint8_t spsDevicePositionInDaisyChain = channel / SPS_NR_CONTACTOR_PER_IC;
        uint8_t writeData                     = (~(1u << (channel % SPS_NR_CONTACTOR_PER_IC)) & 0xFFu);
        TEST_SPS_SingleDeviceRegisterWrite(
            spsDevicePositionInDaisyChain,
            SPS_OUTPUT_CONTROL_REGISTER_ADDRESS,
            writeData,               /* data to write, output control */
            SPS_andWithCurrentValue, /* AND because we want to set additional bits to 0 */
            sps_spiTxWriteToChannelChannelControlRegister);
    }
    SPI_TransmitReceiveData_ExpectAndReturn(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE,
        STD_OK);
    SPS_Ctrl();

    /* check that channels have been marked as off */
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[0u].channel);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[1u].channel);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[6u].channel);
    TEST_ASSERT_EQUAL(SPS_CHANNEL_OFF, sps_channelStatus[7u].channel);
}

/** test the states of PEX feedback function */
void testSPS_GetChannelPexFeedback(void) {
    /* report pin state 0 with normally open --> should report switch off */
    PEX_GetPin_ExpectAndReturn(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0, 0u);
    TEST_ASSERT_EQUAL(CONT_SWITCH_OFF, SPS_GetChannelPexFeedback(0u, true));

    /* report pin state 1 with normally open --> should report switch on */
    PEX_GetPin_ExpectAndReturn(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0, 1u);
    TEST_ASSERT_EQUAL(CONT_SWITCH_ON, SPS_GetChannelPexFeedback(0u, true));

    /* report pin state 0 with normally closed --> should report switch on */
    PEX_GetPin_ExpectAndReturn(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0, 0u);
    TEST_ASSERT_EQUAL(CONT_SWITCH_ON, SPS_GetChannelPexFeedback(0u, false));

    /* report pin state 1 with normally closed --> should report switch off */
    PEX_GetPin_ExpectAndReturn(PEX_PORT_EXPANDER1, PEX_PORT_0_PIN_0, 1u);
    TEST_ASSERT_EQUAL(CONT_SWITCH_OFF, SPS_GetChannelPexFeedback(0u, false));
}

void testSPS_GlobalRegisterWrite(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_SPS_GlobalRegisterWrite(0u, 0u, NULL_PTR));
}

void testSPS_SetCommandTxBuffer(void) {
    TEST_SPS_SetCommandTxBuffer(SPS_ACTION_CONFIGURE_CONTROL_REGISTER);
}
