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
 * @file    ltc_6813-1.c
 * @author  foxBMS Team
 * @date    2019-09-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  LTC
 *
 * @brief   Driver for the LTC analog front-end.
 * @details TODO
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "ltc_6813-1_cfg.h"
#include "ltc.h"
/* clang-format on */

#include "HL_spi.h"
#include "HL_system.h"

#include "afe_plausibility.h"
#include "database.h"
#include "diag.h"
#include "io.h"
#include "ltc_pec.h"
#include "os.h"
#include "pex.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * TI port expander register addresses
 * @{
 */
#define LTC_PORT_EXPANDER_TI_INPUT_REG_ADR  (0x00u)
#define LTC_PORT_EXPANDER_TI_OUTPUT_REG_ADR (0x01u)
#define LTC_PORT_EXPANDER_TI_CONFIG_REG_ADR (0x03u)
/**@}*/

/**
 * LTC COMM definitions
 * @{
 */
#define LTC_ICOM_START            (0x60u)
#define LTC_ICOM_STOP             (0x10u)
#define LTC_ICOM_BLANK            (0x00u)
#define LTC_ICOM_NO_TRANSMIT      (0x70u)
#define LTC_FCOM_MASTER_ACK       (0x00u)
#define LTC_FCOM_MASTER_NACK      (0x08u)
#define LTC_FCOM_MASTER_NACK_STOP (0x09u)
/**@}*/

/* Balancing outputs 0 - 11 in CONFIGURATION REGISTER A */
#define LTC_LOWER_INDEX_FOR_BALANCING_OUTPUTS_IN_REGISTER_WRCFG (0u)
#define LTC_UPPER_INDEX_FOR_BALANCING_OUTPUTS_IN_REGISTER_WRCFG (11u)

/* Balancing outputs 12 - 15/18 in CONFIGURATION REGISTER A */
#define LTC_LOWER_INDEX_FOR_BALANCING_OUTPUTS_IN_REGISTER_WRCFG2 (12u)
#define LTC_UPPER_INDEX_FOR_BALANCING_OUTPUTS_IN_REGISTER_WRCFG2 (17u)

/*========== Static Constant and Variable Definitions =======================*/
/**
 * PEC buffer for RX and TX
 * @{
 */
/* AXIVION Disable Style MisraC2012-1.2: The Pec buffer must be put in the shared RAM section for performance reasons */
#pragma SET_DATA_SECTION(".sharedRAM")
uint16_t ltc_RxPecBuffer[LTC_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
uint16_t ltc_TxPecBuffer[LTC_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
#pragma SET_DATA_SECTION()
/* AXIVION Enable Style MisraC2012-1.2: only Pec buffer needed to be in the shared RAM section */
/**@}*/

/** index of used cells */
static uint16_t ltc_used_cells_index[BS_NR_OF_STRINGS] = {0};
/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s ltc_cellVoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s ltc_cellTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s ltc_balancing_feedback = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_BALANCING_CONTROL_s ltc_balancing_control = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_SLAVE_CONTROL_s ltc_slave_control         = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s ltc_AllGpioVoltage    = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_OPEN_WIRE_s ltc_openWire                  = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
/**@}*/
/** stores information on the detected open wires locally */
static LTC_OPENWIRE_DETECTION_s ltc_openWireDetection = {0};
static LTC_ERRORTABLE_s ltc_errorTable                = {0}; /*!< init in LTC_ResetErrorTable-function */

/** local definition of plausible cell voltage values for the LTC6813 (and similar) */
static const AFE_PLAUSIBILITY_VALUES_s ltc_plausibleCellVoltages681x = {
    .maximumPlausibleVoltage_mV = 5000,
    .minimumPlausibleVoltage_mV = 0,
};

/*========== Extern Constant and Variable Definitions =======================*/

LTC_STATE_s ltc_stateBase = {
    .timer                     = 0,
    .statereq                  = {.request = LTC_STATE_NO_REQUEST, .string = 0xFFu},
    .state                     = LTC_STATEMACH_UNINITIALIZED,
    .substate                  = 0,
    .lastState                 = LTC_STATEMACH_UNINITIALIZED,
    .lastSubstate              = 0,
    .adcModeRequest            = LTC_ADCMODE_FAST_DCP0,
    .adcMode                   = LTC_ADCMODE_FAST_DCP0,
    .adcMeasChannelRequest     = LTC_ADCMEAS_UNDEFINED,
    .adcMeasCh                 = LTC_ADCMEAS_UNDEFINED,
    .numberOfMeasuredMux       = 32,
    .triggerentry              = 0,
    .ErrRetryCounter           = 0,
    .ErrRequestCounter         = 0,
    .VoltageSampleTime         = 0,
    .muxSampleTime             = 0,
    .commandDataTransferTime   = 3,
    .commandTransferTime       = 3,
    .gpioClocksTransferTime    = 3,
    .muxmeas_seqptr            = {NULL_PTR},
    .muxmeas_seqendptr         = {NULL_PTR},
    .muxmeas_nr_end            = {0},
    .first_measurement_made    = false,
    .ltc_muxcycle_finished     = STD_NOT_OK,
    .check_spi_flag            = STD_NOT_OK,
    .balance_control_done      = STD_NOT_OK,
    .transmit_ongoing          = false,
    .dummyByte_ongoing         = STD_NOT_OK,
    .spiDiagErrorEntry         = DIAG_ID_AFE_SPI,
    .pecDiagErrorEntry         = DIAG_ID_AFE_COMMUNICATION_INTEGRITY,
    .muxDiagErrorEntry         = DIAG_ID_AFE_MUX,
    .voltMeasDiagErrorEntry    = DIAG_ID_AFE_CELL_VOLTAGE_MEAS_ERROR,
    .tempMeasDiagErrorEntry    = DIAG_ID_AFE_CELL_TEMPERATURE_MEAS_ERROR,
    .ltcData.pSpiInterface     = spi_ltcInterface,
    .ltcData.txBuffer          = ltc_TxPecBuffer,
    .ltcData.rxBuffer          = ltc_RxPecBuffer,
    .ltcData.frameLength       = LTC_N_BYTES_FOR_DATA_TRANSMISSION,
    .ltcData.cellVoltage       = &ltc_cellVoltage,
    .ltcData.cellTemperature   = &ltc_cellTemperature,
    .ltcData.balancingFeedback = &ltc_balancing_feedback,
    .ltcData.balancingControl  = &ltc_balancing_control,
    .ltcData.slaveControl      = &ltc_slave_control,
    .ltcData.openWireDetection = &ltc_openWireDetection,
    .ltcData.errorTable        = &ltc_errorTable,
    .ltcData.allGpioVoltages   = &ltc_AllGpioVoltage,
    .ltcData.openWire          = &ltc_openWire,
    .ltcData.usedCellIndex     = ltc_used_cells_index,
    .currentString             = 0u,
    .requestedString           = 0u,
};

static uint16_t ltc_cmdWRCFG[4]  = {0x00, 0x01, 0x3D, 0x6E};
static uint16_t ltc_cmdWRCFG2[4] = {0x00, 0x24, 0xB1, 0x9E};
static uint16_t ltc_cmdRDCFG[4]  = {0x00, 0x02, 0x2B, 0x0A};

static uint16_t ltc_cmdRDCVA[4]  = {0x00, 0x04, 0x07, 0xC2};
static uint16_t ltc_cmdRDCVB[4]  = {0x00, 0x06, 0x9A, 0x94};
static uint16_t ltc_cmdRDCVC[4]  = {0x00, 0x08, 0x5E, 0x52};
static uint16_t ltc_cmdRDCVD[4]  = {0x00, 0x0A, 0xC3, 0x04};
static uint16_t ltc_cmdRDCVE[4]  = {0x00, 0x09, 0xD5, 0x60};
static uint16_t ltc_cmdRDCVF[4]  = {0x00, 0x0B, 0x48, 0x36};
static uint16_t ltc_cmdWRCOMM[4] = {0x07, 0x21, 0x24, 0xB2};
static uint16_t ltc_cmdSTCOMM[4] = {0x07, 0x23, 0xB9, 0xE4};
static uint16_t ltc_cmdRDCOMM[4] = {0x07, 0x22, 0x32, 0xD6};
static uint16_t ltc_cmdRDAUXA[4] = {0x00, 0x0C, 0xEF, 0xCC};
static uint16_t ltc_cmdRDAUXB[4] = {0x00, 0x0E, 0x72, 0x9A};
static uint16_t ltc_cmdRDAUXC[4] = {0x00, 0x0D, 0x64, 0xFE};
static uint16_t ltc_cmdRDAUXD[4] = {0x00, 0x0F, 0xF9, 0xA8};

/* static uint16_t ltc_cmdMUTE[4] = {0x00, 0x28, 0xE8, 0x0E};                    !< MUTE discharging via S pins */
/* static uint16_t ltc_cmdUNMUTE[4] = {0x00, 0x29, 0x63, 0x3C};                  !< UN-MUTE discharging via S pins */

/* LTC I2C commands */
/* static uint16_t ltc_I2CcmdDummy[6] = {0x7F, 0xF9, 0x7F, 0xF9, 0x7F, 0xF9};      !< dummy command (no transmit) */

static uint16_t ltc_I2CcmdTempSens0[6] = {
    0x69,
    0x08,
    0x00,
    0x09,
    0x7F,
    0xF9}; /*!< sets the internal data pointer of the temperature sensor (address 0x48) to 0x00 */
static uint16_t ltc_I2CcmdTempSens1[6] =
    {0x69, 0x18, 0x0F, 0xF0, 0x0F, 0xF9}; /*!< reads two data bytes from the temperature sensor */

static uint16_t ltc_I2CcmdPortExpander1[6] =
    {0x64, 0x18, 0x0F, 0xF9, 0x7F, 0xF9}; /*!< reads one data byte from the port expander */

/* Cells */
static uint16_t ltc_cmdADCV_normal_DCP0[4] =
    {0x03, 0x60, 0xF4, 0x6C}; /*!< All cells, normal mode, discharge not permitted (DCP=0)    */
static uint16_t ltc_cmdADCV_normal_DCP1[4] =
    {0x03, 0x70, 0xAF, 0x42}; /*!< All cells, normal mode, discharge permitted (DCP=1)        */
static uint16_t ltc_cmdADCV_filtered_DCP0[4] =
    {0x03, 0xE0, 0xB0, 0x4A}; /*!< All cells, filtered mode, discharge not permitted (DCP=0)  */
static uint16_t ltc_cmdADCV_filtered_DCP1[4] =
    {0x03, 0xF0, 0xEB, 0x64}; /*!< All cells, filtered mode, discharge permitted (DCP=1)      */
static uint16_t ltc_cmdADCV_fast_DCP0[4] =
    {0x02, 0xE0, 0x38, 0x06}; /*!< All cells, fast mode, discharge not permitted (DCP=0)      */
static uint16_t ltc_cmdADCV_fast_DCP1[4] =
    {0x02, 0xF0, 0x63, 0x28}; /*!< All cells, fast mode, discharge permitted (DCP=1)          */
static uint16_t ltc_cmdADCV_fast_DCP0_twocells[4] =
    {0x02, 0xE1, 0xB3, 0x34}; /*!< Two cells (1 and 7), fast mode, discharge not permitted (DCP=0) */

/* GPIOs  */
static uint16_t ltc_cmdADAX_normal_GPIO1[4]   = {0x05, 0x61, 0x58, 0x92}; /*!< Single channel, GPIO 1, normal mode   */
static uint16_t ltc_cmdADAX_filtered_GPIO1[4] = {0x05, 0xE1, 0x1C, 0xB4}; /*!< Single channel, GPIO 1, filtered mode */
static uint16_t ltc_cmdADAX_fast_GPIO1[4]     = {0x04, 0xE1, 0x94, 0xF8}; /*!< Single channel, GPIO 1, fast mode     */
static uint16_t ltc_cmdADAX_normal_GPIO2[4]   = {0x05, 0x62, 0x4E, 0xF6}; /*!< Single channel, GPIO 2, normal mode   */
static uint16_t ltc_cmdADAX_filtered_GPIO2[4] = {0x05, 0xE2, 0x0A, 0xD0}; /*!< Single channel, GPIO 2, filtered mode */
static uint16_t ltc_cmdADAX_fast_GPIO2[4]     = {0x04, 0xE2, 0x82, 0x9C}; /*!< Single channel, GPIO 2, fast mode     */
static uint16_t ltc_cmdADAX_normal_GPIO3[4]   = {0x05, 0x63, 0xC5, 0xC4}; /*!< Single channel, GPIO 3, normal mode   */
static uint16_t ltc_cmdADAX_filtered_GPIO3[4] = {0x05, 0xE3, 0x81, 0xE2}; /*!< Single channel, GPIO 3, filtered mode */
static uint16_t ltc_cmdADAX_fast_GPIO3[4]     = {0x04, 0xE3, 0x09, 0xAE}; /*!< Single channel, GPIO 3, fast mode     */
/* static uint16_t ltc_cmdADAX_normal_GPIO4[4] = {0x05, 0x64, 0x62, 0x3E};      !< Single channel, GPIO 4, normal mode
 */
/* static uint16_t ltc_cmdADAX_filtered_GPIO4[4] = {0x05, 0xE4, 0x26, 0x18};    !< Single channel, GPIO 4, filtered mode
 */
/* static uint16_t ltc_cmdADAX_fast_GPIO4[4] = {0x04, 0xE4, 0xAE, 0x54};        !< Single channel, GPIO 4, fast mode */
/* static uint16_t ltc_cmdADAX_normal_GPIO5[4] = {0x05, 0x65, 0xE9, 0x0C};      !< Single channel, GPIO 5, normal mode
 */
/* static uint16_t ltc_cmdADAX_filtered_GPIO5[4] = {0x05, 0xE5, 0xAD, 0x2A};    !< Single channel, GPIO 5, filtered mode
 */
/* static uint16_t ltc_cmdADAX_fast_GPIO5[4] = {0x04, 0xE5, 0x25, 0x66};        !< Single channel, GPIO 5, fast mode */
static uint16_t ltc_cmdADAX_normal_ALL_GPIOS[4]   = {0x05, 0x60, 0xD3, 0xA0}; /*!< All channels, normal mode   */
static uint16_t ltc_cmdADAX_filtered_ALL_GPIOS[4] = {0x05, 0xE0, 0x97, 0x86}; /*!< All channels, filtered mode */
static uint16_t ltc_cmdADAX_fast_ALL_GPIOS[4] = {0x04, 0xE0, 0x1F, 0xCA}; /*!< All channels, fast mode               */

/* Open-wire */
static uint16_t ltc_BC_cmdADOW_PUP_normal_DCP0[4] = {
    0x03,
    0x68,
    0x1C,
    0x62}; /*!< Broadcast, Pull-up current, All cells, normal mode, discharge not permitted (DCP=0) */
static uint16_t ltc_BC_cmdADOW_PDOWN_normal_DCP0[4] = {
    0x03,
    0x28,
    0xFB,
    0xE8}; /*!< Broadcast, Pull-down current, All cells, normal mode, discharge not permitted (DCP=0) */
static uint16_t ltc_BC_cmdADOW_PUP_filtered_DCP0[4] = {
    0x03,
    0xE8,
    0x58,
    0x44}; /*!< Broadcast, Pull-up current, All cells, filtered mode, discharge not permitted (DCP=0)   */
static uint16_t ltc_BC_cmdADOW_PDOWN_filtered_DCP0[4] = {
    0x03,
    0xA8,
    0xBF,
    0xCE}; /*!< Broadcast, Pull-down current, All cells, filtered mode, discharge not permitted (DCP=0) */

/*========== Static Function Prototypes =====================================*/
static void LTC_SetFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state);
static void LTC_InitializeDatabase(LTC_STATE_s *ltc_state);
static void LTC_SaveBalancingFeedback(LTC_STATE_s *ltc_state, uint16_t *DataBufferSPI_RX, uint8_t stringNumber);
static void LTC_GetBalancingControlValues(LTC_STATE_s *ltc_state);
static void LTC_SaveLastStates(LTC_STATE_s *ltc_state);
static void LTC_StateTransition(LTC_STATE_s *ltc_state, LTC_STATEMACH_e state, uint8_t substate, uint16_t timer_ms);
static void LTC_CondBasedStateTransition(
    LTC_STATE_s *ltc_state,
    STD_RETURN_TYPE_e retVal,
    DIAG_ID_e diagCode,
    LTC_STATEMACH_e state_ok,
    uint8_t substate_ok,
    uint16_t timer_ms_ok,
    LTC_STATEMACH_e state_nok,
    uint8_t substate_nok,
    uint16_t timer_ms_nok);

static STD_RETURN_TYPE_e LTC_BalanceControl(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t registerSet,
    uint8_t stringNumber);

static void LTC_ResetErrorTable(LTC_STATE_s *ltc_state);
static STD_RETURN_TYPE_e LTC_Init(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);

static STD_RETURN_TYPE_e LTC_StartVoltageMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    LTC_ADCMEAS_CHAN_e adcMeasCh);
static STD_RETURN_TYPE_e LTC_StartGpioMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    LTC_ADCMEAS_CHAN_e adcMeasCh);
static STD_RETURN_TYPE_e LTC_StartOpenWireMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    uint8_t PUP);

static uint16_t LTC_GetMeasurementTimeCycle(LTC_ADCMODE_e adcMode, LTC_ADCMEAS_CHAN_e adcMeasCh);
static void LTC_SaveRxToVoltageBuffer(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t registerSet,
    uint8_t stringNumber);
static void LTC_SaveRxToGpioBuffer(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t registerSet,
    uint8_t stringNumber);

static STD_RETURN_TYPE_e LTC_CheckPec(
    LTC_STATE_s *ltc_state,
    uint16_t *DataBufferSPI_RX_with_PEC,
    uint8_t stringNumber);
static STD_RETURN_TYPE_e LTC_ReadRegister(
    uint16_t *Command,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);
static STD_RETURN_TYPE_e LTC_WriteRegister(
    uint16_t *Command,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);
static void LTC_SetMuxChCommand(uint16_t *pTxBuff, uint8_t mux, uint8_t channel);
static STD_RETURN_TYPE_e LTC_SendEepromReadCommand(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t step);
static void LTC_SetEepromReadCommand(LTC_STATE_s *ltc_state, uint16_t *pTxBuff, uint8_t step);
static void LTC_EepromSaveReadValue(LTC_STATE_s *ltc_state, uint16_t *pRxBuff);
static STD_RETURN_TYPE_e LTC_SendEepromWriteCommand(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t step);
static void LTC_SetEepromWriteCommand(LTC_STATE_s *ltc_state, uint16_t *pTxBuff, uint8_t step);
static STD_RETURN_TYPE_e LTC_SetMuxChannel(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t mux,
    uint8_t channel);
static STD_RETURN_TYPE_e LTC_SetPortExpander(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);
static void LTC_PortExpanderSaveValues(LTC_STATE_s *ltc_state, uint16_t *pRxBuff);
static void LTC_TempSensSaveTemp(LTC_STATE_s *ltc_state, uint16_t *pRxBuff);
static STD_RETURN_TYPE_e LTC_SetPortExpanderDirectionTi(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    LTC_PORT_EXPANDER_TI_DIRECTION_e direction);
static STD_RETURN_TYPE_e LTC_SetPortExpanderOutputTi(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);
static STD_RETURN_TYPE_e LTC_GetPortExpanderInputTi(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t step);
static void LTC_PortExpanderSaveValuesTi(LTC_STATE_s *ltc_state, uint16_t *pTxBuff);

static STD_RETURN_TYPE_e LTC_I2cClock(SPI_INTERFACE_CONFIG_s *pSpiInterface);
static STD_RETURN_TYPE_e LTC_SendI2cCommand(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint16_t *cmd_data);

static STD_RETURN_TYPE_e LTC_I2cCheckAcknowledge(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t mux,
    uint8_t stringNumber);

static void LTC_SaveMuxMeasurement(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    LTC_MUX_CH_CFG_s *muxseqptr,
    uint8_t stringNumber);

static uint32_t LTC_GetSpiClock(SPI_INTERFACE_CONFIG_s *pSpiInterface);
static void LTC_SetTransferTimes(LTC_STATE_s *ltc_state);

static LTC_RETURN_TYPE_e LTC_CheckStateRequest(LTC_STATE_s *ltc_state, LTC_REQUEST_s statereq);

/*========== Static Function Implementations ================================*/
/**
 * @brief   in the database, initializes the fields related to the LTC drivers.
 *
 * This function loops through all the LTC-related data fields in the database
 * and sets them to 0. It should be called in the initialization or re-initialization
 * routine of the LTC driver.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 */
static void LTC_InitializeDatabase(LTC_STATE_s *ltc_state) {
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        ltc_state->ltcData.cellVoltage->state = 0;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                ltc_state->ltcData.cellVoltage->cellVoltage_mV[s][m][cb] = 0u;
            }
        }

        for (uint16_t i = 0; i < BS_NR_OF_CELL_BLOCKS_PER_STRING; i++) {
            ltc_state->ltcData.openWireDetection->openWirePup[s][i]   = 0;
            ltc_state->ltcData.openWireDetection->openWirePdown[s][i] = 0;
            ltc_state->ltcData.openWireDetection->openWireDelta[s][i] = 0;
        }

        ltc_state->ltcData.cellTemperature->state = 0;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                ltc_state->ltcData.cellTemperature->cellTemperature_ddegC[s][m][ts] = 0;
            }
        }

        ltc_state->ltcData.balancingFeedback->state = 0;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                ltc_state->ltcData.balancingControl->activateBalancing[s][m][cb] = false;
            }
        }
        ltc_state->ltcData.balancingControl->nrBalancedCells[s] = 0u;
        for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
            ltc_state->ltcData.balancingFeedback->value[s][i] = 0;
        }

        ltc_state->ltcData.slaveControl->state = 0;
        for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
            ltc_state->ltcData.slaveControl->ioValueIn[i]                 = 0;
            ltc_state->ltcData.slaveControl->ioValueOut[i]                = 0;
            ltc_state->ltcData.slaveControl->externalTemperatureSensor[i] = 0;
            ltc_state->ltcData.slaveControl->eepromValueRead[i]           = 0;
            ltc_state->ltcData.slaveControl->eepromValueWrite[i]          = 0;
        }
        ltc_state->ltcData.slaveControl->eepromReadAddressLastUsed  = 0xFFFFFFFF;
        ltc_state->ltcData.slaveControl->eepromReadAddressToUse     = 0xFFFFFFFF;
        ltc_state->ltcData.slaveControl->eepromWriteAddressLastUsed = 0xFFFFFFFF;
        ltc_state->ltcData.slaveControl->eepromWriteAddressToUse    = 0xFFFFFFFF;

        ltc_state->ltcData.allGpioVoltages->state = 0;
        for (uint16_t i = 0; i < (BS_NR_OF_MODULES_PER_STRING * SLV_NR_OF_GPIOS_PER_MODULE); i++) {
            ltc_state->ltcData.allGpioVoltages->gpioVoltages_mV[s][i] = 0;
        }

        for (uint16_t i = 0; i < (BS_NR_OF_MODULES_PER_STRING * (BS_NR_OF_CELL_BLOCKS_PER_MODULE + 1)); i++) {
            ltc_state->ltcData.openWire->openWire[s][i] = 0;
        }
        ltc_state->ltcData.openWire->state = 0;
    }

    DATA_WRITE_DATA(
        ltc_state->ltcData.cellVoltage,
        ltc_state->ltcData.cellTemperature,
        ltc_state->ltcData.balancingFeedback,
        ltc_state->ltcData.openWire);
    DATA_WRITE_DATA(ltc_state->ltcData.balancingControl, ltc_state->ltcData.slaveControl);
}

/**
 * @brief Saves the last state and the last substate
 *
 * @param  ltc_state:  state of the ltc state machine
 */
static void LTC_SaveLastStates(LTC_STATE_s *ltc_state) {
    ltc_state->lastState    = ltc_state->state;
    ltc_state->lastSubstate = ltc_state->substate;
}

/**
 * @brief   function for setting LTC_Trigger state transitions
 *
 * @param  ltc_state:  state of the ltc state machine
 * @param  state:      state to transition into
 * @param  substate:   substate to transition into
 * @param  timer_ms:   transition into state, substate after timer elapsed
 */
static void LTC_StateTransition(LTC_STATE_s *ltc_state, LTC_STATEMACH_e state, uint8_t substate, uint16_t timer_ms) {
    ltc_state->state    = state;
    ltc_state->substate = substate;
    ltc_state->timer    = timer_ms;
}

/**
 * @brief   condition-based state transition depending on retVal
 *
 * If retVal is #STD_OK, after timer_ms_ok is elapsed the LTC state machine will
 * transition into state_ok and substate_ok, otherwise after timer_ms_nok the
 * state machine will transition to state_nok and substate_nok. Depending on
 * value of retVal the corresponding diagnosis entry will be called.
 *
 * @param  ltc_state    state of the ltc state machine
 * @param  retVal       condition to determine if state machine will transition
 *                      into ok or nok states
 * @param  diagCode     symbolic IDs for diagnosis entry, called with
 *                      #DIAG_EVENT_OK if retVal is #STD_OK, #DIAG_EVENT_NOT_OK
 *                      otherwise
 * @param  state_ok     state to transition into if retVal is #STD_OK
 * @param  substate_ok  substate to transition into if retVal is #STD_OK
 * @param  timer_ms_ok  transition into state_ok, substate_ok after timer_ms_ok
 *                      elapsed
 * @param  state_nok    state to transition into if retVal is #STD_NOT_OK
 * @param  substate_nok substate to transition into if retVal is #STD_NOT_OK
 * @param  timer_ms_nok transition into state_nok, substate_nok after
 *                      timer_ms_nok elapsed
 */
static void LTC_CondBasedStateTransition(
    LTC_STATE_s *ltc_state,
    STD_RETURN_TYPE_e retVal,
    DIAG_ID_e diagCode,
    LTC_STATEMACH_e state_ok,
    uint8_t substate_ok,
    uint16_t timer_ms_ok,
    LTC_STATEMACH_e state_nok,
    uint8_t substate_nok,
    uint16_t timer_ms_nok) {
    if ((retVal != STD_OK)) {
        DIAG_Handler(diagCode, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
        LTC_StateTransition(ltc_state, state_nok, substate_nok, timer_ms_nok);
    } else {
        DIAG_Handler(diagCode, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
        LTC_StateTransition(ltc_state, state_ok, substate_ok, timer_ms_ok);
    }
}

extern void LTC_SaveVoltages(LTC_STATE_s *ltc_state, uint8_t stringNumber) {
    /* Pointer validity check */
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);

    /* Iterate over all cell to:
     *
     * 1. Check open-wires and set respective cell measurements to invalid
     * 2. Perform minimum/maximum measurement value plausibility check
     * 3. Calculate string values
     */
    STD_RETURN_TYPE_e cellVoltageMeasurementValid = STD_OK;
    int32_t stringVoltage_mV                      = 0;
    uint16_t numberValidMeasurements              = 0;
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
            /* ------- 1. Check open-wires -----------------
             * Is cell N input not open wire &&
             * Is cell N+1 input not open wire &&
             * Is cell voltage valid because of previous PEC error
             * If so, everything okay, else set cell voltage measurement to invalid.
             */
            if ((ltc_state->ltcData.openWire
                     ->openWire[stringNumber][(m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE + 1u)) + cb] == 0u) &&
                (ltc_state->ltcData.openWire
                     ->openWire[stringNumber][(m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE + 1u)) + cb + 1u] == 0u) &&
                ((ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][m][cb] == false))) {
                /* Cell voltage is valid -> perform minimum/maximum plausibility check */

                /* ------- 2. Perform minimum/maximum measurement range check ---------- */
                if (STD_OK == AFE_PlausibilityCheckVoltageMeasurementRange(
                                  ltc_state->ltcData.cellVoltage->cellVoltage_mV[stringNumber][m][cb],
                                  ltc_plausibleCellVoltages681x)) {
                    /* Cell voltage is valid ->  calculate string voltage */
                    /* -------- 3. Calculate string values ------------- */
                    stringVoltage_mV += ltc_state->ltcData.cellVoltage->cellVoltage_mV[stringNumber][m][cb];
                    numberValidMeasurements++;
                } else {
                    /* Invalidate cell voltage measurement */
                    ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][m][cb] = true;
                    cellVoltageMeasurementValid                                             = STD_NOT_OK;
                }
            } else {
                /* Set cell voltage measurement value invalid, if not already invalid because of PEC Error */
                ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][m][cb] = true;
                cellVoltageMeasurementValid                                             = STD_NOT_OK;
            }
        }
    }
    DIAG_CheckEvent(cellVoltageMeasurementValid, ltc_state->voltMeasDiagErrorEntry, DIAG_STRING, stringNumber);
    ltc_state->ltcData.cellVoltage->stringVoltage_mV[stringNumber]    = stringVoltage_mV;
    ltc_state->ltcData.cellVoltage->nrValidCellVoltages[stringNumber] = numberValidMeasurements;

    /* Increment state variable each time new values are written into database */
    ltc_state->ltcData.cellVoltage->state++;

    DATA_WRITE_DATA(ltc_state->ltcData.cellVoltage);
}

/*========== Extern Function Implementations ================================*/
extern void LTC_SaveTemperatures(LTC_STATE_s *ltc_state, uint8_t stringNumber) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    STD_RETURN_TYPE_e cellTemperatureMeasurementValid = STD_OK;
    uint16_t numberValidMeasurements                  = 0;

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
            /* ------- 1. Check valid flag  -----------------
             * Is cell temperature valid because of previous PEC error
             * If so, everything okay, else set cell temperature measurement to invalid.
             */
            if (ltc_state->ltcData.cellTemperature->invalidCellTemperature[stringNumber][m][ts] == false) {
                /* Cell temperature is valid -> perform minimum/maximum plausibility check */

                /* ------- 2. Perform minimum/maximum measurement range check ---------- */
                if (STD_OK == AFE_PlausibilityCheckTempMinMax(
                                  ltc_state->ltcData.cellTemperature->cellTemperature_ddegC[stringNumber][m][ts])) {
                    numberValidMeasurements++;
                } else {
                    /* Invalidate cell temperature measurement */
                    ltc_state->ltcData.cellTemperature->invalidCellTemperature[stringNumber][m][ts] = true;
                    cellTemperatureMeasurementValid                                                 = STD_NOT_OK;
                }
            } else {
                /* Already invalid because of PEC Error */
                cellTemperatureMeasurementValid = STD_NOT_OK;
            }
        }
    }
    DIAG_CheckEvent(cellTemperatureMeasurementValid, ltc_state->tempMeasDiagErrorEntry, DIAG_STRING, stringNumber);

    ltc_state->ltcData.cellTemperature->nrValidTemperatures[stringNumber] = numberValidMeasurements;

    ltc_state->ltcData.cellTemperature->state++;
    DATA_WRITE_DATA(ltc_state->ltcData.cellTemperature);
}

extern void LTC_SaveAllGpioMeasurement(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    ltc_state->ltcData.allGpioVoltages->state++;
    DATA_WRITE_DATA(ltc_state->ltcData.allGpioVoltages);
}

/**
 * @brief   stores the measured balancing feedback values in the database.
 *
 * This function stores the global balancing feedback value measured on GPIO3 of the LTC into the database
 *
 * @param  ltc_state          state of the ltc state machine
 * @param  DataBufferSPI_RX   receive buffer of the SPI interface
 * @param  stringNumber       string addressed
 *
 */
static void LTC_SaveBalancingFeedback(LTC_STATE_s *ltc_state, uint16_t *DataBufferSPI_RX, uint8_t stringNumber) {
    for (uint16_t i = 0; i < LTC_N_LTC; i++) {
        const uint16_t val_i = DataBufferSPI_RX[8u + (1u * i * 8u)] |
                               (DataBufferSPI_RX[8u + (1u * i * 8u) + 1u] << 8u); /* raw value, GPIO3 */

        ltc_state->ltcData.balancingFeedback->value[stringNumber][i] = val_i;
    }

    ltc_state->ltcData.balancingFeedback->state++;
    DATA_WRITE_DATA(ltc_state->ltcData.balancingFeedback);
}

/**
 * @brief   gets the balancing orders from the database.
 *
 * This function gets the balancing control from the database. Balancing control
 * is set by the BMS. The LTC driver only executes the balancing orders.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 */
static void LTC_GetBalancingControlValues(LTC_STATE_s *ltc_state) {
    DATA_READ_DATA(ltc_state->ltcData.balancingControl);
}

/**
 * @brief   re-entrance check of LTC state machine trigger function
 *
 * This function is not re-entrant and should only be called time- or event-triggered.
 * It increments the triggerentry counter from the state variable ltc_state.
 * It should never be called by two different processes, so if it is the case, triggerentry
 * should never be higher than 0 when this function is called.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 * @return  retval  0 if no further instance of the function is active, 0xff else
 *
 */
uint8_t LTC_CheckReEntrance(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    uint8_t retval = 0;

    OS_EnterTaskCritical();
    if (!ltc_state->triggerentry) {
        ltc_state->triggerentry++;
    } else {
        retval = 0xFF; /* multiple calls of function */
    }
    OS_ExitTaskCritical();

    return (retval);
}

extern LTC_REQUEST_s LTC_GetStateRequest(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    LTC_REQUEST_s retval = {.request = LTC_STATE_NO_REQUEST, .string = 0x0u};

    OS_EnterTaskCritical();
    retval.request = ltc_state->statereq.request;
    retval.string  = ltc_state->statereq.string;
    OS_ExitTaskCritical();

    return (retval);
}

extern LTC_STATEMACH_e LTC_GetState(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    return ltc_state->state;
}

/**
 * @brief   transfers the current state request to the state machine.
 *
 * This function takes the current state request from ltc_state and transfers it to the state machine.
 * It resets the value from ltc_state to LTC_STATE_NO_REQUEST
 *
 * @param   ltc_state:  state of the ltc state machine
 * @param   pBusIDptr       bus ID, main or backup (deprecated)
 * @param   pAdcModeptr     LTC ADCMeasurement mode (fast, normal or filtered)
 * @param   pAdcMeasChptr   number of channels measured for GPIOS (one at a time for multiplexers or all five GPIOs)
 *
 * @return  retVal          current state request, taken from LTC_STATE_REQUEST_e
 *
 */
LTC_REQUEST_s LTC_TransferStateRequest(
    LTC_STATE_s *ltc_state,
    uint8_t *pBusIDptr,
    LTC_ADCMODE_e *pAdcModeptr,
    LTC_ADCMEAS_CHAN_e *pAdcMeasChptr) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pBusIDptr != NULL_PTR);
    FAS_ASSERT(pAdcModeptr != NULL_PTR);
    FAS_ASSERT(pAdcMeasChptr != NULL_PTR);
    LTC_REQUEST_s retval = {.request = LTC_STATE_NO_REQUEST, .string = 0x0u};

    OS_EnterTaskCritical();
    retval.request              = ltc_state->statereq.request;
    retval.string               = ltc_state->statereq.string;
    ltc_state->requestedString  = ltc_state->statereq.string;
    *pAdcModeptr                = ltc_state->adcModeRequest;
    *pAdcMeasChptr              = ltc_state->adcMeasChannelRequest;
    ltc_state->statereq.request = LTC_STATE_NO_REQUEST;
    ltc_state->statereq.string  = 0x0u;
    OS_ExitTaskCritical();

    return (retval);
}

LTC_RETURN_TYPE_e LTC_SetStateRequest(LTC_STATE_s *ltc_state, LTC_REQUEST_s statereq) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    LTC_RETURN_TYPE_e retVal = LTC_ERROR;

    OS_EnterTaskCritical();
    retVal = LTC_CheckStateRequest(ltc_state, statereq);

    if ((retVal == LTC_OK) || (retVal == LTC_BUSY_OK) || (retVal == LTC_OK_FROM_ERROR)) {
        ltc_state->statereq = statereq;
    }
    OS_ExitTaskCritical();

    return (retVal);
}

void LTC_Trigger(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    STD_RETURN_TYPE_e retVal           = STD_OK;
    LTC_REQUEST_s statereq             = {.request = LTC_STATE_NO_REQUEST, .string = 0x0u};
    uint8_t tmpbusID                   = 0;
    LTC_ADCMODE_e tmpadcMode           = LTC_ADCMODE_UNDEFINED;
    LTC_ADCMEAS_CHAN_e tmpadcMeasCh    = LTC_ADCMEAS_UNDEFINED;
    STD_RETURN_TYPE_e continueFunction = STD_OK;

    FAS_ASSERT(ltc_state != NULL_PTR);

    /* Check re-entrance of function */
    if (LTC_CheckReEntrance(ltc_state) > 0u) {
        continueFunction = STD_NOT_OK;
    }

    if (ltc_state->check_spi_flag == STD_NOT_OK) {
        if (ltc_state->timer > 0u) {
            if ((--ltc_state->timer) > 0u) {
                ltc_state->triggerentry--;
                continueFunction = STD_NOT_OK; /* handle state machine only if timer has elapsed */
            }
        }
    } else {
        if (AFE_IsTransmitOngoing(ltc_state) == true) {
            if (ltc_state->timer > 0u) {
                if ((--ltc_state->timer) > 0u) {
                    ltc_state->triggerentry--;
                    continueFunction = STD_NOT_OK; /* handle state machine only if timer has elapsed */
                }
            }
        }
    }

    if (continueFunction == STD_OK) {
        switch (ltc_state->state) {
            /****************************UNINITIALIZED***********************************/
            case LTC_STATEMACH_UNINITIALIZED:
                /* waiting for Initialization Request */
                statereq = LTC_TransferStateRequest(ltc_state, &tmpbusID, &tmpadcMode, &tmpadcMeasCh);
                if (statereq.request == LTC_STATE_INIT_REQUEST) {
                    LTC_SaveLastStates(ltc_state);
                    LTC_InitializeDatabase(ltc_state);
                    LTC_ResetErrorTable(ltc_state);
                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_INITIALIZATION, LTC_INIT_STRING, LTC_STATEMACH_SHORTTIME);
                    ltc_state->adcMode   = tmpadcMode;
                    ltc_state->adcMeasCh = tmpadcMeasCh;
                } else if (statereq.request == LTC_STATE_NO_REQUEST) {
                    /* no actual request pending */
                } else {
                    ltc_state->ErrRequestCounter++; /* illegal request pending */
                }
                break;

            /****************************INITIALIZATION**********************************/
            case LTC_STATEMACH_INITIALIZATION:

                LTC_SetTransferTimes(ltc_state);

                if (ltc_state->substate == LTC_INIT_STRING) {
                    LTC_SaveLastStates(ltc_state);
                    ltc_state->currentString = 0u;

                    ltc_state->spiSeqPtr           = ltc_state->ltcData.pSpiInterface;
                    ltc_state->spiNumberInterfaces = BS_NR_OF_STRINGS;
                    ltc_state->spiSeqEndPtr        = ltc_state->ltcData.pSpiInterface + BS_NR_OF_STRINGS;
                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_INITIALIZATION, LTC_ENTRY_INITIALIZATION, LTC_STATEMACH_SHORTTIME);
                } else if (ltc_state->substate == LTC_ENTRY_INITIALIZATION) {
                    LTC_SaveLastStates(ltc_state);

                    ltc_state->muxmeas_seqptr[ltc_state->currentString] = ltc_mux_seq.seqptr;
                    ltc_state->muxmeas_nr_end[ltc_state->currentString] = ltc_mux_seq.nr_of_steps;
                    ltc_state->muxmeas_seqendptr[ltc_state->currentString] =
                        ((LTC_MUX_CH_CFG_s *)ltc_mux_seq.seqptr) + ltc_mux_seq.nr_of_steps; /* last sequence + 1 */

                    retVal =
                        LTC_TRANSMIT_WAKE_UP(ltc_state->spiSeqPtr); /* Send dummy byte to wake up the daisy chain */
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_RE_ENTRY_INITIALIZATION,
                        LTC_STATEMACH_DAISY_CHAIN_FIRST_INITIALIZATION_TIME,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_ENTRY_INITIALIZATION,
                        LTC_STATEMACH_SHORTTIME);
                } else if (ltc_state->substate == LTC_RE_ENTRY_INITIALIZATION) {
                    LTC_SaveLastStates(ltc_state);
                    retVal = LTC_TRANSMIT_WAKE_UP(
                        ltc_state->spiSeqPtr); /* Send dummy byte again to wake up the daisy chain */
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_START_INIT_INITIALIZATION,
                        LTC_STATEMACH_DAISY_CHAIN_SECOND_INITIALIZATION_TIME,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_RE_ENTRY_INITIALIZATION,
                        LTC_STATEMACH_SHORTTIME);
                } else if (ltc_state->substate == LTC_START_INIT_INITIALIZATION) {
                    LTC_SaveLastStates(ltc_state);
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_Init(
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength); /* Initialize main LTC loop */
                    ltc_state->lastSubstate = ltc_state->substate;
                    DIAG_CheckEvent(retVal, ltc_state->spiDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_StateTransition(
                        ltc_state,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_CHECK_INITIALIZATION,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT);
                } else if (ltc_state->substate == LTC_CHECK_INITIALIZATION) {
                    /* Read values written in config register, currently unused */
                    LTC_SaveLastStates(ltc_state);
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCFG,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength); /* Read config register */
                    LTC_StateTransition(
                        ltc_state,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_EXIT_INITIALIZATION,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT);
                } else if (ltc_state->substate == LTC_EXIT_INITIALIZATION) {
                    LTC_SaveLastStates(ltc_state);
                    ++ltc_state->spiSeqPtr;
                    ++ltc_state->currentString;
                    if (ltc_state->spiSeqPtr >= ltc_state->spiSeqEndPtr) {
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_INITIALIZED, LTC_ENTRY_INITIALIZATION, LTC_STATEMACH_SHORTTIME);
                    } else {
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_INITIALIZATION, LTC_ENTRY_INITIALIZATION, LTC_STATEMACH_SHORTTIME);
                    }
                }
                break;

            /****************************INITIALIZED*************************************/
            case LTC_STATEMACH_INITIALIZED:
                LTC_SaveLastStates(ltc_state);
                LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                break;

            /****************************START MEASUREMENT*******************************/
            case LTC_STATEMACH_STARTMEAS:

                ltc_state->adcMode   = LTC_VOLTAGE_MEASUREMENT_MODE;
                ltc_state->adcMeasCh = LTC_ADCMEAS_ALL_CHANNEL_CELLS;

                ltc_state->spiSeqPtr           = ltc_state->ltcData.pSpiInterface;
                ltc_state->spiNumberInterfaces = BS_NR_OF_STRINGS;
                ltc_state->spiSeqEndPtr        = ltc_state->ltcData.pSpiInterface + BS_NR_OF_STRINGS;
                ltc_state->currentString       = 0u;

                ltc_state->check_spi_flag = STD_NOT_OK;
                retVal = LTC_StartVoltageMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, ltc_state->adcMeasCh);

                LTC_CondBasedStateTransition(
                    ltc_state,
                    retVal,
                    ltc_state->spiDiagErrorEntry,
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    (ltc_state->commandTransferTime +
                     LTC_GetMeasurementTimeCycle(ltc_state->adcMode, ltc_state->adcMeasCh)),
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    LTC_STATEMACH_SHORTTIME);

                break;

            /****************************START MEASUREMENT CONTINUE*******************************/
            /* Do not reset SPI interface pointer */
            case LTC_STATEMACH_STARTMEAS_CONTINUE:

                ltc_state->adcMode   = LTC_VOLTAGE_MEASUREMENT_MODE;
                ltc_state->adcMeasCh = LTC_ADCMEAS_ALL_CHANNEL_CELLS;

                ltc_state->check_spi_flag = STD_NOT_OK;
                retVal = LTC_StartVoltageMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, ltc_state->adcMeasCh);

                LTC_CondBasedStateTransition(
                    ltc_state,
                    retVal,
                    ltc_state->spiDiagErrorEntry,
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    (ltc_state->commandTransferTime +
                     LTC_GetMeasurementTimeCycle(ltc_state->adcMode, ltc_state->adcMeasCh)),
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    LTC_STATEMACH_SHORTTIME);

                break;

            /****************************READ VOLTAGE************************************/
            case LTC_STATEMACH_READVOLTAGE:

                if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCVA,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_B_RDCVB_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_B_RDCVB_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_B_RDCVB_READVOLTAGE) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 0, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCVB,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_C_RDCVC_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_C_RDCVC_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_C_RDCVC_READVOLTAGE) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 1, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCVC,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_D_RDCVD_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_D_RDCVD_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_D_RDCVD_READVOLTAGE) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 2, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCVD,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    if (LTC_6813_MAX_SUPPORTED_CELLS > 12u) {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_READ_VOLTAGE_REGISTER_E_RDCVE_READVOLTAGE,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_READ_VOLTAGE_REGISTER_E_RDCVE_READVOLTAGE,
                            LTC_STATEMACH_SHORTTIME);
                    } else {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_EXIT_READVOLTAGE,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_EXIT_READVOLTAGE,
                            LTC_STATEMACH_SHORTTIME);
                    }
                    break;
                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_E_RDCVE_READVOLTAGE) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 3, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCVE,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    if (LTC_6813_MAX_SUPPORTED_CELLS > 15u) {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_READ_VOLTAGE_REGISTER_F_RDCVF_READVOLTAGE,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_READ_VOLTAGE_REGISTER_F_RDCVF_READVOLTAGE,
                            LTC_STATEMACH_SHORTTIME);
                    } else {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_EXIT_READVOLTAGE,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_READVOLTAGE,
                            LTC_EXIT_READVOLTAGE,
                            LTC_STATEMACH_SHORTTIME);
                    }
                    break;
                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_F_RDCVF_READVOLTAGE) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 4, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCVF,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_EXIT_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_EXIT_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EXIT_READVOLTAGE) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    if (LTC_6813_MAX_SUPPORTED_CELLS == 12u) {
                        LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 3, ltc_state->currentString);
                    } else if (LTC_6813_MAX_SUPPORTED_CELLS == 15u) {
                        LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 4, ltc_state->currentString);
                    } else if (LTC_6813_MAX_SUPPORTED_CELLS == 18u) {
                        LTC_SaveRxToVoltageBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 5, ltc_state->currentString);
                    }

                    /* Switch to different state if read voltage state is reused
                     * e.g. open-wire check...                                */
                    if (ltc_state->reusageMeasurementMode == LTC_NOT_REUSED) {
                        LTC_SaveVoltages(ltc_state, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_STATEMACH_MUXCONFIGURATION_INIT,
                            LTC_STATEMACH_SHORTTIME);
                    } else if (ltc_state->reusageMeasurementMode == LTC_REUSE_READVOLTAGE_FOR_ADOW_PUP) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_READ_VOLTAGES_PULLUP_OPENWIRE_CHECK,
                            LTC_STATEMACH_SHORTTIME);
                    } else if (ltc_state->reusageMeasurementMode == LTC_REUSE_READVOLTAGE_FOR_ADOW_PDOWN) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_READ_VOLTAGES_PULLDOWN_OPENWIRE_CHECK,
                            LTC_STATEMACH_SHORTTIME);
                    }
                    ltc_state->check_spi_flag = STD_NOT_OK;
                }
                break;

            /****************************MULTIPLEXED MEASUREMENT CONFIGURATION***********/
            case LTC_STATEMACH_MUXMEASUREMENT:

                ltc_state->adcMode   = LTC_VOLTAGE_MEASUREMENT_MODE;
                ltc_state->adcMeasCh = LTC_ADCMEAS_SINGLECHANNEL_GPIO1;

                if (ltc_state->substate == LTC_STATEMACH_MUXCONFIGURATION_INIT) {
                    ltc_state->adcMode   = LTC_GPIO_MEASUREMENT_MODE;
                    ltc_state->adcMeasCh = LTC_ADCMEAS_SINGLECHANNEL_GPIO1;

                    if (ltc_state->muxmeas_seqptr[ltc_state->currentString] >=
                        ltc_state->muxmeas_seqendptr[ltc_state->currentString]) {
                        /* last step of sequence reached (or no sequence configured) */

                        ltc_state->muxmeas_seqptr[ltc_state->currentString] = ltc_mux_seq.seqptr;
                        ltc_state->muxmeas_nr_end[ltc_state->currentString] = ltc_mux_seq.nr_of_steps;
                        ltc_state->muxmeas_seqendptr[ltc_state->currentString] =
                            ((LTC_MUX_CH_CFG_s *)ltc_mux_seq.seqptr) + ltc_mux_seq.nr_of_steps; /* last sequence + 1 */

                        LTC_SaveTemperatures(ltc_state, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SetMuxChannel(
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        ltc_state->muxmeas_seqptr[ltc_state->currentString]->muxID, /* mux */
                        ltc_state->muxmeas_seqptr[ltc_state->currentString]->muxCh /* channel */);
                    if (retVal != STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        ++ltc_state->muxmeas_seqptr[ltc_state->currentString];
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_STATEMACH_MUXCONFIGURATION_INIT,
                            LTC_STATEMACH_SHORTTIME);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_SEND_CLOCK_STCOMM_MUXMEASUREMENT_CONFIG,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT));
                    }
                    break;
                } else if (ltc_state->substate == LTC_SEND_CLOCK_STCOMM_MUXMEASUREMENT_CONFIG) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_I2cClock(ltc_state->spiSeqPtr);
                    if (LTC_GOTO_MUX_CHECK == true) {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_READ_I2C_TRANSMISSION_RESULT_RDCOMM_MUXMEASUREMENT_CONFIG,
                            (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_READ_I2C_TRANSMISSION_RESULT_RDCOMM_MUXMEASUREMENT_CONFIG,
                            LTC_STATEMACH_SHORTTIME);
                        ;
                    } else {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_STATEMACH_MUXMEASUREMENT,
                            (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_STATEMACH_MUXMEASUREMENT,
                            LTC_STATEMACH_SHORTTIME);
                    }
                    break;
                } else if (ltc_state->substate == LTC_READ_I2C_TRANSMISSION_RESULT_RDCOMM_MUXMEASUREMENT_CONFIG) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCOMM,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_MUXMEASUREMENT,
                        LTC_READ_I2C_TRANSMISSION_CHECK_MUXMEASUREMENT_CONFIG,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_MUXMEASUREMENT,
                        LTC_READ_I2C_TRANSMISSION_CHECK_MUXMEASUREMENT_CONFIG,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_READ_I2C_TRANSMISSION_CHECK_MUXMEASUREMENT_CONFIG) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);

                    /* if CRC OK: check multiplexer answer on i2C bus */
                    retVal = LTC_I2cCheckAcknowledge(
                        ltc_state,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->muxmeas_seqptr[ltc_state->currentString]->muxID,
                        ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->muxDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_MUXMEASUREMENT, LTC_STATEMACH_MUXMEASUREMENT, LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_STATEMACH_MUXMEASUREMENT) {
                    if (ltc_state->muxmeas_seqptr[ltc_state->currentString]->muxCh == 0xFF) {
                        /* actual multiplexer is switched off, so do not make a measurement and follow up with next step
                         * (mux configuration) */
                        ++ltc_state
                              ->muxmeas_seqptr[ltc_state->currentString]; /*  go further with next step of sequence
                                                                 ltc_state.numberOfMeasuredMux not decremented, this
                                                                 does not count as a measurement */
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        if (LTC_GOTO_MUX_CHECK == false) {
                            bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                            if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                                DIAG_Handler(
                                    ltc_state->spiDiagErrorEntry,
                                    DIAG_EVENT_NOT_OK,
                                    DIAG_STRING,
                                    ltc_state->currentString);
                            } else {
                                DIAG_Handler(
                                    ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                            }
                        }

                        ltc_state->check_spi_flag = STD_NOT_OK;
                        /* user multiplexer type -> connected to GPIO2! */
                        if ((ltc_state->muxmeas_seqptr[ltc_state->currentString]->muxID == 1) ||
                            (ltc_state->muxmeas_seqptr[ltc_state->currentString]->muxID == 2)) {
                            retVal = LTC_StartGpioMeasurement(
                                ltc_state->spiSeqPtr, ltc_state->adcMode, LTC_ADCMEAS_SINGLECHANNEL_GPIO2);
                        } else {
                            retVal = LTC_StartGpioMeasurement(
                                ltc_state->spiSeqPtr, ltc_state->adcMode, LTC_ADCMEAS_SINGLECHANNEL_GPIO1);
                        }
                    }
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_MUXMEASUREMENT,
                        LTC_STATEMACH_READMUXMEASUREMENT,
                        (ltc_state->commandTransferTime +
                         LTC_GetMeasurementTimeCycle(
                             ltc_state->adcMode, LTC_ADCMEAS_SINGLECHANNEL_GPIO2)), /*  wait, ADAX-Command */
                        LTC_STATEMACH_MUXMEASUREMENT,
                        LTC_STATEMACH_READMUXMEASUREMENT,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_STATEMACH_READMUXMEASUREMENT) {
                    ltc_state->check_spi_flag = STD_OK;

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDAUXA,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_MUXMEASUREMENT,
                        LTC_STATEMACH_STOREMUXMEASUREMENT,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_MUXMEASUREMENT,
                        LTC_STATEMACH_STOREMUXMEASUREMENT,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_STATEMACH_STOREMUXMEASUREMENT) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveMuxMeasurement(
                        ltc_state,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->muxmeas_seqptr[ltc_state->currentString],
                        ltc_state->currentString);

                    ++ltc_state->muxmeas_seqptr[ltc_state->currentString];

                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_MEASCYCLE_FINISHED, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    break;
                }

                break;

            /****************************END OF MEASUREMENT CYCLE************************/
            case LTC_STATEMACH_MEASCYCLE_FINISHED:

                if (ltc_state->balance_control_done == STD_OK) {
                    if (LTC_IsFirstMeasurementCycleFinished(ltc_state) == false) {
                        LTC_SetFirstMeasurementCycleFinished(ltc_state);
                    }
                    statereq = LTC_TransferStateRequest(ltc_state, &tmpbusID, &tmpadcMode, &tmpadcMeasCh);
                    if (statereq.request == LTC_STATE_USER_IO_WRITE_REQUEST) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_USER_IO_CONTROL,
                            LTC_USER_IO_SET_OUTPUT_REGISTER,
                            LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else if (statereq.request == LTC_STATE_USER_IO_READ_REQUEST) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_USER_IO_FEEDBACK,
                            LTC_USER_IO_READ_INPUT_REGISTER,
                            LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else if (statereq.request == LTC_STATE_USER_IO_WRITE_REQUEST_TI) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_USER_IO_CONTROL_TI,
                            LTC_USER_IO_SET_DIRECTION_REGISTER_TI,
                            LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else if (statereq.request == LTC_STATE_USER_IO_READ_REQUEST_TI) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                            LTC_USER_IO_SET_DIRECTION_REGISTER_TI,
                            LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else if (statereq.request == LTC_STATE_EEPROM_READ_REQUEST) {
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_EEPROM_READ, LTC_EEPROM_READ_DATA1, LTC_STATEMACH_SHORTTIME);
                    } else if (statereq.request == LTC_STATE_EEPROM_WRITE_REQUEST) {
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_EEPROM_WRITE, LTC_EEPROM_WRITE_DATA1, LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else if (statereq.request == LTC_STATE_TEMP_SENS_READ_REQUEST) {
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_TEMP_SENS_READ, LTC_TEMP_SENS_SEND_DATA1, LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else if (statereq.request == LTC_STATEMACH_BALANCE_FEEDBACK_REQUEST) {
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_BALANCE_FEEDBACK, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else if (statereq.request == LTC_STATE_OPENWIRE_CHECK_REQUEST) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_REQUEST_PULLUP_CURRENT_OPENWIRE_CHECK,
                            LTC_STATEMACH_SHORTTIME);
                        /* Send ADOW command with PUP two times */
                        ltc_state->resendCommandCounter = LTC_NUMBER_REQ_ADOW_COMMANDS;
                        ltc_state->balance_control_done = STD_NOT_OK;
                    } else {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_BALANCE_CONTROL,
                            LTC_CONFIG_BALANCE_CONTROL,
                            LTC_STATEMACH_SHORTTIME);
                        ltc_state->balance_control_done = STD_NOT_OK;
                    }
                } else {
                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_BALANCE_CONTROL, LTC_CONFIG_BALANCE_CONTROL, LTC_STATEMACH_SHORTTIME);
                }

                break;

            /****************************BALANCE CONTROL*********************************/
            case LTC_STATEMACH_BALANCE_CONTROL:

                if (ltc_state->substate == LTC_CONFIG_BALANCE_CONTROL) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_BalanceControl(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        0u,
                        ltc_state->currentString);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_BALANCE_CONTROL,
                        LTC_CONFIG2_BALANCE_CONTROL,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_BALANCE_CONTROL,
                        LTC_CONFIG2_BALANCE_CONTROL,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_CONFIG2_BALANCE_CONTROL) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    if (BS_NR_OF_CELL_BLOCKS_PER_MODULE > 12) {
                        AFE_SetTransmitOngoing(ltc_state);
                        retVal = LTC_BalanceControl(
                            ltc_state,
                            ltc_state->spiSeqPtr,
                            ltc_state->ltcData.txBuffer,
                            ltc_state->ltcData.rxBuffer,
                            ltc_state->ltcData.frameLength,
                            1u,
                            ltc_state->currentString);
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_BALANCE_CONTROL,
                            LTC_CONFIG2_BALANCE_CONTROL_END,
                            ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                            LTC_STATEMACH_BALANCE_CONTROL,
                            LTC_CONFIG2_BALANCE_CONTROL_END,
                            LTC_STATEMACH_SHORTTIME);
                    } else {
                        /* 12 cells, balancing control finished */
                        ltc_state->check_spi_flag = STD_NOT_OK;
                        ++ltc_state->spiSeqPtr;
                        ++ltc_state->currentString;
                        if (ltc_state->spiSeqPtr >= ltc_state->spiSeqEndPtr) {
                            ltc_state->balance_control_done = STD_OK;
                            LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        } else {
                            LTC_StateTransition(
                                ltc_state, LTC_STATEMACH_STARTMEAS_CONTINUE, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        }
                    }

                    break;
                } else if (ltc_state->substate == LTC_CONFIG2_BALANCE_CONTROL_END) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }
                    /* More than 12 cells, balancing control finished */
                    ltc_state->check_spi_flag = STD_NOT_OK;
                    ++ltc_state->spiSeqPtr;
                    ++ltc_state->currentString;
                    if (ltc_state->spiSeqPtr >= ltc_state->spiSeqEndPtr) {
                        ltc_state->balance_control_done = STD_OK;
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    } else {
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_STARTMEAS_CONTINUE, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    }

                    break;
                }
                break;

            /****************************START MEASUREMENT*******************************/
            case LTC_STATEMACH_ALL_GPIO_MEASUREMENT:

                ltc_state->adcMode   = LTC_GPIO_MEASUREMENT_MODE;
                ltc_state->adcMeasCh = LTC_ADCMEAS_ALL_CHANNEL_GPIOS;

                ltc_state->check_spi_flag = STD_NOT_OK;
                retVal = LTC_StartGpioMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, ltc_state->adcMeasCh);
                LTC_CondBasedStateTransition(
                    ltc_state,
                    retVal,
                    ltc_state->spiDiagErrorEntry,
                    LTC_STATEMACH_READALLGPIO,
                    LTC_READ_AUXILIARY_REGISTER_A_RDAUXA,
                    (ltc_state->commandTransferTime +
                     LTC_GetMeasurementTimeCycle(ltc_state->adcMode, ltc_state->adcMeasCh)),
                    LTC_STATEMACH_ALL_GPIO_MEASUREMENT,
                    LTC_ENTRY,
                    LTC_STATEMACH_SHORTTIME); /* TODO: here same state is kept if error occurs */
                break;

            /****************************READ ALL GPIO VOLTAGE************************************/
            case LTC_STATEMACH_READALLGPIO:

                if (ltc_state->substate == LTC_READ_AUXILIARY_REGISTER_A_RDAUXA) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDAUXA,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_READALLGPIO,
                        LTC_READ_AUXILIARY_REGISTER_B_RDAUXB,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READALLGPIO,
                        LTC_READ_AUXILIARY_REGISTER_B_RDAUXB,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_READ_AUXILIARY_REGISTER_B_RDAUXB) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToGpioBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 0, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDAUXB,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);

                    if (LTC_6813_MAX_SUPPORTED_CELLS > 12u) {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_READALLGPIO,
                            LTC_READ_AUXILIARY_REGISTER_C_RDAUXC,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_READALLGPIO,
                            LTC_READ_AUXILIARY_REGISTER_C_RDAUXC,
                            LTC_STATEMACH_SHORTTIME);
                    } else {
                        LTC_CondBasedStateTransition(
                            ltc_state,
                            retVal,
                            ltc_state->spiDiagErrorEntry,
                            LTC_STATEMACH_READALLGPIO,
                            LTC_EXIT_READAUXILIARY_ALL_GPIOS,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                            LTC_STATEMACH_READALLGPIO,
                            LTC_EXIT_READAUXILIARY_ALL_GPIOS,
                            LTC_STATEMACH_SHORTTIME);
                    }
                    break;
                } else if (ltc_state->substate == LTC_READ_AUXILIARY_REGISTER_C_RDAUXC) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToGpioBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 1, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDAUXC,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_READALLGPIO,
                        LTC_READ_AUXILIARY_REGISTER_D_RDAUXD,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READALLGPIO,
                        LTC_READ_AUXILIARY_REGISTER_D_RDAUXD,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_READ_AUXILIARY_REGISTER_D_RDAUXD) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRxToGpioBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 2, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDAUXD,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_READALLGPIO,
                        LTC_EXIT_READAUXILIARY_ALL_GPIOS,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READALLGPIO,
                        LTC_EXIT_READAUXILIARY_ALL_GPIOS,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EXIT_READAUXILIARY_ALL_GPIOS) {
                    retVal = LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, ltc_state->pecDiagErrorEntry, DIAG_STRING, ltc_state->currentString);

                    if (LTC_6813_MAX_SUPPORTED_CELLS == 12u) {
                        LTC_SaveRxToGpioBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 1, ltc_state->currentString);
                    } else if (LTC_6813_MAX_SUPPORTED_CELLS > 12u) {
                        LTC_SaveRxToGpioBuffer(ltc_state, ltc_state->ltcData.rxBuffer, 3, ltc_state->currentString);
                    }

                    LTC_SaveAllGpioMeasurement(ltc_state);

                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_MEASCYCLE_FINISHED, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                }

                break;

            /****************************BALANCE FEEDBACK*********************************/
            case LTC_STATEMACH_BALANCE_FEEDBACK:

                ltc_state->adcMode   = LTC_GPIO_MEASUREMENT_MODE;
                ltc_state->adcMeasCh = LTC_ADCMEAS_SINGLECHANNEL_GPIO3;

                if (ltc_state->substate == LTC_ENTRY) {
                    ltc_state->spiSeqPtr = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->adcMode   = LTC_ADCMODE_NORMAL_DCP0;
                    ltc_state->adcMeasCh = LTC_ADCMEAS_SINGLECHANNEL_GPIO3;

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal = LTC_StartGpioMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, ltc_state->adcMeasCh);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_BALANCE_FEEDBACK,
                        LTC_READ_FEEDBACK_BALANCE_CONTROL,
                        (ltc_state->commandDataTransferTime +
                         LTC_GetMeasurementTimeCycle(ltc_state->adcMode, ltc_state->adcMeasCh)),
                        LTC_STATEMACH_BALANCE_FEEDBACK,
                        LTC_READ_FEEDBACK_BALANCE_CONTROL,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_READ_FEEDBACK_BALANCE_CONTROL) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDAUXA,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength); /* read AUXA register */
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_BALANCE_FEEDBACK,
                        LTC_SAVE_FEEDBACK_BALANCE_CONTROL,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_BALANCE_FEEDBACK,
                        LTC_SAVE_FEEDBACK_BALANCE_CONTROL,
                        LTC_STATEMACH_SHORTTIME);
                } else if (ltc_state->substate == LTC_SAVE_FEEDBACK_BALANCE_CONTROL) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    if (LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString) != STD_OK) {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_SaveBalancingFeedback(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    }
                    LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    break;
                }
                break;

            /****************************BOARD TEMPERATURE SENSOR*********************************/
            case LTC_STATEMACH_TEMP_SENS_READ:

                if (ltc_state->substate == LTC_TEMP_SENS_SEND_DATA1) {
                    ltc_state->spiSeqPtr      = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SendI2cCommand(
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        ltc_I2CcmdTempSens0);

                    if (retVal != STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        ++ltc_state->muxmeas_seqptr[ltc_state->requestedString];
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_TEMP_SENS_READ,
                            LTC_TEMP_SENS_SEND_CLOCK_STCOMM1,
                            ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT);
                    }

                    break;
                } else if (ltc_state->substate == LTC_TEMP_SENS_SEND_CLOCK_STCOMM1) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_TEMP_SENS_READ,
                        LTC_TEMP_SENS_READ_DATA1,
                        (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_TEMP_SENS_READ,
                        LTC_TEMP_SENS_READ_DATA1,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_TEMP_SENS_READ_DATA1) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SendI2cCommand(
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        ltc_I2CcmdTempSens1);

                    if (retVal != STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        ++ltc_state->muxmeas_seqptr[ltc_state->requestedString];
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_TEMP_SENS_READ,
                            LTC_TEMP_SENS_SEND_CLOCK_STCOMM2,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT));
                    }

                    break;
                } else if (ltc_state->substate == LTC_TEMP_SENS_SEND_CLOCK_STCOMM2) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_TEMP_SENS_READ,
                        LTC_TEMP_SENS_READ_I2C_TRANSMISSION_RESULT_RDCOMM,
                        (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_TEMP_SENS_READ,
                        LTC_TEMP_SENS_READ_I2C_TRANSMISSION_RESULT_RDCOMM,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_TEMP_SENS_READ_I2C_TRANSMISSION_RESULT_RDCOMM) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCOMM,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_TEMP_SENS_READ,
                        LTC_TEMP_SENS_SAVE_TEMP,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_TEMP_SENS_READ,
                        LTC_TEMP_SENS_SAVE_TEMP,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_TEMP_SENS_SAVE_TEMP) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    if (LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString) != STD_OK) {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_TempSensSaveTemp(ltc_state, ltc_state->ltcData.rxBuffer);
                    }

                    LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    break;
                }
                break;

            /****************************WRITE TO PORT EXPANDER IO***********/
            case LTC_STATEMACH_USER_IO_CONTROL:

                if (ltc_state->substate == LTC_USER_IO_SET_OUTPUT_REGISTER) {
                    ltc_state->spiSeqPtr      = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SetPortExpander(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);

                    if (retVal != STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        ++ltc_state->muxmeas_seqptr[ltc_state->requestedString];
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_USER_IO_CONTROL,
                            LTC_SEND_CLOCK_STCOMM_MUXMEASUREMENT_CONFIG,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT));
                    }
                    break;
                } else if (ltc_state->substate == LTC_SEND_CLOCK_STCOMM_MUXMEASUREMENT_CONFIG) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal                    = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        ltc_state->gpioClocksTransferTime,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                }
                break;

            /****************************READ FROM PORT EXPANDER IO***********/
            case LTC_STATEMACH_USER_IO_FEEDBACK:

                if (ltc_state->substate == LTC_USER_IO_READ_INPUT_REGISTER) {
                    ltc_state->spiSeqPtr      = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SendI2cCommand(
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        ltc_I2CcmdPortExpander1);

                    if (retVal != STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        ++ltc_state->muxmeas_seqptr[ltc_state->requestedString];
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_USER_IO_FEEDBACK,
                            LTC_USER_IO_SEND_CLOCK_STCOMM,
                            ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT);
                    }

                    break;
                } else if (ltc_state->substate == LTC_USER_IO_SEND_CLOCK_STCOMM) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal                    = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK,
                        LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM,
                        ltc_state->gpioClocksTransferTime,
                        LTC_STATEMACH_USER_IO_FEEDBACK,
                        LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCOMM,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK,
                        LTC_USER_IO_SAVE_DATA,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_USER_IO_FEEDBACK,
                        LTC_USER_IO_SAVE_DATA,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_SAVE_DATA) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    if (LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString) != STD_OK) {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_PortExpanderSaveValues(ltc_state, ltc_state->ltcData.rxBuffer);
                    }

                    LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    break;
                }

                break;

            /****************************WRITE TO TI PORT EXPANDER IO***********/
            case LTC_STATEMACH_USER_IO_CONTROL_TI:

                if (ltc_state->substate == LTC_USER_IO_SET_DIRECTION_REGISTER_TI) {
                    ltc_state->spiSeqPtr      = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SetPortExpanderDirectionTi(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        LTC_PORT_EXPANDER_TI_OUTPUT);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_CONTROL_TI,
                        LTC_USER_IO_SEND_CLOCK_STCOMM_TI,
                        LTC_STATEMACH_SHORTTIME,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_SEND_CLOCK_STCOMM_TI) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal                    = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_CONTROL_TI,
                        LTC_USER_IO_SET_OUTPUT_REGISTER_TI,
                        ltc_state->gpioClocksTransferTime,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_SET_OUTPUT_REGISTER_TI) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SetPortExpanderOutputTi(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_CONTROL_TI,
                        LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_SECOND,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_SECOND) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal                    = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        ltc_state->gpioClocksTransferTime,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                }
                break;

            /****************************READ TI PORT EXPANDER IO***********/
            case LTC_STATEMACH_USER_IO_FEEDBACK_TI:

                if (ltc_state->substate == LTC_USER_IO_SET_DIRECTION_REGISTER_TI) {
                    ltc_state->spiSeqPtr      = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SetPortExpanderDirectionTi(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        LTC_PORT_EXPANDER_TI_INPUT);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_SEND_CLOCK_STCOMM_TI,
                        LTC_STATEMACH_SHORTTIME,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_SEND_CLOCK_STCOMM_TI) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal                    = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_READ_INPUT_REGISTER_TI_FIRST,
                        ltc_state->gpioClocksTransferTime,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_READ_INPUT_REGISTER_TI_FIRST) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_GetPortExpanderInputTi(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        0);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_SECOND,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_SECOND) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal                    = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_READ_INPUT_REGISTER_TI_SECOND,
                        ltc_state->gpioClocksTransferTime,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_READ_INPUT_REGISTER_TI_SECOND) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_GetPortExpanderInputTi(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        1);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_THIRD,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_THIRD) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    ltc_state->check_spi_flag = STD_NOT_OK;
                    retVal                    = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_FOURTH,
                        ltc_state->gpioClocksTransferTime,
                        LTC_STATEMACH_STARTMEAS,
                        LTC_ENTRY,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_FOURTH) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCOMM,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_SAVE_DATA_TI,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT,
                        LTC_STATEMACH_USER_IO_FEEDBACK_TI,
                        LTC_USER_IO_SAVE_DATA_TI,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_USER_IO_SAVE_DATA_TI) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    if (LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString) != STD_OK) {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_PortExpanderSaveValuesTi(ltc_state, ltc_state->ltcData.txBuffer);
                    }

                    LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    break;
                }

                break;

            /****************************EEPROM READ*********************************/
            case LTC_STATEMACH_EEPROM_READ:

                if (ltc_state->substate == LTC_EEPROM_READ_DATA1) {
                    ltc_state->spiSeqPtr      = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SendEepromReadCommand(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        0);

                    if (retVal != STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        ++ltc_state->muxmeas_seqptr[ltc_state->requestedString];
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_EEPROM_READ,
                            LTC_EEPROM_SEND_CLOCK_STCOMM1,
                            ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT);
                    }

                    break;
                } else if (ltc_state->substate == LTC_EEPROM_SEND_CLOCK_STCOMM1) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_READ_DATA2,
                        (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_READ_DATA2,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EEPROM_READ_DATA2) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SendEepromReadCommand(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        1);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_SEND_CLOCK_STCOMM2,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_SEND_CLOCK_STCOMM2,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EEPROM_SEND_CLOCK_STCOMM2) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_READ_I2C_TRANSMISSION_RESULT_RDCOMM,
                        (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_READ_I2C_TRANSMISSION_RESULT_RDCOMM,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EEPROM_READ_I2C_TRANSMISSION_RESULT_RDCOMM) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_ReadRegister(
                        ltc_cmdRDCOMM,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_SAVE_READ,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_EEPROM_READ,
                        LTC_EEPROM_SAVE_READ,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EEPROM_SAVE_READ) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    if (LTC_CheckPec(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString) != STD_OK) {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                    } else {
                        DIAG_Handler(
                            ltc_state->pecDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_EepromSaveReadValue(ltc_state, ltc_state->ltcData.rxBuffer);
                    }
                    LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    break;
                }

                break;

            /****************************EEPROM READ*********************************/
            case LTC_STATEMACH_EEPROM_WRITE:

                if (ltc_state->substate == LTC_EEPROM_WRITE_DATA1) {
                    ltc_state->spiSeqPtr      = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SendEepromWriteCommand(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        0);

                    if (retVal != STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        ++ltc_state->muxmeas_seqptr[ltc_state->requestedString];
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_EEPROM_WRITE,
                            LTC_EEPROM_SEND_CLOCK_STCOMM3,
                            (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT));
                    }

                    break;
                } else if (ltc_state->substate == LTC_EEPROM_SEND_CLOCK_STCOMM3) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_EEPROM_WRITE,
                        LTC_EEPROM_WRITE_DATA2,
                        (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_EEPROM_WRITE,
                        LTC_EEPROM_WRITE_DATA2,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EEPROM_WRITE_DATA2) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_SendEepromWriteCommand(
                        ltc_state,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength,
                        1);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_EEPROM_WRITE,
                        LTC_EEPROM_SEND_CLOCK_STCOMM4,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_EEPROM_WRITE,
                        LTC_EEPROM_SEND_CLOCK_STCOMM4,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EEPROM_SEND_CLOCK_STCOMM4) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_I2cClock(ltc_state->spiSeqPtr);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        ltc_state->spiDiagErrorEntry,
                        LTC_STATEMACH_EEPROM_WRITE,
                        LTC_EEPROM_FINISHED,
                        (ltc_state->gpioClocksTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_EEPROM_WRITE,
                        LTC_EEPROM_FINISHED,
                        LTC_STATEMACH_SHORTTIME);
                    break;
                } else if (ltc_state->substate == LTC_EEPROM_FINISHED) {
                    bool transmitOngoing = AFE_IsTransmitOngoing(ltc_state);
                    if ((ltc_state->timer == 0) && (transmitOngoing == true)) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                        break;
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                    }
                    LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    break;
                }

                break;

            /**************************OPEN-WIRE CHECK*******************************/
            case LTC_STATEMACH_OPENWIRE_CHECK:
                ltc_state->spiSeqPtr = ltc_state->ltcData.pSpiInterface + ltc_state->requestedString;
                /* This is necessary because the state machine will go through read voltage measurement registers */
                ltc_state->currentString = ltc_state->requestedString;
                if (ltc_state->substate == LTC_REQUEST_PULLUP_CURRENT_OPENWIRE_CHECK) {
                    /* Run ADOW command with PUP = 1 */
                    ltc_state->adcMode        = LTC_OW_MEASUREMENT_MODE;
                    ltc_state->check_spi_flag = STD_NOT_OK;

                    retVal = LTC_StartOpenWireMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, 1);
                    if (retVal == STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_REQUEST_PULLUP_CURRENT_OPENWIRE_CHECK,
                            (ltc_state->commandDataTransferTime +
                             LTC_GetMeasurementTimeCycle(ltc_state->adcMode, LTC_ADCMEAS_ALL_CHANNEL_CELLS)));
                        ltc_state->resendCommandCounter--;

                        /* Check how many retries are left */
                        if (ltc_state->resendCommandCounter == 0) {
                            /* Switch to read voltage state to read cell voltages */
                            LTC_StateTransition(
                                ltc_state,
                                LTC_STATEMACH_READVOLTAGE,
                                LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                                (ltc_state->commandDataTransferTime +
                                 LTC_GetMeasurementTimeCycle(ltc_state->adcMode, LTC_ADCMEAS_ALL_CHANNEL_CELLS)));
                            /* Reuse read voltage register */
                            ltc_state->reusageMeasurementMode = LTC_REUSE_READVOLTAGE_FOR_ADOW_PUP;
                        }
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_STARTMEAS_CONTINUE, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    }
                } else if (ltc_state->substate == LTC_READ_VOLTAGES_PULLUP_OPENWIRE_CHECK) {
                    /* Previous state: Read voltage -> information stored in voltage buffer */
                    ltc_state->reusageMeasurementMode = LTC_NOT_REUSED;

                    /* Copy data from voltage struct into open-wire struct */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                            ltc_state->ltcData.openWireDetection
                                ->openWirePup[ltc_state->requestedString][(m * BS_NR_OF_CELL_BLOCKS_PER_MODULE) + cb] =
                                ltc_state->ltcData.cellVoltage->cellVoltage_mV[ltc_state->requestedString][m][cb];
                        }
                    }

                    /* Set number of ADOW retries - send ADOW command with pull-down two times */
                    ltc_state->resendCommandCounter = LTC_NUMBER_REQ_ADOW_COMMANDS;
                    LTC_StateTransition(
                        ltc_state,
                        LTC_STATEMACH_OPENWIRE_CHECK,
                        LTC_REQUEST_PULLDOWN_CURRENT_OPENWIRE_CHECK,
                        LTC_STATEMACH_SHORTTIME);
                } else if (ltc_state->substate == LTC_REQUEST_PULLDOWN_CURRENT_OPENWIRE_CHECK) {
                    /* Run ADOW command with PUP = 0 */
                    ltc_state->adcMode        = LTC_OW_MEASUREMENT_MODE;
                    ltc_state->check_spi_flag = STD_NOT_OK;

                    retVal = LTC_StartOpenWireMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, 0);
                    if (retVal == STD_OK) {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_REQUEST_PULLDOWN_CURRENT_OPENWIRE_CHECK,
                            (ltc_state->commandDataTransferTime +
                             LTC_GetMeasurementTimeCycle(ltc_state->adcMode, LTC_ADCMEAS_ALL_CHANNEL_CELLS)));
                        ltc_state->resendCommandCounter--;

                        /* Check how many retries are left */
                        if (ltc_state->resendCommandCounter == 0) {
                            /* Switch to read voltage state to read cell voltages */
                            LTC_StateTransition(
                                ltc_state,
                                LTC_STATEMACH_READVOLTAGE,
                                LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                                (ltc_state->commandDataTransferTime +
                                 LTC_GetMeasurementTimeCycle(ltc_state->adcMode, LTC_ADCMEAS_ALL_CHANNEL_CELLS)));
                            /* Reuse read voltage register */
                            ltc_state->reusageMeasurementMode = LTC_REUSE_READVOLTAGE_FOR_ADOW_PDOWN;
                        }
                    } else {
                        DIAG_Handler(
                            ltc_state->spiDiagErrorEntry, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->currentString);
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_STARTMEAS_CONTINUE, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    }
                } else if (ltc_state->substate == LTC_READ_VOLTAGES_PULLDOWN_OPENWIRE_CHECK) {
                    /* Previous state: Read voltage -> information stored in voltage buffer */
                    ltc_state->reusageMeasurementMode = LTC_NOT_REUSED;

                    /* Copy data from voltage struct into open-wire struct */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                            ltc_state->ltcData.openWireDetection
                                ->openWirePdown[ltc_state->requestedString]
                                               [(m * BS_NR_OF_CELL_BLOCKS_PER_MODULE) + cb] =
                                ltc_state->ltcData.cellVoltage->cellVoltage_mV[ltc_state->requestedString][m][cb];
                        }
                    }
                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_OPENWIRE_CHECK, LTC_PERFORM_OPENWIRE_CHECK, LTC_STATEMACH_SHORTTIME);
                } else if (ltc_state->substate == LTC_PERFORM_OPENWIRE_CHECK) {
                    /* Perform actual open-wire check */
                    for (uint8_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        /* Open-wire at C0: cell_pup(0) == 0 */
                        if (ltc_state->ltcData.openWireDetection
                                ->openWirePup[ltc_state->requestedString][0 + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE)] ==
                            0u) {
                            ltc_state->ltcData.openWire->openWire[ltc_state->requestedString]
                                                                 [0 + (m * (BS_NR_OF_CELL_BLOCKS_PER_MODULE))] = 1u;
                        }
                        /* Open-wire at Cmax: cell_pdown(BS_NR_OF_CELL_BLOCKS_PER_MODULE-1) == 0 */
                        if (ltc_state->ltcData.openWireDetection->openWirePdown[ltc_state->requestedString][(
                                (BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1) + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE))] == 0u) {
                            ltc_state->ltcData.openWire
                                ->openWire[ltc_state->requestedString]
                                          [BS_NR_OF_CELL_BLOCKS_PER_MODULE + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE)] =
                                1u;
                        }
                    }

                    /* Take difference between pull-up and pull-down measurement */
                    for (uint16_t i = 1u; i < BS_NR_OF_CELL_BLOCKS_PER_STRING; i++) {
                        ltc_state->ltcData.openWireDetection->openWireDelta[ltc_state->requestedString][i] =
                            ltc_state->ltcData.openWireDetection->openWirePup[ltc_state->requestedString][i] -
                            ltc_state->ltcData.openWireDetection->openWirePdown[ltc_state->requestedString][i];
                    }

                    /* Open-wire at C(N): delta cell(n+1) < -400mV */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
                        for (uint8_t c = 1u; c < (BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1); c++) {
                            if (ltc_state->ltcData.openWireDetection
                                    ->openWireDelta[ltc_state->requestedString]
                                                   [c + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE)] < LTC_ADOW_THRESHOLD) {
                                ltc_state->ltcData.openWire->openWire[ltc_state->requestedString]
                                                                     [c + (m * BS_NR_OF_CELL_BLOCKS_PER_MODULE)] = 1;
                            }
                        }
                    }

                    /* Write database entry */
                    DATA_WRITE_DATA(ltc_state->ltcData.openWire);
                    /* Start new measurement cycle */
                    LTC_StateTransition(ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                }
                break;

            /****************************DEFAULT**************************/
            default:
                /* invalid state */
                FAS_ASSERT(FAS_TRAP);
                break;
        }

        ltc_state->triggerentry--; /* reentrance counter */
    } /* continueFunction */
}

/**
 * @brief   saves the multiplexer values read from the LTC daisy-chain.
 *
 * After a voltage measurement was initiated on GPIO 1 to read the currently
 * selected multiplexer voltage, the results is read via SPI from the
 * daisy-chain.
 * This function is called to store the result from the transmission in a
 * buffer.
 *
 * @param   ltc_state   state of the ltc state machine
 * @param   pRxBuff     receive buffer
 * @param   muxseqptr   pointer to the multiplexer sequence, which
 *                      configures the currently selected multiplexer ID and
 *                      channel
 * @param  stringNumber string addressed
 */
static void LTC_SaveMuxMeasurement(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    LTC_MUX_CH_CFG_s *muxseqptr,
    uint8_t stringNumber) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    FAS_ASSERT(muxseqptr != NULL_PTR);
    uint16_t val_ui           = 0;
    int16_t temperature_ddegC = 0;
    uint8_t sensor_idx        = 0;
    uint8_t ch_idx            = 0;
    uint16_t buffer_LSB       = 0;
    uint16_t buffer_MSB       = 0;

    /* pointer to measurement Sequence of Mux- and Channel-Configurations (1,0xFF)...(3,0xFF),(0,1),...(0,7)) */
    /* Channel 0xFF means that the multiplexer is deactivated, therefore no measurement will be made and saved*/
    if (muxseqptr->muxCh != 0xFF) {
        /* user multiplexer type -> connected to GPIO2! */
        if ((muxseqptr->muxID == 1) || (muxseqptr->muxID == 2)) {
            for (uint16_t i = 0; i < LTC_N_LTC; i++) {
                if (muxseqptr->muxID == 1) {
                    ch_idx = 0 + muxseqptr->muxCh; /* channel index 0..7 */
                } else {
                    ch_idx = 8 + muxseqptr->muxCh; /* channel index 8..15 */
                }

                if (ch_idx < (2u * 8u)) {
                    val_ui = *((uint16_t *)(&pRxBuff[6u + (1u * i * 8u)])); /* raw values, all mux on all LTCs */
                    /* ltc_user_mux.value[i*8*2+ch_idx] = (uint16_t)(((float_t)(val_ui))*100e-6f*1000.0f); */ /* Unit ->
                                                                                                                 in V ->
                                                                                                                 in mV
                                                                                                               */
                }
            }
        } else {
            /* temperature multiplexer type -> connected to GPIO1! */
            for (uint16_t i = 0; i < LTC_N_LTC; i++) {
                buffer_MSB = pRxBuff[4u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[4u + (i * 8u)];
                val_ui     = buffer_LSB | (buffer_MSB << 8);
                /* val_ui = *((uint16_t *)(&pRxBuff[4+i*8])); */
                /* GPIO voltage in 100uV -> * 0.1 ----  conversion to mV */
                temperature_ddegC = LTC_ConvertMuxVoltagesToTemperatures(val_ui / 10u); /* unit: deci &deg;C */
                sensor_idx        = ltc_muxSensorTemperature_cfg[muxseqptr->muxCh];
                /* wrong configuration! */
                if (sensor_idx >= BS_NR_OF_TEMP_SENSORS_PER_MODULE) {
                    FAS_ASSERT(FAS_TRAP);
                }
                /* Set bitmask for valid flags */

                /* Check LTC PEC error */
                if (ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] == true) {
                    /* Reset invalid flag */
                    ltc_state->ltcData.cellTemperature->invalidCellTemperature[stringNumber][i][sensor_idx] = false;

                    ltc_state->ltcData.cellTemperature->cellTemperature_ddegC[stringNumber][i][sensor_idx] =
                        temperature_ddegC;
                } else {
                    /* Set invalid flag */
                    ltc_state->ltcData.cellTemperature->invalidCellTemperature[stringNumber][i][sensor_idx] = true;
                }
            }
        }
    }
}

/**
 * @brief   saves the voltage values read from the LTC daisy-chain.
 *
 * After a voltage measurement was initiated to measure the voltages of the cells,
 * the result is read via SPI from the daisy-chain.
 * There are 6 register to read _(A,B,C,D,E,F) to get all cell voltages.
 * Only one register can be read at a time.
 * This function is called to store the result from the transmission in a buffer.
 *
 * @param   ltc_state      state of the ltc state machine
 * @param   pRxBuff        receive buffer
 * @param   registerSet    voltage register that was read (voltage register A,B,C,D,E or F)
 * @param   stringNumber    string addressed
 *
 */
static void LTC_SaveRxToVoltageBuffer(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t registerSet,
    uint8_t stringNumber) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    uint16_t cellOffset    = 0;
    uint16_t voltage_index = 0;
    uint16_t val_ui        = 0;
    uint16_t voltage       = 0;
    uint16_t buffer_LSB    = 0;
    uint16_t buffer_MSB    = 0;
    bool continueFunction  = true;

    if (registerSet <= 5u) {
        /* RDCVA command -> voltage register group A */
        /* RDCVB command -> voltage register group B */
        /* RDCVC command -> voltage register group C */
        /* RDCVD command -> voltage register group D */
        /* RDCVE command -> voltage register group E (only LTC6812-1 and LTC6813-1) */
        /* RDCVF command -> voltage register group F (only LTC6813-1) */
        cellOffset = registerSet * LTC_NUMBER_OF_CELL_VOLTAGES_PER_REGISTER;
    } else {
        continueFunction = false;
    }

    if (continueFunction == true) {
        /* reinitialize index counter at begin of cycle */
        if (cellOffset == 0u) {
            (ltc_state->ltcData.usedCellIndex[stringNumber]) = 0;
        }

        /* Retrieve data without command and CRC*/
        for (uint16_t m = 0u; m < LTC_N_LTC; m++) {
            uint16_t incrementation = 0u;

            /* parse all three voltages (3 * 2bytes) contained in one register */
            for (uint8_t c = 0u; c < LTC_NUMBER_OF_CELL_VOLTAGES_PER_REGISTER; c++) {
                /* index considering maximum number of cells */
                voltage_index = c + cellOffset;

                if (ltc_voltage_input_used[voltage_index] == 1u) {
                    buffer_MSB = pRxBuff[4u + (2u * c) + (m * 8u) + 1u];
                    buffer_LSB = pRxBuff[4u + (2u * c) + (m * 8u)];
                    val_ui     = buffer_LSB | (buffer_MSB << 8u);
                    /* val_ui = *((uint16_t *)(&pRxBuff[4+2*j+i*8])); */
                    voltage = ((val_ui)) * 100e-6f * 1000.0f; /* Unit V -> in mV */

                    /* Check PEC for every LTC in the daisy-chain */
                    if (ltc_state->ltcData.errorTable->PEC_valid[stringNumber][m] == true) {
                        ltc_state->ltcData.cellVoltage
                            ->cellVoltage_mV[stringNumber][m][ltc_state->ltcData.usedCellIndex[stringNumber]] = voltage;

                        /* Set the register relevant invalidCellVoltage to valid (false) */
                        for (uint8_t i = 0u; i < LTC_NUMBER_OF_CELL_VOLTAGES_PER_REGISTER; i++) {
                            ltc_state->ltcData.cellVoltage
                                ->invalidCellVoltage[stringNumber][(m / LTC_NUMBER_OF_LTC_PER_MODULE)][cellOffset + i] =
                                false;
                        }
                    } else {
                        /* PEC_valid == false: Invalidate only flags of this voltage register */
                        for (uint8_t i = 0u; i < LTC_NUMBER_OF_CELL_VOLTAGES_PER_REGISTER; i++) {
                            ltc_state->ltcData.cellVoltage
                                ->invalidCellVoltage[stringNumber][(m / LTC_NUMBER_OF_LTC_PER_MODULE)][cellOffset + i] =
                                true;
                        }
                    }

                    (ltc_state->ltcData.usedCellIndex[stringNumber])++;
                    incrementation++;

                    if ((ltc_state->ltcData.usedCellIndex[stringNumber]) > BS_NR_OF_CELL_BLOCKS_PER_MODULE) {
                        break;
                    }
                }
            }

            /* Restore start value for next module in the daisy-chain. Only
             * decrement used cell index if current module is not the last
             * module in the daisy-chain. */
            if ((m + 1u) < LTC_N_LTC) {
                (ltc_state->ltcData.usedCellIndex[stringNumber]) -= incrementation;
            }
        }
    }
}

/**
 * @brief   saves the GPIO voltage values read from the LTC daisy-chain.
 *
 * After a voltage measurement was initiated to measure the voltages on all GPIOs,
 * the result is read via SPI from the daisy-chain. In order to read the result of all GPIO measurements,
 * it is necessary to read auxiliary register A and B.
 * Only one register can be read at a time.
 * This function is called to store the result from the transmission in a buffer.
 *
 * @param   ltc_state      state of the ltc state machine
 * @param   pRxBuff        receive buffer
 * @param   registerSet    voltage register that was read (auxiliary register A, B, C or D)
 * @param  stringNumber    string addressed
 *
 */
static void LTC_SaveRxToGpioBuffer(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t registerSet,
    uint8_t stringNumber) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    uint8_t i_offset    = 0;
    uint32_t bitmask    = 0;
    uint16_t buffer_LSB = 0;
    uint16_t buffer_MSB = 0;

    if (registerSet == 0u) {
        /* RDAUXA command -> GPIO register group A */
        i_offset = 0;
        bitmask  = 0x07u << i_offset; /* 0x07: three temperatures in this register */
        /* Retrieve data without command and CRC*/
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            /* Check if PEC is valid */
            if (ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] == true) {
                bitmask = ~bitmask; /* negate bitmask to only validate flags of this voltage register */
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] &= bitmask;
                /* values received in 100uV -> divide by 10 to convert to mV */
                buffer_MSB = pRxBuff[4u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[4u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][0u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][0 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[4+i*8]))/10; */
                buffer_MSB = pRxBuff[6u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[6u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][1u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][1 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[6+i*8]))/10; */
                buffer_MSB = pRxBuff[8u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[8u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][2u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][2 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[8+i*8]))/10; */
            } else {
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] |= bitmask;
            }
        }
    } else if (registerSet == 1u) {
        /* RDAUXB command -> GPIO register group B */
        i_offset = 3;
        bitmask  = 0x03u << i_offset; /* 0x03: two temperatures in this register */
        /* Retrieve data without command and CRC*/
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            /* Check if PEC is valid */
            if (ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] == true) {
                bitmask = ~bitmask; /* negate bitmask to only validate flags of this voltage register */
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] &= bitmask;
                /* values received in 100uV -> divide by 10 to convert to mV */
                buffer_MSB = pRxBuff[4u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[4u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][0u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][0 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[4+i*8]))/10; */
                buffer_MSB = pRxBuff[6u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[6u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][1u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][1 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[6+i*8]))/10; */
            } else {
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] |= bitmask;
            }
        }
    } else if (registerSet == 2u) {
        /* RDAUXC command -> GPIO register group C, for 18 cell version */
        i_offset = 5;
        bitmask  = 0x07u << i_offset; /* 0x07: three temperatures in this register */
        /* Retrieve data without command and CRC*/
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            /* Check if PEC is valid */
            if (ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] == true) {
                bitmask = ~bitmask; /* negate bitmask to only validate flags of this voltage register */
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] &= bitmask;
                /* values received in 100uV -> divide by 10 to convert to mV */
                buffer_MSB = pRxBuff[4u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[4u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][0u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][0 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[4+i*8]))/10; */
                buffer_MSB = pRxBuff[6u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[6u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][1u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][1 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[6+i*8]))/10; */
                buffer_MSB = pRxBuff[8u + (i * 8u) + 1u];
                buffer_LSB = pRxBuff[8u + (i * 8u)];
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][2u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    ((buffer_LSB | (buffer_MSB << 8u))) / 10u;
                /* ltc_state->ltcData.allGpioVoltages->gpioVoltage[stringNumber][2 + i_offset +
                 * SLV_NR_OF_GPIOS_PER_MODULE*i]=
                 * *((uint16_t *)(&pRxBuff[8+i*8]))/10; */
            } else {
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] |= bitmask;
            }
        }
    } else if (registerSet == 3u) {
        /* RDAUXD command -> GPIO register group D, for 18 cell version */
        i_offset = 8;
        bitmask  = 0x01u << i_offset; /* 0x01: one temperature in this register */
        /* Retrieve data without command and CRC*/
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            /* Check if PEC is valid */
            if (ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] == true) {
                bitmask = ~bitmask; /* negate bitmask to only validate flags of this voltage register */
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] &= bitmask;
                /* values received in 100uV -> divide by 10 to convert to mV */
                ltc_state->ltcData.allGpioVoltages
                    ->gpioVoltages_mV[stringNumber][0u + i_offset + (SLV_NR_OF_GPIOS_PER_MODULE * i)] =
                    *((uint16_t *)(&pRxBuff[4u + (i * 8u)])) / 10u;
            } else {
                ltc_state->ltcData.allGpioVoltages->invalidGpioVoltages[stringNumber][i] |= bitmask;
            }
        }
    } else {
        ; /* Nothing to do */
    }
}

/**
 * @brief   checks if the multiplexers acknowledged transmission.
 *
 * The RDCOMM command can be used to read the answer of the multiplexers to a
 * I2C transmission.
 * This function determines if the communication with the multiplexers was
 * successful or not.
 * The array error table is updated to locate the multiplexers that did not
 * acknowledge transmission.
 *
 * @param   ltc_state      state of the ltc state machine
 * @param   pRxBuff        receive buffer
 * @param   mux            multiplexer to be addressed (multiplexer ID)
 * @param  stringNumber    string addressed
 *
 * @return  STD_OK if there was no error, STD_NOT_OK if there was errors
 */
static STD_RETURN_TYPE_e LTC_I2cCheckAcknowledge(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t mux,
    uint8_t stringNumber) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e muxError = STD_OK;

    for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
        if ((pRxBuff[4u + 1u + (LTC_NUMBER_OF_LTC_PER_MODULE * i * 8u)] & 0x0Fu) != 0x07u) { /* ACK = 0xX7 */
            if (LTC_DISCARD_MUX_CHECK == false) {
                if (mux == 0u) {
                    ltc_state->ltcData.errorTable->mux0[stringNumber][i] = 1;
                }
                if (mux == 1u) {
                    ltc_state->ltcData.errorTable->mux1[stringNumber][i] = 1;
                }
                if (mux == 2u) {
                    ltc_state->ltcData.errorTable->mux2[stringNumber][i] = 1;
                }
                if (mux == 3u) {
                    ltc_state->ltcData.errorTable->mux3[stringNumber][i] = 1;
                }
            }
            muxError = STD_NOT_OK;
        } else {
            if (mux == 0u) {
                ltc_state->ltcData.errorTable->mux0[stringNumber][i] = 0;
            }
            if (mux == 1u) {
                ltc_state->ltcData.errorTable->mux1[stringNumber][i] = 0;
            }
            if (mux == 2u) {
                ltc_state->ltcData.errorTable->mux2[stringNumber][i] = 0;
            }
            if (mux == 3u) {
                ltc_state->ltcData.errorTable->mux3[stringNumber][i] = 0;
            }
        }
    }

    if (LTC_DISCARD_MUX_CHECK == true) {
        muxError = STD_OK;
    }
    return muxError;
}

/**
 * @brief   initialize the daisy-chain.
 *
 * To initialize the LTC6804 daisy-chain, a dummy byte (0x00) is sent.
 *
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 *
 * @return  retVal  #STD_OK if dummy byte was sent correctly by SPI, #STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_Init(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    uint8_t PEC_Check[LTC_DATA_SIZE_IN_BYTES];
    uint16_t PEC_result = 0;

    /* now construct the message to be sent: it contains the wanted data, PLUS the needed PECs */
    pTxBuff[0] = ltc_cmdWRCFG[0];
    pTxBuff[1] = ltc_cmdWRCFG[1];
    pTxBuff[2] = ltc_cmdWRCFG[2];
    pTxBuff[3] = ltc_cmdWRCFG[3];

    /* set REFON bit to 1 */
    /* data for the configuration */
    for (uint16_t i = 0u; i < LTC_N_LTC; i++) {
        /* FC = disable all pull-downs, REFON = 1, DTEN = 0, ADCOPT = 0 */
        pTxBuff[4u + (i * 8u)] = 0xFC;
        pTxBuff[5u + (i * 8u)] = 0x00;
        pTxBuff[6u + (i * 8u)] = 0x00;
        pTxBuff[7u + (i * 8u)] = 0x00;
        pTxBuff[8u + (i * 8u)] = 0x00;
        pTxBuff[9u + (i * 8u)] = 0x00;

        PEC_Check[0] = pTxBuff[4u + (i * 8u)];
        PEC_Check[1] = pTxBuff[5u + (i * 8u)];
        PEC_Check[2] = pTxBuff[6u + (i * 8u)];
        PEC_Check[3] = pTxBuff[7u + (i * 8u)];
        PEC_Check[4] = pTxBuff[8u + (i * 8u)];
        PEC_Check[5] = pTxBuff[9u + (i * 8u)];

        PEC_result              = LTC_CalculatePec15(LTC_DATA_SIZE_IN_BYTES, PEC_Check);
        pTxBuff[10u + (i * 8u)] = (PEC_result >> 8u) & 0xFFu;
        pTxBuff[11u + (i * 8u)] = PEC_result & 0xFFu;
    } /* end for */

    retVal = LTC_TRANSMIT_RECEIVE_DATA(pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return retVal;
}

/**
 * @brief   sets the balancing according to the control values read in the database.
 *
 * To set balancing for the cells, the corresponding bits have to be written in the configuration register.
 * The LTC driver only executes the balancing orders written by the BMS in the database.
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 * @param   registerSet          register Set, 0: cells 1 to 12 (WRCFG), 1: cells 13 to 15/18 (WRCFG2)
 * @param   stringNumber         string addressed
 *
 * @return                 STD_OK if dummy byte was sent correctly by SPI, STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_BalanceControl(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t registerSet,
    uint8_t stringNumber) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e retVal = STD_OK;

    uint8_t PEC_Check[LTC_DATA_SIZE_IN_BYTES];
    uint16_t PEC_result = 0u;

    LTC_GetBalancingControlValues(ltc_state);

    if (registerSet == 0u) { /* cells 1 to 12, WRCFG */
        pTxBuff[0] = ltc_cmdWRCFG[0];
        pTxBuff[1] = ltc_cmdWRCFG[1];
        pTxBuff[2] = ltc_cmdWRCFG[2];
        pTxBuff[3] = ltc_cmdWRCFG[3];
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* The daisy-chain works like a shift register, so the order has to be reversed:
               when addressing e.g. the first module in the daisy-chain, the data will be sent last on the SPI bus and
               when addressing e.g. the last module in the daisy-chain, the data will be sent first on the SPI bus  */
            const uint16_t reverseModuleNumber = BS_NR_OF_MODULES_PER_STRING - m - 1u;

            /* FC = disable all pull-downs, REFON = 1 (reference always on), DTEN off, ADCOPT = 0 */
            pTxBuff[4u + (reverseModuleNumber * 8u)] = 0xFCu;
            pTxBuff[5u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[6u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[7u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[8u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[9u + (reverseModuleNumber * 8u)] = 0x00u;

            /* Iterate over all cell block to check if any of first 12 balancing inputs shall be activated */
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (ltc_state->ltcData.balancingControl->activateBalancing[stringNumber][m][cb] == true) {
                    /* Activate balancing for the cell block */
                    const uint8_t voltageInputIndex = LTC_GetVoltageInputIndexFromCellBlockIndex(cb);
                    /* Check if index is within the possible balancing inputs for WRCFG register */
                    if (voltageInputIndex <= LTC_UPPER_INDEX_FOR_BALANCING_OUTPUTS_IN_REGISTER_WRCFG) {
                        /* --------- Set respective balancing flags -----------
                         * Balancing outputs 0 - 7 are in byte 8
                         * Balancing outputs 8 - 11 are in bits 0-3 in byte 9 */
                        if (voltageInputIndex <= 7u) {
                            pTxBuff[8u + (reverseModuleNumber * 8u)] |= (0x01u << voltageInputIndex);
                        } else {
                            const uint8_t correctedIndexForByte9 = voltageInputIndex % 8u;
                            pTxBuff[9u + (reverseModuleNumber * 8u)] |= (0x01u << correctedIndexForByte9);
                        }
                    }
                }
            }

            /* Copy data for PEC calculation */
            PEC_Check[0] = pTxBuff[4u + (reverseModuleNumber * 8u)];
            PEC_Check[1] = pTxBuff[5u + (reverseModuleNumber * 8u)];
            PEC_Check[2] = pTxBuff[6u + (reverseModuleNumber * 8u)];
            PEC_Check[3] = pTxBuff[7u + (reverseModuleNumber * 8u)];
            PEC_Check[4] = pTxBuff[8u + (reverseModuleNumber * 8u)];
            PEC_Check[5] = pTxBuff[9u + (reverseModuleNumber * 8u)];

            PEC_result                                = LTC_CalculatePec15(LTC_DATA_SIZE_IN_BYTES, PEC_Check);
            pTxBuff[10u + (reverseModuleNumber * 8u)] = (PEC_result >> 8u) & 0xFFu;
            pTxBuff[11u + (reverseModuleNumber * 8u)] = PEC_result & 0xFFu;
        }
        retVal = LTC_TRANSMIT_RECEIVE_DATA(pSpiInterface, pTxBuff, pRxBuff, frameLength);
    } else if (registerSet == 1u) { /* cells 13 to 15/18 WRCFG2 */
        pTxBuff[0] = ltc_cmdWRCFG2[0];
        pTxBuff[1] = ltc_cmdWRCFG2[1];
        pTxBuff[2] = ltc_cmdWRCFG2[2];
        pTxBuff[3] = ltc_cmdWRCFG2[3];
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            /* The daisy-chain works like a shift register, so the order has to be reversed:
               when addressing e.g. the first module in the daisy-chain, the data will be sent last on the SPI bus and
               when addressing e.g. the last module in the daisy-chain, the data will be sent first on the SPI bus  */
            const uint16_t reverseModuleNumber = BS_NR_OF_MODULES_PER_STRING - m - 1u;

            /* 0x0F = disable pull-downs on GPIO6-9 */
            pTxBuff[4u + (reverseModuleNumber * 8u)] = 0x0Fu;
            pTxBuff[5u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[6u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[7u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[8u + (reverseModuleNumber * 8u)] = 0x00u;
            pTxBuff[9u + (reverseModuleNumber * 8u)] = 0x00u;

            /* Iterate over all cell block to check if any of first 12 balancing inputs shall be activated */
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                if (ltc_state->ltcData.balancingControl->activateBalancing[stringNumber][m][cb] == true) {
                    /* Activate balancing for the cell block */
                    const uint8_t voltageInputIndex = LTC_GetVoltageInputIndexFromCellBlockIndex(cb);
                    /* Check if index is within the possible balancing inputs for WRCFG register */
                    if ((voltageInputIndex >= LTC_LOWER_INDEX_FOR_BALANCING_OUTPUTS_IN_REGISTER_WRCFG2) &&
                        (voltageInputIndex <= LTC_UPPER_INDEX_FOR_BALANCING_OUTPUTS_IN_REGISTER_WRCFG2)) {
                        /* --------- Set respective balancing flags -----------
                         * Balancing outputs 12 - 15 are inbits 4-7 in byte 4
                         * Balancing outputs 16 - 17 are in bits 0-1 in byte 5 */
                        if (voltageInputIndex <= 15u) {
                            /* Indices 12 - 15 */
                            const uint8_t correctedIndexForByte4 = voltageInputIndex % 12u;
                            pTxBuff[4u + (reverseModuleNumber * 8u)] |= (0x10u << correctedIndexForByte4);
                        } else {
                            /* Indices 16 + 17 */
                            const uint8_t correctedIndexForByte5 = voltageInputIndex % 16u;
                            pTxBuff[5u + (reverseModuleNumber * 8u)] |= (0x01u << correctedIndexForByte5);
                        }
                    }
                }
            }

            PEC_Check[0] = pTxBuff[4u + (reverseModuleNumber * 8u)];
            PEC_Check[1] = pTxBuff[5u + (reverseModuleNumber * 8u)];
            PEC_Check[2] = pTxBuff[6u + (reverseModuleNumber * 8u)];
            PEC_Check[3] = pTxBuff[7u + (reverseModuleNumber * 8u)];
            PEC_Check[4] = pTxBuff[8u + (reverseModuleNumber * 8u)];
            PEC_Check[5] = pTxBuff[9u + (reverseModuleNumber * 8u)];

            PEC_result                                = LTC_CalculatePec15(LTC_DATA_SIZE_IN_BYTES, PEC_Check);
            pTxBuff[10u + (reverseModuleNumber * 8u)] = (PEC_result >> 8u) & 0xFFu;
            pTxBuff[11u + (reverseModuleNumber * 8u)] = PEC_result & 0xFFu;
        }
        retVal = LTC_TRANSMIT_RECEIVE_DATA(pSpiInterface, pTxBuff, pRxBuff, frameLength);
    } else {
        retVal = STD_NOT_OK;
    }
    return retVal;
}

/**
 * @brief   resets the error table.
 *
 * This function should be called during initialization or before starting a new measurement cycle
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 */
static void LTC_ResetErrorTable(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            ltc_state->ltcData.errorTable->PEC_valid[s][i] = false;
            ltc_state->ltcData.errorTable->mux0[s][i]      = 0;
            ltc_state->ltcData.errorTable->mux1[s][i]      = 0;
            ltc_state->ltcData.errorTable->mux2[s][i]      = 0;
            ltc_state->ltcData.errorTable->mux3[s][i]      = 0;
        }
    }
}

/**
 * @brief   brief missing
 * @details Gets the measurement time needed by the LTC analog front-end,
 *          depending on the measurement mode and the number of channels.
 *          For all cell voltages or all 5 GPIOS, the measurement time is the
 *          same.
 *          For 2 cell voltages or 1 GPIO, the measurement time is the same.
 *          As a consequence, this function is used for cell voltage and for
 *          GPIO measurement.
 * @param   adcMode     LTC ADCMeasurement mode (fast, normal or filtered)
 * @param   adcMeasCh   number of channels measured for GPIOS (one at a time
 *                      for multiplexers or all five GPIOs) or number of cell
 *                      voltage measured (2 cells or all cells)
 * @return  measurement time in ms
 */
static uint16_t LTC_GetMeasurementTimeCycle(LTC_ADCMODE_e adcMode, LTC_ADCMEAS_CHAN_e adcMeasCh) {
    uint16_t retVal = LTC_ADCMEAS_UNDEFINED; /* default */

    if (adcMeasCh == LTC_ADCMEAS_ALL_CHANNEL_CELLS) {
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_ALL_CELLS_FAST_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_NORMAL_DCP0) || (adcMode == LTC_ADCMODE_NORMAL_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_ALL_CELLS_NORMAL_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_ALL_CELLS_FILTERED_TCYCLE;
        }
    } else if (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_TWOCELLS) {
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_TWO_CELLS_FAST_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_NORMAL_DCP0) || (adcMode == LTC_ADCMODE_NORMAL_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_TWO_CELLS_NORMAL_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_TWO_CELLS_FILTERED_TCYCLE;
        }
    } else if (adcMeasCh == LTC_ADCMEAS_ALL_CHANNEL_GPIOS) {
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_ALL_GPIOS_FAST_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_NORMAL_DCP0) || (adcMode == LTC_ADCMODE_NORMAL_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_ALL_GPIOS_NORMAL_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_ALL_GPIOS_FILTERED_TCYCLE;
        }
    } else if (
        (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO1) || (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO2) ||
        (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO3) || (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO4) ||
        (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO5)) {
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_SINGLE_GPIO_FAST_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_NORMAL_DCP0) || (adcMode == LTC_ADCMODE_NORMAL_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_SINGLE_GPIO_NORMAL_TCYCLE;
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_STATEMACH_MEAS_SINGLE_GPIO_FILTERED_TCYCLE;
        }
    } else {
        retVal = LTC_ADCMEAS_UNDEFINED;
    }

    return retVal;
}

/**
 * @brief   tells the LTC daisy-chain to start measuring the voltage on all cells.
 *
 * This function sends an instruction to the daisy-chain via SPI, in order to start voltage measurement for all cells.
 *
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   adcMode              LTC ADCMeasurement mode (fast, normal or filtered)
 * @param   adcMeasCh            number of cell voltage measured (2 cells or all cells)
 *
 * @return  retVal      #STD_OK if dummy byte was sent correctly by SPI, #STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_StartVoltageMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    LTC_ADCMEAS_CHAN_e adcMeasCh) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    STD_RETURN_TYPE_e retVal = STD_OK;

    if (adcMeasCh == LTC_ADCMEAS_ALL_CHANNEL_CELLS) {
        if (adcMode == LTC_ADCMODE_FAST_DCP0) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADCV_fast_DCP0);
        } else if (adcMode == LTC_ADCMODE_NORMAL_DCP0) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADCV_normal_DCP0);
        } else if (adcMode == LTC_ADCMODE_FILTERED_DCP0) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADCV_filtered_DCP0);
        } else if (adcMode == LTC_ADCMODE_FAST_DCP1) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADCV_fast_DCP1);
        } else if (adcMode == LTC_ADCMODE_NORMAL_DCP1) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADCV_normal_DCP1);
        } else if (adcMode == LTC_ADCMODE_FILTERED_DCP1) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADCV_filtered_DCP1);
        } else {
            retVal = STD_NOT_OK;
        }
    } else if (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_TWOCELLS) {
        if (adcMode == LTC_ADCMODE_FAST_DCP0) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADCV_fast_DCP0_twocells);
        } else {
            retVal = STD_NOT_OK;
        }
    } else {
        retVal = STD_NOT_OK;
    }
    return retVal;
}

/**
 * @brief   tells LTC daisy-chain to start measuring the voltage on GPIOS.
 * @details This function sends an instruction to the daisy-chain via SPI to
 *          start the measurement.
 *
 * @param   pSpiInterface   pointer to SPI configuration
 * @param   adcMode         LTC ADCMeasurement mode (fast, normal or filtered)
 * @param   adcMeasCh       number of channels measured for GPIOS (one at a
 *                          time, typically when multiplexers are used, or all
 *                          five GPIOs)
 *
 * @return  #STD_OK if dummy byte was sent correctly by SPI, #STD_NOT_OK
 *          otherwise
 */
static STD_RETURN_TYPE_e LTC_StartGpioMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    LTC_ADCMEAS_CHAN_e adcMeasCh) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    STD_RETURN_TYPE_e retVal;

    if (adcMeasCh == LTC_ADCMEAS_ALL_CHANNEL_GPIOS) {
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_fast_ALL_GPIOS);
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_filtered_ALL_GPIOS);
        } else {
            /*if (adcMode == LTC_ADCMODE_NORMAL_DCP0 || adcMode == LTC_ADCMODE_NORMAL_DCP1)*/
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_normal_ALL_GPIOS);
        }
    } else if (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO1) {
        /* Single Channel */
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_fast_GPIO1);
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_filtered_GPIO1);
        } else {
            /*if (adcMode == LTC_ADCMODE_NORMAL_DCP0 || adcMode == LTC_ADCMODE_NORMAL_DCP1)*/

            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_normal_GPIO1);
        }
    } else if (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO2) {
        /* Single Channel */
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_fast_GPIO2);
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_filtered_GPIO2);
        } else {
            /*if (adcMode == LTC_ADCMODE_NORMAL_DCP0 || adcMode == LTC_ADCMODE_NORMAL_DCP1)*/

            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_normal_GPIO2);
        }
    } else if (adcMeasCh == LTC_ADCMEAS_SINGLECHANNEL_GPIO3) {
        /* Single Channel */
        if ((adcMode == LTC_ADCMODE_FAST_DCP0) || (adcMode == LTC_ADCMODE_FAST_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_fast_GPIO3);
        } else if ((adcMode == LTC_ADCMODE_FILTERED_DCP0) || (adcMode == LTC_ADCMODE_FILTERED_DCP1)) {
            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_filtered_GPIO3);
        } else {
            /*if (adcMode == LTC_ADCMODE_NORMAL_DCP0 || adcMode == LTC_ADCMODE_NORMAL_DCP1)*/

            retVal = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_cmdADAX_normal_GPIO3);
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

/**
 * @brief   tells LTC daisy-chain to start measuring the voltage on GPIOS.
 *
 * This function sends an instruction to the daisy-chain via SPI to start the measurement.
 *
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   adcMode     LTC ADCMeasurement mode (fast, normal or filtered)
 * @param   PUP         pull-up bit for pull-up or pull-down current (0: pull-down, 1: pull-up)
 *
 * @return  retVal      #STD_OK if command was sent correctly by SPI, #STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_StartOpenWireMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    uint8_t PUP) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    if (PUP == 0u) {
        /* pull-down current */
        if (adcMode == LTC_ADCMODE_NORMAL_DCP0) {
            retval = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_BC_cmdADOW_PDOWN_normal_DCP0);
        } else if (adcMode == LTC_ADCMODE_FILTERED_DCP0) {
            retval = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_BC_cmdADOW_PDOWN_filtered_DCP0);
        } else {
            retval = STD_NOT_OK;
        }
    } else if (PUP == 1u) {
        /* pull-up current */
        if (adcMode == LTC_ADCMODE_NORMAL_DCP0) {
            retval = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_BC_cmdADOW_PUP_normal_DCP0);
        } else if (adcMode == LTC_ADCMODE_FILTERED_DCP0) {
            retval = LTC_TRANSMIT_COMMAND(pSpiInterface, ltc_BC_cmdADOW_PUP_filtered_DCP0);
        } else {
            retval = STD_NOT_OK;
        }
    }
    return retval;
}

/**
 * @brief   checks if the data received from the daisy-chain is not corrupt.
 *
 * This function computes the PEC (CRC) from the data received by the daisy-chain.
 * It compares it with the PEC sent by the LTCs.
 * If there are errors, the array LTC_ErrorTable is updated to locate the LTCs in daisy-chain
 * that transmitted corrupt data.
 *
 * @param   ltc_state                    state of the ltc state machine
 * @param   DataBufferSPI_RX_with_PEC    data obtained from the SPI transmission
 * @param  stringNumber                  string addressed
 *
 * @return  retVal                       STD_OK if PEC check is OK, STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_CheckPec(
    LTC_STATE_s *ltc_state,
    uint16_t *DataBufferSPI_RX_with_PEC,
    uint8_t stringNumber) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(DataBufferSPI_RX_with_PEC != NULL_PTR);
    STD_RETURN_TYPE_e retVal = STD_OK;
    uint8_t PEC_TX[2];
    uint16_t PEC_result                       = 0;
    uint8_t PEC_Check[LTC_DATA_SIZE_IN_BYTES] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* check all PECs and put data without command and PEC in DataBufferSPI_RX (easier to use) */
    for (uint16_t i = 0; i < LTC_N_LTC; i++) {
        PEC_Check[0] = DataBufferSPI_RX_with_PEC[4u + (i * 8u)];
        PEC_Check[1] = DataBufferSPI_RX_with_PEC[5u + (i * 8u)];
        PEC_Check[2] = DataBufferSPI_RX_with_PEC[6u + (i * 8u)];
        PEC_Check[3] = DataBufferSPI_RX_with_PEC[7u + (i * 8u)];
        PEC_Check[4] = DataBufferSPI_RX_with_PEC[8u + (i * 8u)];
        PEC_Check[5] = DataBufferSPI_RX_with_PEC[9u + (i * 8u)];

        PEC_result = LTC_CalculatePec15(LTC_DATA_SIZE_IN_BYTES, PEC_Check);
        PEC_TX[0]  = (uint8_t)((PEC_result >> 8u) & 0xFFu);
        PEC_TX[1]  = (uint8_t)(PEC_result & 0xFFu);

        /* if calculated PEC not equal to received PEC */
        if ((PEC_TX[0] != DataBufferSPI_RX_with_PEC[10u + (i * 8u)]) ||
            (PEC_TX[1] != DataBufferSPI_RX_with_PEC[11u + (i * 8u)])) {
            /* update error table of the corresponding LTC only if PEC check is activated */
            if (LTC_DISCARD_PEC == false) {
                ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] = false;
                retVal                                                    = STD_NOT_OK;
            } else {
                ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] = true;
            }
        } else {
            /* update error table of the corresponding LTC */
            ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] = true;
        }
    }
    return retVal;
}

/**
 * @brief   send command to the LTC daisy-chain and receives data from the LTC
 *          daisy-chain.
 * @details This is the core function to receive data from the LTC6813-1
 *          daisy-chain.
 *          A 2 byte command is sent with the corresponding PEC.
 *          *Example*: read configuration register (RDCFG).
 *          Only command has to be set, the function calculates the PEC
 *          automatically.
 *          - The data sent is:
 *            - 2 bytes (COMMAND) 2 bytes (PEC)
 *          - The data received is:
 *            - 6 bytes (LTC1) 2 bytes (PEC) +
 *            - 6 bytes (LTC2) 2 bytes (PEC) +
 *            - 6 bytes (LTC3) 2 bytes (PEC) +
 *            - ... +
 *            - 6 bytes (LTC{LTC_N_LTC}) 2 bytes (PEC)
 *
 *          The function does not check the PECs. This has to be done
 *          elsewhere.
 *
 * @param   Command         command sent to the daisy-chain
 * @param   pSpiInterface   pointer to SPI configuration
 * @param   pTxBuff         transmit buffer
 * @param   pRxBuff         receive buffer
 * @param   frameLength     number of words to transmit
 *
 * @return  #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_ReadRegister(
    uint16_t *Command,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(Command != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    /* DataBufferSPI_RX_with_PEC contains the data to receive.
       The transmission function checks the PECs.
       It constructs DataBufferSPI_RX, which contains the received data without PEC (easier to use). */

    for (uint16_t i = 0; i < LTC_N_BYTES_FOR_DATA_TRANSMISSION; i++) {
        pTxBuff[i] = 0x00;
    }

    pTxBuff[0] = Command[0];
    pTxBuff[1] = Command[1];
    pTxBuff[2] = Command[2];
    pTxBuff[3] = Command[3];

    retVal = LTC_TRANSMIT_RECEIVE_DATA(pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return retVal;
}

/**
 * @brief   sends command and data to the LTC daisy-chain.
 * @details This is the core function to transmit data to the LTC6813-1
 *          daisy-chain.
 *          The data sent is:
 *          - COMMAND +
 *          - 6 bytes (LTC1) +
 *          - 6 bytes (LTC2) +
 *          - 6 bytes (LTC3) +
 *          - ... +
 *          - 6 bytes (LTC{LTC_N_LTC})
 *
 *          A 2 byte command is sent with the corresponding PEC.
 *          *Example*: write configuration register (WRCFG).
 *          The command has to be set and then the function calculates the PEC
 *          automatically.
 *          The function calculates the needed PEC to send the data to the
 *          daisy-chain.
 *          The sent data has the format:
 *          - 2 byte-COMMAND (2 bytes PEC) +
 *          - 6 bytes (LTC1) (2 bytes PEC) +
 *          - 6 bytes (LTC2) (2 bytes PEC) +
 *          - 6 bytes (LTC3) (2 bytes PEC) +
 *          - ... +
 *          - 6 bytes (LTC{LTC_N_LTC}) (2 bytes PEC)
 *
 *          The function returns 0. The only way to check if the transmission
 *          was successful is to read the results of the write operation.
 *          (example: read configuration register after writing to it)
 *
 * @param   Command         command sent to the daisy-chain
 * @param   pSpiInterface   pointer to SPI configuration
 * @param   pTxBuff         transmit buffer
 * @param   pRxBuff         receive buffer
 * @param   frameLength     number of words to transmit
 *
 * @return  STD_OK if SPI transmission is OK, STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_WriteRegister(
    uint16_t *Command,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(Command != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    uint16_t PEC_result                       = 0;
    uint8_t PEC_Check[LTC_DATA_SIZE_IN_BYTES] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    pTxBuff[0] = Command[0];
    pTxBuff[1] = Command[1];
    pTxBuff[2] = Command[2];
    pTxBuff[3] = Command[3];

    /* Calculate PEC of all data (1 PEC value for 6 bytes) */
    for (uint16_t i = 0u; i < LTC_N_LTC; i++) {
        PEC_Check[0] = pTxBuff[4u + (i * 8u)];
        PEC_Check[1] = pTxBuff[5u + (i * 8u)];
        PEC_Check[2] = pTxBuff[6u + (i * 8u)];
        PEC_Check[3] = pTxBuff[7u + (i * 8u)];
        PEC_Check[4] = pTxBuff[8u + (i * 8u)];
        PEC_Check[5] = pTxBuff[9u + (i * 8u)];

        PEC_result              = LTC_CalculatePec15(LTC_DATA_SIZE_IN_BYTES, PEC_Check);
        pTxBuff[10u + (i * 8u)] = (PEC_result >> 8u) & 0xFFu;
        pTxBuff[11u + (i * 8u)] = PEC_result & 0xFFu;
    }

    retVal = LTC_TRANSMIT_RECEIVE_DATA(pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return retVal;
}

/**
 * @brief   configures the data that will be sent to the LTC daisy-chain to configure multiplexer channels.
 *
 * This function does not sent the data to the multiplexer daisy-chain. This is done
 * by the function LTC_SetMuxChannel(), which calls LTC_SetMuxChCommand()..
 *
 * @param   pTxBuff              transmit buffer
 * @param   mux                  multiplexer ID to be configured (0,1,2 or 3)
 * @param   channel              multiplexer channel to be configured (0 to 7)
 *
 */
static void LTC_SetMuxChCommand(uint16_t *pTxBuff, uint8_t mux, uint8_t channel) {
    FAS_ASSERT(pTxBuff != NULL_PTR);
    for (uint16_t i = 0; i < LTC_N_LTC; i++) {
#if SLAVE_BOARD_VERSION == 2u

        /* using ADG728 */
        uint8_t address = 0x98u | ((mux % 4u) << 1u);
        uint8_t data    = 1u << (channel % 8u);
        if (channel == 0xFFu) { /* no channel selected, output of multiplexer is high impedance */
            data = 0x00;
        }

#else

        /* using LTC1380 */
        uint8_t address = 0x90u | ((mux % 4u) << 1u);
        uint8_t data    = 0x08u | (channel % 8u);
        if (channel == 0xFFu) { /* no channel selected, output of multiplexer is high impedance */
            data = 0x00;
        }

#endif

        pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | ((address >> 4u) & 0x0Fu); /* 0x6 : LTC6804: ICOM START from Master */
        pTxBuff[5u + (i * 8u)] = LTC_FCOM_MASTER_NACK | ((address << 4u) & 0xF0u);
        pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | ((data >> 4u) & 0x0Fu);
        pTxBuff[7u + (i * 8u)] = LTC_FCOM_MASTER_NACK_STOP | ((data << 4u) & 0xF0u);
        pTxBuff[8u + (i * 8u)] = LTC_ICOM_NO_TRANSMIT; /* 0x1 : ICOM-STOP */
        pTxBuff[9u + (i * 8u)] = 0x00;                 /* 0x0 : dummy (Dn) */
                                                       /* 9: MASTER NACK + STOP (FCOM) */
    }
}

/**
 * @brief   sends data to the LTC daisy-chain to read EEPROM on slaves.
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 * @param   step                 first or second stage of read process (0 or 1)
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_SendEepromReadCommand(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t step) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    /* send WRCOMM to send I2C message to choose channel */
    LTC_SetEepromReadCommand(ltc_state, pTxBuff, step);
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   configures the data that will be sent to the LTC daisy-chain to read EEPROM on slaves.
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pTxBuff              transmit buffer
 * @param   step                 first or second stage of read process (0 or 1)
 *
 */
static void LTC_SetEepromReadCommand(LTC_STATE_s *ltc_state, uint16_t *pTxBuff, uint8_t step) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    uint32_t address = ltc_state->ltcData.slaveControl->eepromReadAddressToUse;

    address &= 0x3FFFFu;
    const uint8_t address0 = address >> 16u;
    const uint8_t address1 = (address & 0xFFFFu) >> 8u;
    const uint8_t address2 = address & 0xFFu;

    if (step == 0u) {
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | (0x0Au); /* 0x6 : LTC6804: ICOM START from Master */
            pTxBuff[5u + (i * 8u)] = LTC_FCOM_MASTER_NACK | (((address0 & 0x03u) << 5u) | 0x00u);
            pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | (address1 >> 4u);
            pTxBuff[7u + (i * 8u)] = LTC_FCOM_MASTER_NACK | (address1 << 4u);
            pTxBuff[8u + (i * 8u)] = LTC_ICOM_BLANK | (address2 >> 4u);
            pTxBuff[9u + (i * 8u)] = LTC_FCOM_MASTER_NACK | (address2 << 4u);
        }
    } else { /* step == 1 */
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | (0x0Au); /* 0x6 : LTC6804: ICOM START from Master */
            pTxBuff[5u + (i * 8u)] = LTC_FCOM_MASTER_NACK | (((address0 & 0x03u) << 5u) | 0x10u);
            pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | 0x0Fu;
            pTxBuff[7u + (i * 8u)] = LTC_FCOM_MASTER_NACK_STOP | 0xF0u;
            pTxBuff[8u + (i * 8u)] = LTC_ICOM_NO_TRANSMIT | 0x00u;
            pTxBuff[9u + (i * 8u)] = LTC_FCOM_MASTER_NACK_STOP | 0x00u;
        }
    }
}

/**
 * @brief   saves the read values of the external EEPROMs read from the LTC daisy-chain.
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pRxBuff              receive buffer
 *
 */
static void LTC_EepromSaveReadValue(LTC_STATE_s *ltc_state, uint16_t *pRxBuff) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    for (uint16_t i = 0; i < LTC_N_LTC; i++) {
        ltc_state->ltcData.slaveControl->eepromValueRead[i] = (pRxBuff[6u + (i * 8u)] << 4u) |
                                                              ((pRxBuff[7u + (i * 8u)] >> 4u));
    }

    ltc_state->ltcData.slaveControl->eepromReadAddressLastUsed =
        ltc_state->ltcData.slaveControl->eepromReadAddressToUse;
    ltc_state->ltcData.slaveControl->eepromReadAddressToUse = 0xFFFFFFFF;

    DATA_WRITE_DATA(ltc_state->ltcData.slaveControl);
}

/**
 * @brief   sends data to the LTC daisy-chain to write EEPROM on slaves.
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 * @param   step                 first or second stage of read process (0 or 1)
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_SendEepromWriteCommand(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t step) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    /* send WRCOMM to send I2C message to write EEPROM */
    LTC_SetEepromWriteCommand(ltc_state, pTxBuff, step);
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   configures the data that will be sent to the LTC daisy-chain to write EEPROM on slaves.
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pTxBuff              transmit buffer
 * @param   step                 first or second stage of read process (0 or 1)
 *
 */
static void LTC_SetEepromWriteCommand(LTC_STATE_s *ltc_state, uint16_t *pTxBuff, uint8_t step) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    uint32_t address = ltc_state->ltcData.slaveControl->eepromWriteAddressToUse;

    address &= 0x3FFFFu;
    const uint8_t address0 = address >> 16u;
    const uint8_t address1 = (address & 0xFFFFu) >> 8u;
    const uint8_t address2 = address & 0xFFu;

    if (step == 0u) {
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | (0x0Au); /* 0x6 : LTC6804: ICOM START from Master */
            pTxBuff[5u + (i * 8u)] = LTC_FCOM_MASTER_NACK | (((address0 & 0x03u) << 5u) | 0x00u);
            pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | (address1 >> 4u);
            pTxBuff[7u + (i * 8u)] = LTC_FCOM_MASTER_NACK | (address1 << 4u);
            pTxBuff[8u + (i * 8u)] = LTC_ICOM_BLANK | (address2 >> 4u);
            pTxBuff[9u + (i * 8u)] = LTC_FCOM_MASTER_NACK | (address2 << 4u);
        }
    } else { /* step == 1 */
        for (uint16_t i = 0; i < LTC_N_LTC; i++) {
            const uint8_t data = ltc_state->ltcData.slaveControl->eepromValueWrite[i];

            pTxBuff[4u + (i * 8u)] = LTC_ICOM_BLANK | (data >> 4u); /* 0x6 : LTC6804: ICOM START from Master */
            pTxBuff[5u + (i * 8u)] = LTC_FCOM_MASTER_NACK_STOP | (data << 4u);
            pTxBuff[6u + (i * 8u)] = LTC_ICOM_NO_TRANSMIT | 0x00u;
            pTxBuff[7u + (i * 8u)] = LTC_FCOM_MASTER_NACK_STOP | 0x00u;
            pTxBuff[8u + (i * 8u)] = LTC_ICOM_NO_TRANSMIT | 0x00u;
            pTxBuff[9u + (i * 8u)] = LTC_FCOM_MASTER_NACK_STOP | 0x00u;
        }

        ltc_state->ltcData.slaveControl->eepromWriteAddressLastUsed =
            ltc_state->ltcData.slaveControl->eepromWriteAddressToUse;
        ltc_state->ltcData.slaveControl->eepromWriteAddressToUse = 0xFFFFFFFF;

        DATA_WRITE_DATA(ltc_state->ltcData.slaveControl);
    }
}

/**
 * @brief   sends data to the LTC daisy-chain to configure multiplexer channels.
 *
 * This function calls the function LTC_SetMuxChCommand() to set the data.
 *
 * @param   pSpiInterface   pointer to SPI configuration
 * @param   pTxBuff         transmit buffer
 * @param   pRxBuff         receive buffer
 * @param   frameLength     number of words to transmit
 * @param   mux             multiplexer ID to be configured (0,1,2 or 3)
 * @param   channel         multiplexer channel to be configured (0 to 7)
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_SetMuxChannel(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t mux,
    uint8_t channel) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);

    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    /* send WRCOMM to send I2C message to choose channel */
    LTC_SetMuxChCommand(pTxBuff, mux, channel);
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   sends data to the LTC daisy-chain to communicate via I2C
 *
 * This function initiates an I2C signal sent by the LTC6804 on the slave boards
 *
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 * @param        cmd_data       command data to be sent
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_SendI2cCommand(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint16_t *cmd_data) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    FAS_ASSERT(cmd_data != NULL_PTR);
    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
        pTxBuff[4u + (i * 6u)] = cmd_data[0];
        pTxBuff[5u + (i * 6u)] = cmd_data[1];

        pTxBuff[6u + (i * 6u)] = cmd_data[2];
        pTxBuff[7u + (i * 6u)] = cmd_data[3];

        pTxBuff[8u + (i * 6u)] = cmd_data[4];
        pTxBuff[9u + (i * 6u)] = cmd_data[5];
    }

    /* send WRCOMM to send I2C message */
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   saves the temperature value of the external temperature sensors read from the LTC daisy-chain.
 *
 * This function saves the temperature value received from the external temperature sensors
 *
 * @param   ltc_state      state of the ltc state machine
 * @param   pRxBuff        receive buffer
 *
 */
static void LTC_TempSensSaveTemp(LTC_STATE_s *ltc_state, uint16_t *pRxBuff) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    for (uint16_t i = 0; i < LTC_N_LTC; i++) {
        uint8_t temp_tmp[2];
        temp_tmp[0]    = (pRxBuff[6u + (i * 8u)] << 4u) | ((pRxBuff[7u + (i * 8u)] >> 4u));
        temp_tmp[1]    = (pRxBuff[8u + (i * 8u)] << 4u) | ((pRxBuff[9u + (i * 8u)] >> 4u));
        uint16_t val_i = (temp_tmp[0] << 8u) | (temp_tmp[1]);
        val_i          = val_i >> 8u;
        ltc_state->ltcData.slaveControl->externalTemperatureSensor[i] = val_i;
    }

    DATA_WRITE_DATA(ltc_state->ltcData.slaveControl);
}

/**
 * @brief   sends data to the LTC daisy-chain to control the user port expander
 *
 * This function sends a control byte to the register of the user port expander
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_SetPortExpander(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);

    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
        const uint8_t output_data = ltc_state->ltcData.slaveControl->ioValueOut[BS_NR_OF_MODULES_PER_STRING - 1 - i];

        pTxBuff[4u + (i * 8u)] = LTC_ICOM_START |
                                 0x04u; /* 6: ICOM0 start condition, 4: upper nibble of PCA8574 address */
        pTxBuff[5u + (i * 8u)] =
            0u | LTC_FCOM_MASTER_NACK; /* 0: lower nibble of PCA8574 address + R/W bit, 8: FCOM0 master NACK */

        pTxBuff[6u + (i * 8u)] =
            LTC_ICOM_BLANK |
            (output_data >> 4u); /* 0: ICOM1 blank, x: upper nibble of PCA8574 data register (0 == pin low) */
        pTxBuff[7u + (i * 8u)] =
            (uint8_t)(output_data << 4u) |
            LTC_FCOM_MASTER_NACK_STOP; /* x: lower nibble of PCA8574 data register, 9: FCOM1 master NACK + STOP */

        pTxBuff[8u + (i * 8u)] = LTC_ICOM_NO_TRANSMIT; /* 7: no transmission, F: dummy data */
        pTxBuff[9u + (i * 8u)] = 0;                    /* F: dummy data, 9: FCOM2 master NACK + STOP */
    }

    /* send WRCOMM to send I2C message */
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   saves the received values of the external port expander read from the LTC daisy-chain.
 *
 * This function saves the received data byte from the external port expander
 *
 * @param   ltc_state      state of the ltc state machine
 * @param   pRxBuff        receive buffer
 *
 */
static void LTC_PortExpanderSaveValues(LTC_STATE_s *ltc_state, uint16_t *pRxBuff) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    /* extract data */
    for (uint16_t i = 0; i < LTC_N_LTC; i++) {
        const uint8_t val_i = (pRxBuff[6u + (i * 8u)] << 4u) | ((pRxBuff[7u + (i * 8u)] >> 4u));
        ltc_state->ltcData.slaveControl->ioValueIn[i] = val_i;
    }

    DATA_WRITE_DATA(ltc_state->ltcData.slaveControl);
}

/**
 * @brief   sends data to the LTC daisy-chain to control the user port expander from TI
 *
 * This function sends a control byte to the register of the user port expander from TI
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 * @param   direction            use port expander pin as input or output
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_SetPortExpanderDirectionTi(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    LTC_PORT_EXPANDER_TI_DIRECTION_e direction) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
        pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | 0x4u; /*upper nibble of TCA6408A address */
        pTxBuff[5u + (i * 8u)] = (uint8_t)((LTC_PORTEXPANDER_ADR_TI << 1u) << 4u) |
                                 LTC_FCOM_MASTER_NACK; /* 0: lower nibble of TCA6408A address + R/W bit */

        pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | (LTC_PORT_EXPANDER_TI_CONFIG_REG_ADR >>
                                                   4u); /* upper nibble of TCA6408A configuration register address */
        pTxBuff[7u + (i * 8u)] = (uint8_t)(LTC_PORT_EXPANDER_TI_CONFIG_REG_ADR << 4u) |
                                 LTC_FCOM_MASTER_NACK; /* lower nibble of TCA6408A configuration register address */

        pTxBuff[8u + (i * 8u)] = LTC_ICOM_BLANK |
                                 (direction >> 4u); /* upper nibble of TCA6408A configuration register data */
        pTxBuff[9u + (i * 8u)] = (uint8_t)(direction << 4u) |
                                 LTC_FCOM_MASTER_NACK_STOP; /* lower nibble of TCA6408A configuration register data */
    }

    /* send WRCOMM to send I2C message */
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   sends data to the LTC daisy-chain to control the user port expander from TI
 *
 * This function sends a control byte to the register of the user port expander from TI
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_SetPortExpanderOutputTi(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
        const uint8_t output_data = ltc_state->ltcData.slaveControl->ioValueOut[BS_NR_OF_MODULES_PER_STRING - 1 - i];

        pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | 0x4u; /* upper nibble of TCA6408A address */
        pTxBuff[5u + (i * 8u)] = (uint8_t)((LTC_PORTEXPANDER_ADR_TI << 1u) << 4u) |
                                 LTC_FCOM_MASTER_NACK; /* 0: lower nibble of TCA6408A address + R/W bit */

        pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | (LTC_PORT_EXPANDER_TI_OUTPUT_REG_ADR >>
                                                   4u); /* upper nibble of TCA6408A output register address */
        pTxBuff[7u + (i * 8u)] = (uint8_t)(LTC_PORT_EXPANDER_TI_OUTPUT_REG_ADR << 4u) |
                                 LTC_FCOM_MASTER_NACK; /* lower nibble of TCA6408A output register address */

        pTxBuff[8u + (i * 8u)] = LTC_ICOM_BLANK | (output_data >> 4u); /* upper nibble of TCA6408A output register */
        pTxBuff[9u + (i * 8u)] = (uint8_t)(output_data << 4u) |
                                 LTC_FCOM_MASTER_NACK_STOP; /* lower nibble of TCA6408A output register */
    }

    /* send WRCOMM to send I2C message */
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   sends data to the LTC daisy-chain to control the user port expander from TI
 *
 * @details This function sends a control byte to the register of the user port expander from TI
 *
 * @param   ltc_state            state of the ltc state machine
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 * @param   step        first or second stage of read process (0 or 1)
 *
 * @return       #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e LTC_GetPortExpanderInputTi(
    LTC_STATE_s *ltc_state,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength,
    uint8_t step) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    FAS_ASSERT(pRxBuff != NULL_PTR);
    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;

    if (step == 0u) {
        for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
            pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | 0x4u; /* upper nibble of TCA6408A address */
            pTxBuff[5u + (i * 8u)] = (uint8_t)((LTC_PORTEXPANDER_ADR_TI << 1u) << 4u) |
                                     LTC_FCOM_MASTER_NACK; /* lower nibble of TCA6408A address + R/W bit */

            pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | (LTC_PORT_EXPANDER_TI_INPUT_REG_ADR >>
                                                       4u); /* upper nibble of TCA6408A input register address */
            pTxBuff[7u + (i * 8u)] = (uint8_t)(LTC_PORT_EXPANDER_TI_INPUT_REG_ADR << 4u) |
                                     LTC_FCOM_MASTER_NACK; /* x: lower nibble of TCA6408A input register address */

            pTxBuff[8u + (i * 8u)] = LTC_ICOM_NO_TRANSMIT; /* no transmission */
            pTxBuff[9u + (i * 8u)] = 0;                    /* dummy data */
        }
    } else {
        DATA_READ_DATA(ltc_state->ltcData.slaveControl);

        for (uint16_t i = 0; i < BS_NR_OF_MODULES_PER_STRING; i++) {
            pTxBuff[4u + (i * 8u)] = LTC_ICOM_START | 0x4u; /* upper nibble of TCA6408A address */
            pTxBuff[5u + (i * 8u)] = (uint8_t)(((LTC_PORTEXPANDER_ADR_TI << 1u) | 1u) << 4u) |
                                     LTC_FCOM_MASTER_NACK; /* lower nibble of TCA6408A address + R/W bit */

            pTxBuff[6u + (i * 8u)] = LTC_ICOM_BLANK | 0x0Fu;       /* upper nibble slave data, master pulls bus high */
            pTxBuff[7u + (i * 8u)] = LTC_FCOM_MASTER_NACK | 0xF0u; /* lower nibble slave data, master pulls bus high */

            pTxBuff[8u + (i * 8u)] = LTC_ICOM_NO_TRANSMIT; /* no transmission */
            pTxBuff[9u + (i * 8u)] = 0;                    /* dummy data */
        }
    }

    /* send WRCOMM to send I2C message */
    statusSPI = LTC_WriteRegister(ltc_cmdWRCOMM, pSpiInterface, pTxBuff, pRxBuff, frameLength);

    return statusSPI;
}

/**
 * @brief   saves the received values of the external port expander from TI read from the LTC daisy-chain.
 * @details This function saves the received data byte from the external port expander from TI
 * @param   ltc_state            state of the ltc state machine
 * @param   pTxBuff              transmit buffer
 */
static void LTC_PortExpanderSaveValuesTi(LTC_STATE_s *ltc_state, uint16_t *pTxBuff) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    FAS_ASSERT(pTxBuff != NULL_PTR);
    DATA_READ_DATA(ltc_state->ltcData.slaveControl);

    /* extract data */
    for (uint16_t i = 0; i < LTC_N_LTC; i++) {
        const uint8_t val_i = (pTxBuff[6u + (i * 8u)] << 4u) | ((pTxBuff[7u + (i * 8u)] >> 4u));
        ltc_state->ltcData.slaveControl->ioValueIn[i] = val_i;
    }

    DATA_WRITE_DATA(ltc_state->ltcData.slaveControl);
}

/**
 * @brief   sends 72 clock pulses to the LTC daisy-chain.
 *
 * This function is used for the communication with the multiplexers via I2C on the GPIOs.
 * It send the command STCOMM to the LTC daisy-chain.
 *
 * @param   pSpiInterface        pointer to SPI configuration
 *
 * @return  statusSPI            #STD_OK if clock pulses were sent correctly by SPI, #STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_I2cClock(SPI_INTERFACE_CONFIG_s *pSpiInterface) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    STD_RETURN_TYPE_e statusSPI = STD_NOT_OK;
    uint16_t ltc_TXBufferClock[4u + 9u];

    for (uint16_t i = 0; i < (4u + 9u); i++) {
        ltc_TXBufferClock[i] = 0xFF;
    }

    ltc_TXBufferClock[0] = ltc_cmdSTCOMM[0];
    ltc_TXBufferClock[1] = ltc_cmdSTCOMM[1];
    ltc_TXBufferClock[2] = ltc_cmdSTCOMM[2];
    ltc_TXBufferClock[3] = ltc_cmdSTCOMM[3];

    statusSPI = LTC_TRANSMIT_I2C_COMMAND(pSpiInterface, ltc_TXBufferClock);

    return statusSPI;
}

/**
 * @brief   gets the frequency of the SPI clock.
 *
 * This function reads the configuration from the SPI handle directly.
 *
 * @param   pSpiInterface        pointer to SPI configuration
 *
 * @return    frequency of the SPI clock
 */
static uint32_t LTC_GetSpiClock(SPI_INTERFACE_CONFIG_s *pSpiInterface) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    uint32_t SPI_Clock = 0;
    uint32_t prescaler = 0;

    /* if (LTC_SPI_INSTANCE == SPI2 || LTC_SPI_INSTANCE == SPI3) { */
    /* SPI2 and SPI3 are connected to APB1 (PCLK1) */
    /* The prescaler setup bits LTC_SPI_PRESCALER corresponds to the bits 5:3 in the SPI_CR1 register */
    /* Reference manual p.909 */
    /* The shift by 3 puts the bits 5:3 to the first position */
    /* Division are made by powers of 2 which corresponds to shifting to the right */
    /* Then 0 corresponds to divide by 2, 1 corresponds to divide by 4... so 1 has to be added to the value of the
     * configuration bits */

    /* SPI_Clock = HAL_RCC_GetPCLK1Freq()>>((LTC_SPI_PRESCALER>>3)+1);
    } */

    /* if (LTC_SPI_INSTANCE == SPI1 || LTC_SPI_INSTANCE == SPI4 || LTC_SPI_INSTANCE == SPI5 || LTC_SPI_INSTANCE == SPI6)
     * {
     */
    /* SPI1, SPI4, SPI5 and SPI6 are connected to APB2 (PCLK2) */
    /* The prescaler setup bits LTC_SPI_PRESCALER corresponds to the bits 5:3 in the SPI_CR1 register */
    /* Reference manual p.909 */
    /* The shift by 3 puts the bits 5:3 to the first position */
    /* Division are made by powers of 2 which corresponds to shifting to the right */
    /* Then 0 corresponds to divide by 2, 1 corresponds to divide by 4... so 1 has to be added to the value of the
     * configuration bits */

    /* SPI_Clock = HAL_RCC_GetPCLK2Freq()>>((LTC_SPI_PRESCALER>>3)+1);
    } */

    /* Get SPI prescaler */
    prescaler = ((pSpiInterface->pNode->FMT0) >> 8u) & 0xFFu;
    SPI_Clock = (uint32_t)(AVCLK1_FREQ * 1000000u) / (prescaler + 1u);

    return SPI_Clock;
}

/**
 * @brief   sets the transfer time needed to receive/send data with the LTC daisy-chain.
 *
 * This function gets the clock frequency and uses the number of LTCs in the daisy-chain.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 */
static void LTC_SetTransferTimes(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    uint32_t transferTime_us = 0;
    uint32_t SPI_Clock       = 0;

    SPI_Clock = LTC_GetSpiClock(ltc_state->ltcData.pSpiInterface);

    /* Transmission of a command and data */
    /* Multiplication by 1000*1000 to get us */
    transferTime_us = (8u * 1000u * 1000u) / (SPI_Clock);
    transferTime_us *= LTC_N_BYTES_FOR_DATA_TRANSMISSION;
    transferTime_us                    = transferTime_us + LTC_SPI_WAKEUP_WAIT_TIME_US;
    ltc_state->commandDataTransferTime = (transferTime_us / 1000u) + 1u;

    /* Transmission of a command */
    /* Multiplication by 1000*1000 to get us */
    transferTime_us                = ((4u) * 8u * 1000u * 1000u) / (SPI_Clock);
    transferTime_us                = transferTime_us + LTC_SPI_WAKEUP_WAIT_TIME_US;
    ltc_state->commandTransferTime = (transferTime_us / 1000u) + 1u;

    /* Transmission of a command + 9 clocks */
    /* Multiplication by 1000*1000 to get us */
    transferTime_us                   = ((4u + 9u) * 8u * 1000u * 1000u) / (SPI_Clock);
    transferTime_us                   = transferTime_us + LTC_SPI_WAKEUP_WAIT_TIME_US;
    ltc_state->gpioClocksTransferTime = (transferTime_us / 1000u) + 1u;
}

/**
 * @brief   checks the state requests that are made.
 *
 * This function checks the validity of the state requests.
 * The results of the checked is returned immediately.
 *
 * @param  ltc_state:  state of the ltc state machine
 * @param   statereq    state request to be checked
 *
 * @return              result of the state request that was made, taken from LTC_RETURN_TYPE_e
 */
static LTC_RETURN_TYPE_e LTC_CheckStateRequest(LTC_STATE_s *ltc_state, LTC_REQUEST_s statereq) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    LTC_RETURN_TYPE_e retVal = LTC_OK;
    if (statereq.string >= BS_NR_OF_STRINGS) {
        retVal = LTC_ILLEGAL_REQUEST;
    } else if (ltc_state->statereq.request == LTC_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (statereq.request == LTC_STATE_INIT_REQUEST) {
            if (ltc_state->state == LTC_STATEMACH_UNINITIALIZED) {
                retVal = LTC_OK;
            } else {
                retVal = LTC_ALREADY_INITIALIZED;
            }
        } else {
            retVal = LTC_OK;
        }
    } else {
        retVal = LTC_REQUEST_PENDING;
    }

    return retVal;
}

extern bool LTC_IsFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state) {
    FAS_ASSERT(ltc_state != NULL_PTR);
    bool retval = false;

    OS_EnterTaskCritical();
    retval = ltc_state->first_measurement_made;
    OS_ExitTaskCritical();

    return (retval);
}

/**
 * @brief   sets the measurement initialization status.
 */
static void LTC_SetFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state) {
    OS_EnterTaskCritical();
    ltc_state->first_measurement_made = true;
    OS_ExitTaskCritical();
}

extern void LTC_InitializeMonitoringPin(void) {
    /* Set 3rd PE pins to enable daisy chains */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_1);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_2);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_3);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_4);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_5);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_6);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_7);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_0);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_1);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_2);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_3);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_4);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_5);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_6);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PORT_1_PIN_7);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
uint8_t TEST_LTC_CheckReEntrance(LTC_STATE_s *ltc_state) {
    return LTC_CheckReEntrance(ltc_state);
}

extern void TEST_LTC_SetFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state) {
    LTC_SetFirstMeasurementCycleFinished(ltc_state);
}

/** this define is used for creating the declaration of a function for variable extraction */
#define TEST_LTC_DEFINE_GET(VARIABLE)                      \
    extern void TEST_LTC_Get_##VARIABLE(uint8_t data[4]) { \
        for (uint8_t i = 0u; i < 4u; i++) {                \
            data[i] = (uint8_t)(VARIABLE)[i];              \
        }                                                  \
    }
TEST_LTC_DEFINE_GET(ltc_cmdWRCFG)
TEST_LTC_DEFINE_GET(ltc_cmdWRCFG2)
TEST_LTC_DEFINE_GET(ltc_cmdRDCFG)
TEST_LTC_DEFINE_GET(ltc_cmdRDCVA)
TEST_LTC_DEFINE_GET(ltc_cmdRDCVB)
TEST_LTC_DEFINE_GET(ltc_cmdRDCVC)
TEST_LTC_DEFINE_GET(ltc_cmdRDCVD)
TEST_LTC_DEFINE_GET(ltc_cmdRDCVE)
TEST_LTC_DEFINE_GET(ltc_cmdRDCVF)
TEST_LTC_DEFINE_GET(ltc_cmdWRCOMM)
TEST_LTC_DEFINE_GET(ltc_cmdSTCOMM)
TEST_LTC_DEFINE_GET(ltc_cmdRDCOMM)
TEST_LTC_DEFINE_GET(ltc_cmdRDAUXA)
TEST_LTC_DEFINE_GET(ltc_cmdRDAUXB)
TEST_LTC_DEFINE_GET(ltc_cmdRDAUXC)
TEST_LTC_DEFINE_GET(ltc_cmdRDAUXD)
TEST_LTC_DEFINE_GET(ltc_cmdADCV_normal_DCP0)
TEST_LTC_DEFINE_GET(ltc_cmdADCV_normal_DCP1)
TEST_LTC_DEFINE_GET(ltc_cmdADCV_filtered_DCP0)
TEST_LTC_DEFINE_GET(ltc_cmdADCV_filtered_DCP1)
TEST_LTC_DEFINE_GET(ltc_cmdADCV_fast_DCP0)
TEST_LTC_DEFINE_GET(ltc_cmdADCV_fast_DCP1)
TEST_LTC_DEFINE_GET(ltc_cmdADCV_fast_DCP0_twocells)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_normal_GPIO1)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_filtered_GPIO1)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_fast_GPIO1)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_normal_GPIO2)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_filtered_GPIO2)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_fast_GPIO2)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_normal_GPIO3)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_filtered_GPIO3)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_fast_GPIO3)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_normal_ALL_GPIOS)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_filtered_ALL_GPIOS)
TEST_LTC_DEFINE_GET(ltc_cmdADAX_fast_ALL_GPIOS)
TEST_LTC_DEFINE_GET(ltc_BC_cmdADOW_PUP_normal_DCP0)
TEST_LTC_DEFINE_GET(ltc_BC_cmdADOW_PDOWN_normal_DCP0)
TEST_LTC_DEFINE_GET(ltc_BC_cmdADOW_PUP_filtered_DCP0)
TEST_LTC_DEFINE_GET(ltc_BC_cmdADOW_PDOWN_filtered_DCP0)
#endif
