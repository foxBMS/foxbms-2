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
 * @file    ltc_6806.c
 * @author  foxBMS Team
 * @date    2019-09-01 (date of creation)
 * @updated 2021-12-07 (date of last update)
 * @ingroup DRIVERS
 * @prefix  LTC
 *
 * @brief   Driver for the LTC monitoring chip.
 *
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "ltc.h"
#include "ltc_6806_cfg.h"
/* clang-format on */

#include "HL_spi.h"
#include "HL_system.h"

#include "database.h"
#include "diag.h"
#include "io.h"
#include "ltc_pec.h"
#include "pex.h"
#include "os.h"
#include "afe_plausibility.h"

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
 * Value of the LSB in mV
 */
#if LTC_HIRNG == 0u
#define LTC_FUEL_CELL_LSB_RESOLUTION_mV (1.5f)
#else
#define LTC_FUEL_CELL_LSB_RESOLUTION_mV (3.0f)
#endif

/**
 * Value for positive full scale measurement for fuel cell
 */
#define LTC_FUELCELL_POSITIVE_FULLSCALE_RANGE_mV ((int16_t)((0x7FF * LTC_FUEL_CELL_LSB_RESOLUTION_mV)))

/**
 * Value for negative full scale measurement for fuel cell
 */
#define LTC_FUELCELL_NEGATIVE_FULLSCALE_RANGE_mV \
    ((int16_t)((((~0x001) + 1) & 0x7FF) * (-LTC_FUEL_CELL_LSB_RESOLUTION_mV)))

/*========== Static Constant and Variable Definitions =======================*/
/**
 * PEC buffer for RX and TX
 * @{
 */
#pragma SET_DATA_SECTION(".sharedRAM")
uint16_t ltc_RxPecBuffer[LTC_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
uint16_t ltc_TxPecBuffer[LTC_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
#pragma SET_DATA_SECTION()
/**@}*/

/** index of used cells */
static uint16_t ltc_used_cells_index[BS_NR_OF_STRINGS] = {0};
/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s ltc_cellvoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s ltc_celltemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s ltc_allgpiovoltage = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_OPEN_WIRE_s ltc_openwire               = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
/**@}*/
/** stores information on the detected open wires locally */
static LTC_OPENWIRE_DETECTION_s ltc_openWireDetection = {0};
static LTC_ERRORTABLE_s ltc_errorTable                = {0}; /*!< init in LTC_ResetErrorTable-function */

/** local definition of plausible cell voltage values for the LTC 6806 */
static const AFE_PLAUSIBILITY_VALUES_s ltc_plausibleCellVoltages6806 = {
    .maximumPlausibleVoltage_mV = 5000,
    .minimumPlausibleVoltage_mV = -5000,
};

/*========== Extern Constant and Variable Definitions =======================*/

LTC_STATE_s ltc_stateBase = {
    .timer                     = 0,
    .statereq                  = {.request = LTC_STATE_NO_REQUEST, .string = 0xFFu},
    .state                     = LTC_STATEMACH_UNINITIALIZED,
    .substate                  = 0,
    .laststate                 = LTC_STATEMACH_UNINITIALIZED,
    .lastsubstate              = 0,
    .adcModereq                = LTC_ADCMODE_FAST_DCP0,
    .adcMode                   = LTC_ADCMODE_FAST_DCP0,
    .adcMeasChreq              = LTC_ADCMEAS_UNDEFINED,
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
    .muxmeas_seqptr            = NULL_PTR,
    .muxmeas_seqendptr         = NULL_PTR,
    .muxmeas_nr_end            = 0,
    .first_measurement_made    = false,
    .ltc_muxcycle_finished     = STD_NOT_OK,
    .check_spi_flag            = STD_NOT_OK,
    .balance_control_done      = STD_NOT_OK,
    .transmit_ongoing          = false,
    .dummyByte_ongoing         = STD_NOT_OK,
    .ltcData.pSpiInterface     = spi_ltcInterface,
    .ltcData.txBuffer          = ltc_TxPecBuffer,
    .ltcData.rxBuffer          = ltc_RxPecBuffer,
    .ltcData.frameLength       = LTC_N_BYTES_FOR_DATA_TRANSMISSION,
    .ltcData.cellVoltage       = &ltc_cellvoltage,
    .ltcData.cellTemperature   = &ltc_celltemperature,
    .ltcData.balancingFeedback = NULL_PTR,
    .ltcData.balancingControl  = NULL_PTR,
    .ltcData.slaveControl      = NULL_PTR,
    .ltcData.openWireDetection = &ltc_openWireDetection,
    .ltcData.errorTable        = &ltc_errorTable,
    .ltcData.allGpioVoltages   = &ltc_allgpiovoltage,
    .ltcData.openWire          = &ltc_openwire,
    .ltcData.usedCellIndex     = ltc_used_cells_index,
    .currentString             = 0u,
    .requestedString           = 0u,
};

static uint16_t ltc_cmdWRCFG[4] = {0x00, 0x01, 0x3D, 0x6E};
static uint16_t ltc_cmdRDCFG[4] = {0x00, 0x02, 0x2B, 0x0A};

/* static uint16_t ltc_cmdRDAUXA[4] = {0x00, 0x0C, 0xEF, 0xCC};
static uint16_t ltc_cmdRDAUXB[4] = {0x00, 0x0E, 0x72, 0x9A};
static uint16_t ltc_cmdRDAUXC[4] = {0x00, 0x0D, 0x64, 0xFE};
static uint16_t ltc_cmdRDAUXD[4] = {0x00, 0x0F, 0xF9, 0xA8}; */

static uint16_t ltc_cmdRDCVA_Fuelcell[4] = {0x00, 0x04, 0x07, 0xC2};
static uint16_t ltc_cmdRDCVB_Fuelcell[4] = {0x00, 0x05, 0x8C, 0xF0};
static uint16_t ltc_cmdRDCVC_Fuelcell[4] = {0x00, 0x06, 0x9A, 0x94};
static uint16_t ltc_cmdRDCVD_Fuelcell[4] = {0x00, 0x07, 0x11, 0xA6};
static uint16_t ltc_cmdRDCVE_Fuelcell[4] = {0x00, 0x08, 0x5E, 0x52};
static uint16_t ltc_cmdRDCVF_Fuelcell[4] = {0x00, 0x09, 0xD5, 0x60};
static uint16_t ltc_cmdRDCVG_Fuelcell[4] = {0x00, 0x0A, 0xC3, 0x04};
static uint16_t ltc_cmdRDCVH_Fuelcell[4] = {0x00, 0x0B, 0x48, 0x36};
static uint16_t ltc_cmdRDCVI_Fuelcell[4] = {0x00, 0x0C, 0xEF, 0xCC};

/* static uint16_t ltc_cmdMUTE[4] = {0x00, 0x28, 0xE8, 0x0E};                    !< MUTE discharging via S pins */
/* static uint16_t ltc_cmdUNMUTE[4] = {0x00, 0x29, 0x63, 0x3C};                  !< UN-MUTE discharging via S pins */

static uint16_t ltc_cmdADCV_normal_Fuelcell[4] = {0x04, 0x40, 0xED, 0xB0}; /*!< All cells, normal mode */

/* GPIOs  */
/* static uint16_t ltc_cmdADAX_normal_GPIO1[4]   = {0x05, 0x61, 0x58, 0x92}; !< Single channel, GPIO 1, normal mode   */
/* static uint16_t ltc_cmdADAX_filtered_GPIO1[4] = {0x05, 0xE1, 0x1C, 0xB4}; !< Single channel, GPIO 1, filtered mode */
/* static uint16_t ltc_cmdADAX_fast_GPIO1[4]     = {0x04, 0xE1, 0x94, 0xF8}; !< Single channel, GPIO 1, fast mode     */
/* static uint16_t ltc_cmdADAX_normal_GPIO2[4]   = {0x05, 0x62, 0x4E, 0xF6}; !< Single channel, GPIO 2, normal mode   */
/* static uint16_t ltc_cmdADAX_filtered_GPIO2[4] = {0x05, 0xE2, 0x0A, 0xD0}; !< Single channel, GPIO 2, filtered mode */
/* static uint16_t ltc_cmdADAX_fast_GPIO2[4]     = {0x04, 0xE2, 0x82, 0x9C}; !< Single channel, GPIO 2, fast mode     */
/* static uint16_t ltc_cmdADAX_normal_GPIO3[4]   = {0x05, 0x63, 0xC5, 0xC4}; !< Single channel, GPIO 3, normal mode   */
/* static uint16_t ltc_cmdADAX_filtered_GPIO3[4] = {0x05, 0xE3, 0x81, 0xE2}; !< Single channel, GPIO 3, filtered mode */
/* static uint16_t ltc_cmdADAX_fast_GPIO3[4]     = {0x04, 0xE3, 0x09, 0xAE}; !< Single channel, GPIO 3, fast mode     */
/* static uint16_t ltc_cmdADAX_normal_GPIO4[4] = {0x05, 0x64, 0x62, 0x3E};      !< Single channel, GPIO 4, normal mode   */
/* static uint16_t ltc_cmdADAX_filtered_GPIO4[4] = {0x05, 0xE4, 0x26, 0x18};    !< Single channel, GPIO 4, filtered mode */
/* static uint16_t ltc_cmdADAX_fast_GPIO4[4] = {0x04, 0xE4, 0xAE, 0x54};        !< Single channel, GPIO 4, fast mode     */
/* static uint16_t ltc_cmdADAX_normal_GPIO5[4] = {0x05, 0x65, 0xE9, 0x0C};      !< Single channel, GPIO 5, normal mode   */
/* static uint16_t ltc_cmdADAX_filtered_GPIO5[4] = {0x05, 0xE5, 0xAD, 0x2A};    !< Single channel, GPIO 5, filtered mode */
/* static uint16_t ltc_cmdADAX_fast_GPIO5[4] = {0x04, 0xE5, 0x25, 0x66};        !< Single channel, GPIO 5, fast mode     */
/* static uint16_t ltc_cmdADAX_normal_ALLGPIOS[4] = {0x05, 0x60, 0xD3, 0xA0}; !< All channels, normal mode             */
/* static uint16_t ltc_cmdADAX_filtered_ALLGPIOS[4] =
    {0x05, 0xE0, 0x97, 0x86};                                            !< All channels, filtered mode           */
/* static uint16_t ltc_cmdADAX_fast_ALLGPIOS[4] = {0x04, 0xE0, 0x1F, 0xCA}; !< All channels, fast mode               */

/* Open-wire */
static uint16_t ltc_BC_cmdADOW_PUP_100ms_fuelcell[4] =
    {0x07, 0xC0, 0xBA, 0x70}; /*!< Broadcast, Pull-up current, All cells, normal mode, 100ms   */
static uint16_t ltc_BC_cmdADOW_PDOWN_100ms_fuelcell[4] =
    {0x06, 0xC0, 0x32, 0x3C}; /*!< Broadcast, Pull-down current, All cells, normal mode, 100ms */

/*========== Static Function Prototypes =====================================*/
static void LTC_SetFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state);
static void LTC_Initialize_Database(LTC_STATE_s *ltc_state);
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
static STD_RETURN_TYPE_e LTC_StartOpenWireMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    uint8_t PUP);

static void LTC_SaveRXtoVoltagebuffer_Fuelcell(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t registerSet,
    uint8_t stringNumber);

static STD_RETURN_TYPE_e LTC_RX_PECCheck(
    LTC_STATE_s *ltc_state,
    uint16_t *DataBufferSPI_RX_with_PEC,
    uint8_t stringNumber);
static STD_RETURN_TYPE_e LTC_RX(
    uint16_t *Command,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength);

static uint32_t LTC_GetSPIClock(SPI_INTERFACE_CONFIG_s *pSpiInterface);
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
static void LTC_Initialize_Database(LTC_STATE_s *ltc_state) {
    uint16_t i = 0;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        ltc_state->ltcData.cellVoltage->state = 0;
        for (i = 0; i < BS_NR_OF_BAT_CELLS; i++) {
            ltc_state->ltcData.cellVoltage->cellVoltage_mV[stringNumber][i]      = 0;
            ltc_state->ltcData.openWireDetection->openWirePup[stringNumber][i]   = 0;
            ltc_state->ltcData.openWireDetection->openWirePdown[stringNumber][i] = 0;
            ltc_state->ltcData.openWireDetection->openWireDelta[stringNumber][i] = 0;
        }

        ltc_state->ltcData.cellTemperature->state = 0;
        for (i = 0; i < BS_NR_OF_TEMP_SENSORS_PER_STRING; i++) {
            ltc_state->ltcData.cellTemperature->cellTemperature_ddegC[stringNumber][i] = 0;
        }

        ltc_state->ltcData.allGpioVoltages->state = 0;
        for (i = 0; i < (BS_NR_OF_MODULES * BS_NR_OF_GPIOS_PER_MODULE); i++) {
            ltc_state->ltcData.allGpioVoltages->gpioVoltages_mV[stringNumber][i] = 0;
        }

        for (i = 0; i < (BS_NR_OF_MODULES * (BS_NR_OF_CELLS_PER_MODULE + 1)); i++) {
            ltc_state->ltcData.openWire->openwire[stringNumber][i] = 0;
        }
        ltc_state->ltcData.openWire->state = 0;
    }

    DATA_WRITE_DATA(ltc_state->ltcData.cellVoltage, ltc_state->ltcData.cellTemperature, ltc_state->ltcData.openWire);
}

/**
 * @brief Saves the last state and the last substate
 *
 * @param  ltc_state:  state of the ltc state machine
 */
static void LTC_SaveLastStates(LTC_STATE_s *ltc_state) {
    ltc_state->laststate    = ltc_state->state;
    ltc_state->lastsubstate = ltc_state->substate;
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
 * If retVal is #STD_OK, after timer_ms_ok is elapsed the LTC statemachine will
 * transition into state_ok and substate_ok, otherwise after timer_ms_nok the
 * statemachine will transition to state_nok and substate_nok. Depending on
 * value of retVal the corresponding diagnosis entry will be called.
 *
 * @param  ltc_state:    state of the ltc state machine
 * @param  retVal:       condition to determine if statemachine will transition into ok or nok states
 * @param  diagCode:     symbolic IDs for diagnosis entry, called with #DIAG_EVENT_OK if retVal is #STD_OK, #DIAG_EVENT_NOT_OK otherwise
 * @param  state_ok      state to transition into if retVal is #STD_OK
 * @param  substate_ok:  substate to transition into if retVal is #STD_OK
 * @param  timer_ms_ok:  transition into state_ok, substate_ok after timer_ms_ok elapsed
 * @param  state_nok:    state to transition into if retVal is #STD_NOT_OK
 * @param  substate_nok: substate to transition into if retVal is #STD_NOT_OK
 * @param  timer_ms_nok: transition into state_nok, substate_nok after timer_ms_nok elapsed
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

/*========== Extern Function Implementations ================================*/
extern void LTC_SaveVoltages(LTC_STATE_s *ltc_state, uint8_t stringNumber) {
    /* Pointer validity check */
    FAS_ASSERT(ltc_state != NULL_PTR);

    /* Iterate over all cell to:
     *
     * 1. Check open-wires and set respective cell measurements to invalid
     * 2. Perform minimum/maximum measurement value plausibility check
     * 3. Calculate string values
     */
    STD_RETURN_TYPE_e cellVoltageMeasurementValid = STD_OK;
    int32_t stringVoltage_mV                      = 0;
    uint16_t numberValidMeasurements              = 0;
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
        for (uint8_t c = 0u; c < BS_NR_OF_CELLS_PER_MODULE; c++) {
            /* ------- 1. Check open-wires -----------------
                 * Is cell N input not open wire &&
                 * Is cell N+1 input not open wire &&
                 * Is cell voltage valid because of previous PEC error
                 * If so, everything okay, else set cell voltage measurement to invalid.
                 */
            if ((ltc_state->ltcData.openWire->openwire[stringNumber][(m * (BS_NR_OF_CELLS_PER_MODULE + 1u)) + c] ==
                 0u) &&
                (ltc_state->ltcData.openWire->openwire[stringNumber][(m * (BS_NR_OF_CELLS_PER_MODULE + 1u)) + c + 1u] ==
                 0u) &&
                ((ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][m] & (0x01u << c)) == 0u)) {
                /* Cell voltage is valid -> perform minimum/maximum plausibility check */

                /* ------- 2. Perform minimum/maximum measurement range check ---------- */
                if (STD_OK == AFE_PlausibilityCheckVoltageMeasurementRange(
                                  ltc_state->ltcData.cellVoltage
                                      ->cellVoltage_mV[stringNumber][(m * BS_NR_OF_CELLS_PER_MODULE) + c],
                                  ltc_plausibleCellVoltages6806)) {
                    /* Cell voltage is valid ->  calculate string voltage */
                    /* -------- 3. Calculate string values ------------- */
                    stringVoltage_mV += ltc_state->ltcData.cellVoltage
                                            ->cellVoltage_mV[stringNumber][(m * BS_NR_OF_CELLS_PER_MODULE) + c];
                    numberValidMeasurements++;
                } else {
                    /* Invalidate cell voltage measurement */
                    ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][m] |= (0x01u << c);
                    cellVoltageMeasurementValid = STD_NOT_OK;
                }
            } else {
                /* Set cell voltage measurement value invalid, if not already invalid because of PEC Error */
                ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][m] |= (0x01u << c);
                cellVoltageMeasurementValid = STD_NOT_OK;
            }
        }
    }
    DIAG_CheckEvent(cellVoltageMeasurementValid, DIAG_ID_AFE_CELL_VOLTAGE_MEAS_ERROR, DIAG_STRING, stringNumber);
    ltc_state->ltcData.cellVoltage->packVoltage_mV[stringNumber]      = stringVoltage_mV;
    ltc_state->ltcData.cellVoltage->nrValidCellVoltages[stringNumber] = numberValidMeasurements;

    /* Increment state variable each time new values are written into database */
    ltc_state->ltcData.cellVoltage->state++;

    DATA_WRITE_DATA(ltc_state->ltcData.cellVoltage);
}

extern void LTC_SaveTemperatures(LTC_STATE_s *ltc_state, uint8_t stringNumber) {
    STD_RETURN_TYPE_e cellTemperatureMeasurementValid = STD_OK;
    uint16_t numberValidMeasurements                  = 0;
    for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
        for (uint8_t c = 0u; c < BS_NR_OF_TEMP_SENSORS_PER_MODULE; c++) {
            /* ------- 1. Check valid flag  -----------------
                 * Is cell temperature valid because of previous PEC error
                 * If so, everything okay, else set cell temperature measurement to invalid.
                 */
            if ((ltc_state->ltcData.cellTemperature->invalidCellTemperature[stringNumber][m] & (0x01u << c)) == 0u) {
                /* Cell temperature is valid -> perform minimum/maximum plausibility check */

                /* ------- 2. Perform minimum/maximum measurement range check ---------- */
                if (STD_OK ==
                    AFE_PlausibilityCheckTempMinMax(
                        ltc_state->ltcData.cellTemperature
                            ->cellTemperature_ddegC[stringNumber][(m * BS_NR_OF_TEMP_SENSORS_PER_MODULE) + c])) {
                    numberValidMeasurements++;
                } else {
                    /* Invalidate cell temperature measurement */
                    ltc_state->ltcData.cellTemperature->invalidCellTemperature[stringNumber][m] |= (0x01u << c);
                    cellTemperatureMeasurementValid = STD_NOT_OK;
                }
            } else {
                /* Already invalid because of PEC Error */
                cellTemperatureMeasurementValid = STD_NOT_OK;
            }
        }
    }
    DIAG_CheckEvent(
        cellTemperatureMeasurementValid, DIAG_ID_AFE_CELL_TEMPERATURE_MEAS_ERROR, DIAG_STRING, stringNumber);

    ltc_state->ltcData.cellTemperature->nrValidTemperatures[stringNumber] = numberValidMeasurements;
    ltc_state->ltcData.cellTemperature->state++;
    DATA_WRITE_DATA(ltc_state->ltcData.cellTemperature);
}

/**
 * @brief   stores the measured GPIOs in the database.
 *
 * This function loops through the data of all modules in the LTC daisy-chain that are
 * stored in the ltc_allgpiovoltage buffer and writes them in the database.
 * At each write iteration, the variable named "state" and related to voltages in the
 * database is incremented.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 */
extern void LTC_SaveAllGPIOMeasurement(LTC_STATE_s *ltc_state) {
    ltc_state->ltcData.allGpioVoltages->state++;
    DATA_WRITE_DATA(ltc_state->ltcData.allGpioVoltages);
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

/**
 * @brief   gets the current state request.
 *
 * This function is used in the functioning of the LTC state machine.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 * @return  retval  current state request, taken from LTC_STATE_REQUEST_e
 */
extern LTC_REQUEST_s LTC_GetStateRequest(LTC_STATE_s *ltc_state) {
    LTC_REQUEST_s retval = {.request = LTC_STATE_NO_REQUEST, .string = 0x0u};

    OS_EnterTaskCritical();
    retval.request = ltc_state->statereq.request;
    retval.string  = ltc_state->statereq.string;
    OS_ExitTaskCritical();

    return (retval);
}

/**
 * @brief   gets the current state.
 *
 * This function is used in the functioning of the LTC state machine.
 *
 * @param  ltc_state:  state of the ltc state machine
 *
 * @return  current state, taken from LTC_STATEMACH_e
 */
extern LTC_STATEMACH_e LTC_GetState(LTC_STATE_s *ltc_state) {
    return ltc_state->state;
}

/**
 * @brief   transfers the current state request to the state machine.
 *
 * This function takes the current state request from ltc_state and transfers it to the state machine.
 * It resets the value from ltc_state to LTC_STATE_NO_REQUEST
 *
 * @param   ltc_state       state of the ltc state machine
 * @param   pBusIDptr       bus ID, main or backup (deprecated)
 * @param   pAdcModeptr     LTC ADCmeasurement mode (fast, normal or filtered)
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
    LTC_REQUEST_s retval = {.request = LTC_STATE_NO_REQUEST, .string = 0x0u};

    OS_EnterTaskCritical();
    retval.request              = ltc_state->statereq.request;
    retval.string               = ltc_state->statereq.string;
    ltc_state->requestedString  = ltc_state->statereq.string;
    *pAdcModeptr                = ltc_state->adcModereq;
    *pAdcMeasChptr              = ltc_state->adcMeasChreq;
    ltc_state->statereq.request = LTC_STATE_NO_REQUEST;
    ltc_state->statereq.string  = 0x0u;
    OS_ExitTaskCritical();

    return (retval);
}

LTC_RETURN_TYPE_e LTC_SetStateRequest(LTC_STATE_s *ltc_state, LTC_REQUEST_s statereq) {
    LTC_RETURN_TYPE_e retVal = LTC_ERROR;

    OS_EnterTaskCritical();
    retVal = LTC_CheckStateRequest(ltc_state, statereq);

    if ((retVal == LTC_OK) || (retVal == LTC_BUSY_OK) || (retVal == LTC_OK_FROM_ERROR)) {
        ltc_state->statereq.request = statereq.request;
        ltc_state->statereq.string  = statereq.string;
    }
    OS_ExitTaskCritical();

    return (retVal);
}

void LTC_Trigger(LTC_STATE_s *ltc_state) {
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
                    LTC_Initialize_Database(ltc_state);
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
                    ltc_state->spiSeqPtr           = ltc_state->ltcData.pSpiInterface;
                    ltc_state->spiNumberInterfaces = BS_NR_OF_STRINGS;
                    ltc_state->spiSeqEndPtr        = ltc_state->ltcData.pSpiInterface + BS_NR_OF_STRINGS;
                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_INITIALIZATION, LTC_ENTRY_INITIALIZATION, LTC_STATEMACH_SHORTTIME);
                } else if (ltc_state->substate == LTC_ENTRY_INITIALIZATION) {
                    LTC_SaveLastStates(ltc_state);
                    retVal = LTC_TransmitWakeUp(ltc_state->spiSeqPtr); /* Send dummy byte to wake up the daisy chain */
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_RE_ENTRY_INITIALIZATION,
                        LTC_STATEMACH_DAISY_CHAIN_FIRST_INITIALIZATION_TIME,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_ENTRY_INITIALIZATION,
                        LTC_STATEMACH_SHORTTIME);

                } else if (ltc_state->substate == LTC_RE_ENTRY_INITIALIZATION) {
                    LTC_SaveLastStates(ltc_state);
                    retVal =
                        LTC_TransmitWakeUp(ltc_state->spiSeqPtr); /* Send dummy byte again to wake up the daisy chain */
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
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
                    ltc_state->lastsubstate = ltc_state->substate;
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_SPI, DIAG_STRING, ltc_state->currentString);
                    LTC_StateTransition(
                        ltc_state,
                        LTC_STATEMACH_INITIALIZATION,
                        LTC_CHECK_INITIALIZATION,
                        ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT);

                } else if (ltc_state->substate == LTC_CHECK_INITIALIZATION) {
                    /* Read values written in config register, currently unused */
                    LTC_SaveLastStates(ltc_state);
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
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
                ltc_state->adcMeasCh = LTC_ADCMEAS_ALLCHANNEL_CELLS;

                ltc_state->spiSeqPtr           = ltc_state->ltcData.pSpiInterface;
                ltc_state->spiNumberInterfaces = BS_NR_OF_STRINGS;
                ltc_state->spiSeqEndPtr        = ltc_state->ltcData.pSpiInterface + BS_NR_OF_STRINGS;
                ltc_state->currentString       = 0u;

                ltc_state->check_spi_flag = STD_NOT_OK;
                retVal = LTC_StartVoltageMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, ltc_state->adcMeasCh);

                LTC_CondBasedStateTransition(
                    ltc_state,
                    retVal,
                    DIAG_ID_LTC_SPI,
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    (ltc_state->commandTransferTime + LTC_FUELCELL_NORMAL_ALL_CELLS_MS),
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    LTC_STATEMACH_SHORTTIME);

                break;

            /****************************START MEASUREMENT CONTINUE*******************************/
            /* Do not reset SPI interface pointer */
            case LTC_STATEMACH_STARTMEAS_CONTINUE:

                ltc_state->adcMode   = LTC_VOLTAGE_MEASUREMENT_MODE;
                ltc_state->adcMeasCh = LTC_ADCMEAS_ALLCHANNEL_CELLS;

                ltc_state->check_spi_flag = STD_NOT_OK;
                retVal = LTC_StartVoltageMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, ltc_state->adcMeasCh);

                LTC_CondBasedStateTransition(
                    ltc_state,
                    retVal,
                    DIAG_ID_LTC_SPI,
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    (ltc_state->commandTransferTime + LTC_FUELCELL_NORMAL_ALL_CELLS_MS),
                    LTC_STATEMACH_READVOLTAGE,
                    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                    LTC_STATEMACH_SHORTTIME);

                break;

            /****************************READ VOLTAGE************************************/
            case LTC_STATEMACH_READVOLTAGE:

                if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE) {
                    ltc_state->check_spi_flag = STD_OK;
                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVA_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_B_RDCVB_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_B_RDCVB_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_B_RDCVB_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 0u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVB_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_C_RDCVC_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_C_RDCVC_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_C_RDCVC_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 1u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVC_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_D_RDCVD_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_D_RDCVD_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_D_RDCVD_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 2u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVD_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_E_RDCVE_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_E_RDCVE_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_E_RDCVE_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 3u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVE_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_F_RDCVF_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_F_RDCVF_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_F_RDCVF_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 4u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVF_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_G_RDCVG_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_G_RDCVG_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_G_RDCVG_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 5u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVG_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_H_RDCVH_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_H_RDCVH_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_H_RDCVH_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 6u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVH_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_I_RDCVI_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_READ_VOLTAGE_REGISTER_I_RDCVI_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_READ_VOLTAGE_REGISTER_I_RDCVI_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 7u, ltc_state->currentString);

                    AFE_SetTransmitOngoing(ltc_state);
                    retVal = LTC_RX(
                        ltc_cmdRDCVI_Fuelcell,
                        ltc_state->spiSeqPtr,
                        ltc_state->ltcData.txBuffer,
                        ltc_state->ltcData.rxBuffer,
                        ltc_state->ltcData.frameLength);
                    LTC_CondBasedStateTransition(
                        ltc_state,
                        retVal,
                        DIAG_ID_LTC_SPI,
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_EXIT_READVOLTAGE,
                        (ltc_state->commandDataTransferTime + LTC_TRANSMISSION_TIMEOUT),
                        LTC_STATEMACH_READVOLTAGE,
                        LTC_EXIT_READVOLTAGE,
                        LTC_STATEMACH_SHORTTIME);
                    break;

                } else if (ltc_state->substate == LTC_EXIT_READVOLTAGE) {
                    retVal = LTC_RX_PECCheck(ltc_state, ltc_state->ltcData.rxBuffer, ltc_state->currentString);
                    DIAG_CheckEvent(retVal, DIAG_ID_LTC_PEC, DIAG_STRING, ltc_state->currentString);
                    LTC_SaveRXtoVoltagebuffer_Fuelcell(
                        ltc_state, ltc_state->ltcData.rxBuffer, 8u, ltc_state->currentString);

                    /* Switch to different state if read voltage state is reused
                * e.g. open-wire check...                                */
                    if (ltc_state->reusageMeasurementMode == LTC_NOT_REUSED) {
                        LTC_SaveVoltages(ltc_state, ltc_state->currentString);

                        ++ltc_state->spiSeqPtr;
                        ++ltc_state->currentString;
                        if (ltc_state->spiSeqPtr >= ltc_state->spiSeqEndPtr) {
                            if (LTC_IsFirstMeasurementCycleFinished(ltc_state) == false) {
                                LTC_SetFirstMeasurementCycleFinished(ltc_state);
                            }
                            statereq = LTC_TransferStateRequest(ltc_state, &tmpbusID, &tmpadcMode, &tmpadcMeasCh);
                            if (statereq.request == LTC_STATE_OPENWIRE_CHECK_REQUEST) {
                                if (statereq.string < BS_NR_OF_STRINGS) {
                                    ltc_state->spiSeqPtr       = ltc_state->ltcData.pSpiInterface + statereq.string;
                                    ltc_state->requestedString = statereq.string;
                                    /* This is necessary because the state machine will go through read voltage measurement registers */
                                    ltc_state->currentString        = statereq.string;
                                    ltc_state->resendCommandCounter = LTC_NMBR_REQ_ADOW_COMMANDS;
                                    LTC_StateTransition(
                                        ltc_state,
                                        LTC_STATEMACH_OPENWIRE_CHECK,
                                        LTC_REQUEST_PULLUP_CURRENT_OPENWIRE_CHECK,
                                        LTC_STATEMACH_SHORTTIME);
                                }
                            } else {
                                LTC_StateTransition(
                                    ltc_state, LTC_STATEMACH_STARTMEAS, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                                ltc_state->check_spi_flag = STD_NOT_OK;
                            }
                        } else {
                            LTC_StateTransition(
                                ltc_state, LTC_STATEMACH_STARTMEAS_CONTINUE, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                            ltc_state->check_spi_flag = STD_NOT_OK;
                        }
                    } else if (ltc_state->reusageMeasurementMode == LTC_REUSE_READVOLT_FOR_ADOW_PUP) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_READ_VOLTAGES_PULLUP_OPENWIRE_CHECK,
                            LTC_STATEMACH_SHORTTIME);
                    } else if (ltc_state->reusageMeasurementMode == LTC_REUSE_READVOLT_FOR_ADOW_PDOWN) {
                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_READ_VOLTAGES_PULLDOWN_OPENWIRE_CHECK,
                            LTC_STATEMACH_SHORTTIME);
                    }
                }
                break;

            /**************************OPEN-WIRE CHECK*******************************/
            case LTC_STATEMACH_OPENWIRE_CHECK:
                if (ltc_state->substate == LTC_REQUEST_PULLUP_CURRENT_OPENWIRE_CHECK) {
                    /* Run ADOW command with PUP = 1 */
                    ltc_state->adcMode        = LTC_OW_MEASUREMENT_MODE;
                    ltc_state->check_spi_flag = STD_NOT_OK;

                    retVal = LTC_StartOpenWireMeasurement(ltc_state->spiSeqPtr, ltc_state->adcMode, 1);
                    if (retVal == STD_OK) {
                        DIAG_Handler(DIAG_ID_LTC_SPI, DIAG_EVENT_OK, DIAG_STRING, ltc_state->requestedString);

                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_REQUEST_PULLUP_CURRENT_OPENWIRE_CHECK,
                            (ltc_state->commandDataTransferTime + LTC_FUEL_CELL_ADOW_TIME_MS));

                        ltc_state->resendCommandCounter--;

                        /* Check how many retries are left */
                        if (ltc_state->resendCommandCounter == 0) {
                            /* Switch to read voltage state to read cell voltages */

                            LTC_StateTransition(
                                ltc_state,
                                LTC_STATEMACH_READVOLTAGE,
                                LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                                (ltc_state->commandDataTransferTime + LTC_FUEL_CELL_ADOW_TIME_MS));

                            /* Reuse read voltage register */
                            ltc_state->reusageMeasurementMode = LTC_REUSE_READVOLT_FOR_ADOW_PUP;
                        }
                    } else {
                        DIAG_Handler(DIAG_ID_LTC_SPI, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->requestedString);
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_STARTMEAS_CONTINUE, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    }
                } else if (ltc_state->substate == LTC_READ_VOLTAGES_PULLUP_OPENWIRE_CHECK) {
                    /* Previous state: Read voltage -> information stored in voltage buffer */
                    ltc_state->reusageMeasurementMode = LTC_NOT_REUSED;

                    /* Copy data from voltage struct into open-wire struct */
                    for (uint16_t i = 0u; i < BS_NR_OF_BAT_CELLS; i++) {
                        ltc_state->ltcData.openWireDetection->openWirePup[ltc_state->requestedString][i] =
                            ltc_state->ltcData.cellVoltage->cellVoltage_mV[ltc_state->requestedString][i];
                    }

                    /* Set number of ADOW retries - send ADOW command with pull-down two times */
                    ltc_state->resendCommandCounter = LTC_NMBR_REQ_ADOW_COMMANDS;
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
                        DIAG_Handler(DIAG_ID_LTC_SPI, DIAG_EVENT_OK, DIAG_STRING, ltc_state->requestedString);

                        LTC_StateTransition(
                            ltc_state,
                            LTC_STATEMACH_OPENWIRE_CHECK,
                            LTC_REQUEST_PULLDOWN_CURRENT_OPENWIRE_CHECK,
                            (ltc_state->commandDataTransferTime + LTC_FUEL_CELL_ADOW_TIME_MS));

                        ltc_state->resendCommandCounter--;

                        /* Check how many retries are left */
                        if (ltc_state->resendCommandCounter == 0) {
                            /* Switch to read voltage state to read cell voltages */

                            LTC_StateTransition(
                                ltc_state,
                                LTC_STATEMACH_READVOLTAGE,
                                LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE,
                                (ltc_state->commandDataTransferTime + LTC_FUEL_CELL_ADOW_TIME_MS));

                            /* Reuse read voltage register */
                            ltc_state->reusageMeasurementMode = LTC_REUSE_READVOLT_FOR_ADOW_PDOWN;
                        }
                    } else {
                        DIAG_Handler(DIAG_ID_LTC_SPI, DIAG_EVENT_NOT_OK, DIAG_STRING, ltc_state->requestedString);
                        LTC_StateTransition(
                            ltc_state, LTC_STATEMACH_STARTMEAS_CONTINUE, LTC_ENTRY, LTC_STATEMACH_SHORTTIME);
                    }
                } else if (ltc_state->substate == LTC_READ_VOLTAGES_PULLDOWN_OPENWIRE_CHECK) {
                    /* Previous state: Read voltage -> information stored in voltage buffer */
                    ltc_state->reusageMeasurementMode = LTC_NOT_REUSED;

                    /* Copy data from voltage struct into open-wire struct */
                    for (uint16_t i = 0u; i < BS_NR_OF_BAT_CELLS; i++) {
                        ltc_state->ltcData.openWireDetection->openWirePdown[ltc_state->requestedString][i] =
                            ltc_state->ltcData.cellVoltage->cellVoltage_mV[ltc_state->requestedString][i];
                    }

                    LTC_StateTransition(
                        ltc_state, LTC_STATEMACH_OPENWIRE_CHECK, LTC_PERFORM_OPENWIRE_CHECK, LTC_STATEMACH_SHORTTIME);

                } else if (ltc_state->substate == LTC_PERFORM_OPENWIRE_CHECK) {
                    /* Perform actual open-wire check */

                    /* Take difference between pull-up and pull-down measurement */
                    for (uint16_t i = 1u; i < BS_NR_OF_BAT_CELLS; i++) {
                        ltc_state->ltcData.openWireDetection->openWireDelta[ltc_state->requestedString][i] = (int32_t)(
                            ltc_state->ltcData.openWireDetection->openWirePup[ltc_state->requestedString][i] -
                            ltc_state->ltcData.openWireDetection->openWirePdown[ltc_state->requestedString][i]);
                    }

                    /* PDOWN or PUP positive or negative full scale value: C(N) or C(N-1) open*/
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
                        /* PUP */
                        for (uint8_t p = 0u; p < BS_NR_OF_CELLS_PER_MODULE; p++) {
                            if ((ltc_state->ltcData.openWireDetection
                                     ->openWirePup[ltc_state->requestedString][p + (m * BS_NR_OF_CELLS_PER_MODULE)] ==
                                 LTC_FUELCELL_POSITIVE_FULLSCALE_RANGE_mV) ||
                                (ltc_state->ltcData.openWireDetection
                                     ->openWirePup[ltc_state->requestedString][p + (m * BS_NR_OF_CELLS_PER_MODULE)] ==
                                 LTC_FUELCELL_NEGATIVE_FULLSCALE_RANGE_mV)) {
                                ltc_state->ltcData.openWire
                                    ->openwire[ltc_state->requestedString][p + (m * (BS_NR_OF_CELLS_PER_MODULE))] = 1;
                                ltc_state->ltcData.openWire->openwire[ltc_state->requestedString]
                                                                     [(p + 1u) + (m * (BS_NR_OF_CELLS_PER_MODULE))] = 1;
                            }
                            if ((ltc_state->ltcData.openWireDetection
                                     ->openWirePdown[ltc_state->requestedString][p + (m * BS_NR_OF_CELLS_PER_MODULE)] ==
                                 LTC_FUELCELL_POSITIVE_FULLSCALE_RANGE_mV) ||
                                (ltc_state->ltcData.openWireDetection
                                     ->openWirePdown[ltc_state->requestedString][p + (m * BS_NR_OF_CELLS_PER_MODULE)] ==
                                 LTC_FUELCELL_NEGATIVE_FULLSCALE_RANGE_mV)) {
                                ltc_state->ltcData.openWire
                                    ->openwire[ltc_state->requestedString][p + (m * (BS_NR_OF_CELLS_PER_MODULE))] = 1;
                                ltc_state->ltcData.openWire->openwire[ltc_state->requestedString]
                                                                     [(p + 1u) + (m * (BS_NR_OF_CELLS_PER_MODULE))] = 1;
                            }
                        }
                    }

                    /* data sheet page 28: "for all values of n from 1 to 36: If CELL Delta(n+1) < -200mV then C(n) is open" */
                    for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
                        /* ltc_state->ltcData.openWireDelta parsed from 1 to (BS_NR_OF_CELLS_PER_MODULE-1) */
                        for (uint8_t c = 1u; c < BS_NR_OF_CELLS_PER_MODULE; c++) {
                            /* If delta cell(n+1) < -200mV: open-wire at C(n) */
                            if (ltc_state->ltcData.openWireDetection
                                    ->openWireDelta[ltc_state->requestedString][c + (m * BS_NR_OF_CELLS_PER_MODULE)] <
                                LTC_ADOW_THRESHOLD) {
                                ltc_state->ltcData.openWire
                                    ->openwire[ltc_state->requestedString][c + (m * BS_NR_OF_CELLS_PER_MODULE)] = 1;
                            }
                        }
                    }

                    ltc_state->ltcData.openWire->nrOpenWires[ltc_state->requestedString] = 0;
                    for (uint16_t c = 0u; c < (BS_NR_OF_MODULES * (BS_NR_OF_CELLS_PER_MODULE + 1)); c++) {
                        if (ltc_state->ltcData.openWire->openwire[ltc_state->requestedString][c] == 1) {
                            ltc_state->ltcData.openWire->nrOpenWires[ltc_state->requestedString]++;
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
    }                              /* continueFunction */
}

/**
 * @brief   saves the voltage values read from the LTC daisy-chain.
 *
 * After a voltage measurement was initiated to measure the voltages of the cells,
 * the result is read via SPI from the daisy-chain.
 * There are 6 register to read _(A,B,C,D,E,F,G,H,I) to get all cell voltages.
 * Only one register can be read at a time.
 * This function is called to store the result from the transmission in a buffer.
 *
 * @param   ltc_state      state of the ltc state machine
 * @param   pRxBuff        receive buffer
 * @param   registerSet    voltage register that was read (voltage register A,B,C,D,E,F,G,H or I)
 * @param  stringNumber    string addressed
 *
 */
static void LTC_SaveRXtoVoltagebuffer_Fuelcell(
    LTC_STATE_s *ltc_state,
    uint16_t *pRxBuff,
    uint8_t registerSet,
    uint8_t stringNumber) {
    uint16_t cellOffset = 0;
    uint16_t val_ui     = 0;
    int16_t voltage     = 0;
    uint64_t bitmask    = 0;
    uint16_t buffer_LSB = 0;
    uint16_t buffer_MSB = 0;

    cellOffset = registerSet * 4u;

    /* Calculate bitmask for valid flags */
    bitmask |= 0x0Full << cellOffset; /* 0x0F: four voltages in each register */

    /* reinitialize index counter at begin of cycle */
    if (cellOffset == 0u) {
        (ltc_state->ltcData.usedCellIndex[stringNumber]) = 0u;
    }

    /* Retrieve data without command and CRC*/
    for (uint8_t m = 0u; m < LTC_N_LTC; m++) {
        uint8_t incrementations = 0u;

        /* parse all four voltages (4 * 12bits) contained in one register */
        for (uint8_t c = 0u; c < 4u; c++) {
            switch (c) {
                case 0u:
                    buffer_MSB = pRxBuff[4u + (m * 8u)];
                    buffer_LSB = (pRxBuff[4u + 1u + (m * 8u)]) >> 4u;
                    val_ui     = (uint16_t)(buffer_LSB | (buffer_MSB << 4u));
                    break;
                case 1u:
                    buffer_MSB = pRxBuff[4u + 1u + (m * 8u)] & 0x0Fu;
                    buffer_LSB = (pRxBuff[4u + 2u + (m * 8u)]);
                    val_ui     = (uint16_t)(buffer_LSB | (buffer_MSB << 8u));
                    break;
                case 2u:
                    buffer_MSB = pRxBuff[4u + 3u + (m * 8u)];
                    buffer_LSB = (pRxBuff[4u + 4u + (m * 8u)]) >> 4u;
                    val_ui     = (uint16_t)(buffer_LSB | (buffer_MSB << 4u));
                    break;
                case 3u:
                    buffer_MSB = pRxBuff[4u + 4u + (m * 8u)] & 0x0Fu;
                    buffer_LSB = (pRxBuff[4u + 5u + (m * 8u)]);
                    val_ui     = (uint16_t)(buffer_LSB | (buffer_MSB << 8u));
                    break;
                default:
                    break;
            }

            /* Check signed bit if measured value is negative or not */
            if ((val_ui & (1u << (12u - 1u))) == 0u) {
                voltage = (int16_t)(((val_ui & 0x7FFu)) * LTC_FUEL_CELL_LSB_RESOLUTION_mV); /* Unit mV */
            } else {
                voltage = (int16_t)(((((~val_ui) + 1) & 0x7FF)) * (-LTC_FUEL_CELL_LSB_RESOLUTION_mV)); /* Unit mV */
            }

            if (ltc_state->ltcData.errorTable->PEC_valid[stringNumber][m] == true) {
                ltc_state->ltcData.cellVoltage->cellVoltage_mV[stringNumber]
                                                              [(ltc_state->ltcData.usedCellIndex[stringNumber]) +
                                                               (m * BS_NR_OF_CELLS_PER_MODULE)] = voltage;
                bitmask = ~bitmask; /* negate bitmask to only validate flags of this voltage register */
                ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][(m / LTC_NUMBER_OF_LTC_PER_MODULE)] &=
                    bitmask;
            } else {
                /* PEC_valid == false: Invalidate only flags of this voltage register */
                ltc_state->ltcData.cellVoltage->invalidCellVoltage[stringNumber][(m / LTC_NUMBER_OF_LTC_PER_MODULE)] |=
                    bitmask;
            }

            (ltc_state->ltcData.usedCellIndex[stringNumber])++;
            incrementations++;

            if ((ltc_state->ltcData.usedCellIndex[stringNumber]) > BS_NR_OF_CELLS_PER_MODULE) {
                break;
            }
        }

        /* Restore start value for next module in the daisy-chain. Only
         * decrement used cell index if current module is not the last
         * module in the daisy-chain. */
        if ((m + 1u) < LTC_N_LTC) {
            (ltc_state->ltcData.usedCellIndex[stringNumber]) -= incrementations;
        }
    }
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
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    uint8_t PEC_Check[6];
    uint16_t PEC_result = 0;

    /* now construct the message to be sent: it contains the wanted data, PLUS the needed PECs */
    pTxBuff[0] = ltc_cmdWRCFG[0];
    pTxBuff[1] = ltc_cmdWRCFG[1];
    pTxBuff[2] = ltc_cmdWRCFG[2];
    pTxBuff[3] = ltc_cmdWRCFG[3];

    /* set REFON bit to 1 */
    /* data for the configuration */
    for (uint16_t i = 0u; i < LTC_N_LTC; i++) {
        /* 3F = disable all pull-downs, 40: REFON = 1 */
        pTxBuff[4u + (i * 8u)] = 0x3F;
        pTxBuff[5u + (i * 8u)] = (LTC_HIRNG << 7u) | 0x40u;
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

        PEC_result              = LTC_pec15_calc(6, PEC_Check);
        pTxBuff[10u + (i * 8u)] = (PEC_result >> 8u) & 0xFFu;
        pTxBuff[11u + (i * 8u)] = PEC_result & 0xFFu;
    } /* end for */

    retVal = LTC_TransmitReceiveData(pSpiInterface, pTxBuff, pRxBuff, frameLength);

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
    uint16_t i = 0;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        for (i = 0; i < LTC_N_LTC; i++) {
            ltc_state->ltcData.errorTable->PEC_valid[stringNumber][i] = false;
            ltc_state->ltcData.errorTable->mux0[stringNumber][i]      = 0;
            ltc_state->ltcData.errorTable->mux1[stringNumber][i]      = 0;
            ltc_state->ltcData.errorTable->mux2[stringNumber][i]      = 0;
            ltc_state->ltcData.errorTable->mux3[stringNumber][i]      = 0;
        }
    }
}

/**
 * @brief   tells the LTC daisy-chain to start measuring the voltage on all cells.
 *
 * This function sends an instruction to the daisy-chain via SPI, in order to start voltage measurement for all cells.
 *
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   adcMode              LTC ADCmeasurement mode (fast, normal or filtered)
 * @param   adcMeasCh            number of cell voltage measured (2 cells or all cells)
 *
 * @return  retVal      #STD_OK if dummy byte was sent correctly by SPI, #STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_StartVoltageMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    LTC_ADCMEAS_CHAN_e adcMeasCh) {
    STD_RETURN_TYPE_e retVal = STD_OK;

    retVal = LTC_TransmitCommand(pSpiInterface, ltc_cmdADCV_normal_Fuelcell);

    return retVal;
}

/**
 * @brief   tells LTC daisy-chain to start measuring the voltage on GPIOS.
 *
 * This function sends an instruction to the daisy-chain via SPI to start the measurement.
 *
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   adcMode              LTC ADCmeasurement mode (fast, normal or filtered)
 * @param   PUP                  pull-up bit for pull-up or pull-down current (0: pull-down, 1: pull-up)
 *
 * @return  retVal      #STD_OK if command was sent correctly by SPI, #STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_StartOpenWireMeasurement(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    LTC_ADCMODE_e adcMode,
    uint8_t PUP) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;

    if (PUP == 0u) {
        /* pull-down current */
        retval = LTC_TransmitCommand(pSpiInterface, ltc_BC_cmdADOW_PDOWN_100ms_fuelcell);
    } else if (PUP == 1u) {
        /* pull-up current */
        retval = LTC_TransmitCommand(pSpiInterface, ltc_BC_cmdADOW_PUP_100ms_fuelcell);
    }

    return retval;
}

/**
 * @brief   checks if the data received from the daisy-chain is not corrupt.
 *
 * This function computes the PEC (CRC) from the data received by the daisy-chain.
 * It compares it with the PEC sent by the LTCs.
 * If there are errors, the array the error table is updated to locate the LTCs in daisy-chain
 * that transmitted corrupt data.
 *
 * @param   ltc_state                    state of the ltc state machine
 * @param   DataBufferSPI_RX_with_PEC    data obtained from the SPI transmission
 * @param  stringNumber                  string addressed
 *
 * @return  retVal                       STD_OK if PEC check is OK, STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_RX_PECCheck(
    LTC_STATE_s *ltc_state,
    uint16_t *DataBufferSPI_RX_with_PEC,
    uint8_t stringNumber) {
    uint16_t i               = 0;
    STD_RETURN_TYPE_e retVal = STD_OK;
    uint8_t PEC_TX[2];
    uint16_t PEC_result  = 0;
    uint8_t PEC_Check[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* check all PECs and put data without command and PEC in DataBufferSPI_RX (easier to use) */
    for (i = 0; i < LTC_N_LTC; i++) {
        PEC_Check[0] = DataBufferSPI_RX_with_PEC[4u + (i * 8u)];
        PEC_Check[1] = DataBufferSPI_RX_with_PEC[5u + (i * 8u)];
        PEC_Check[2] = DataBufferSPI_RX_with_PEC[6u + (i * 8u)];
        PEC_Check[3] = DataBufferSPI_RX_with_PEC[7u + (i * 8u)];
        PEC_Check[4] = DataBufferSPI_RX_with_PEC[8u + (i * 8u)];
        PEC_Check[5] = DataBufferSPI_RX_with_PEC[9u + (i * 8u)];

        PEC_result = LTC_pec15_calc(6, PEC_Check);
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
 * @brief   send command to the LTC daisy-chain and receives data from the LTC daisy-chain.
 *
 * This is the core function to receive data from the LTC6804 daisy-chain.
 * A 2 byte command is sent with the corresponding PEC. Example: read configuration register (RDCFG).
 * Only command has to be set, the function calculates the PEC automatically.
 * The data send is:
 * 2 bytes (COMMAND) 2 bytes (PEC)
 * The data received is:
 * 6 bytes (LTC1) 2 bytes (PEC) + 6 bytes (LTC2) 2 bytes (PEC) + 6 bytes (LTC3) 2 bytes (PEC) + ... + 6 bytes (LTC{LTC_N_LTC}) 2 bytes (PEC)
 *
 * The function does not check the PECs. This has to be done elsewhere.
 *
 * @param   Command              command sent to the daisy-chain
 * @param   pSpiInterface        pointer to SPI configuration
 * @param   pTxBuff              transmit buffer
 * @param   pRxBuff              receive buffer
 * @param   frameLength          number of words to transmit
 *
 * @return  statusSPI                   #STD_OK if SPI transmission is OK, #STD_NOT_OK otherwise
 *
 */
static STD_RETURN_TYPE_e LTC_RX(
    uint16_t *Command,
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint16_t *pTxBuff,
    uint16_t *pRxBuff,
    uint32_t frameLength) {
    STD_RETURN_TYPE_e statusSPI = STD_OK;
    uint16_t i                  = 0;

    /* DataBufferSPI_RX_with_PEC contains the data to receive.
       The transmission function checks the PECs.
       It constructs DataBufferSPI_RX, which contains the received data without PEC (easier to use). */

    for (i = 0; i < LTC_N_BYTES_FOR_DATA_TRANSMISSION; i++) {
        pTxBuff[i] = 0x00;
    }

    pTxBuff[0] = Command[0];
    pTxBuff[1] = Command[1];
    pTxBuff[2] = Command[2];
    pTxBuff[3] = Command[3];

    statusSPI = LTC_TransmitReceiveData(pSpiInterface, pTxBuff, pRxBuff, frameLength);

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
static uint32_t LTC_GetSPIClock(SPI_INTERFACE_CONFIG_s *pSpiInterface) {
    uint32_t SPI_Clock = 0;
    uint32_t prescaler = 0;

    /* if (LTC_SPI_INSTANCE == SPI2 || LTC_SPI_INSTANCE == SPI3) { */
    /* SPI2 and SPI3 are connected to APB1 (PCLK1) */
    /* The prescaler setup bits LTC_SPI_PRESCALER corresponds to the bits 5:3 in the SPI_CR1 register */
    /* Reference manual p.909 */
    /* The shift by 3 puts the bits 5:3 to the first position */
    /* Division are made by powers of 2 which corresponds to shifting to the right */
    /* Then 0 corresponds to divide by 2, 1 corresponds to divide by 4... so 1 has to be added to the value of the configuration bits */

    /* SPI_Clock = HAL_RCC_GetPCLK1Freq()>>((LTC_SPI_PRESCALER>>3)+1);
    } */

    /* if (LTC_SPI_INSTANCE == SPI1 || LTC_SPI_INSTANCE == SPI4 || LTC_SPI_INSTANCE == SPI5 || LTC_SPI_INSTANCE == SPI6) { */
    /* SPI1, SPI4, SPI5 and SPI6 are connected to APB2 (PCLK2) */
    /* The prescaler setup bits LTC_SPI_PRESCALER corresponds to the bits 5:3 in the SPI_CR1 register */
    /* Reference manual p.909 */
    /* The shift by 3 puts the bits 5:3 to the first position */
    /* Division are made by powers of 2 which corresponds to shifting to the right */
    /* Then 0 corresponds to divide by 2, 1 corresponds to divide by 4... so 1 has to be added to the value of the configuration bits */

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
    uint32_t transferTime_us = 0;
    uint32_t SPI_Clock       = 0;

    SPI_Clock = LTC_GetSPIClock(ltc_state->ltcData.pSpiInterface);

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
 * @param  statereq    state request to be checked
 *
 * @return              result of the state request that was made, taken from #LTC_RETURN_TYPE_e
 */
static LTC_RETURN_TYPE_e LTC_CheckStateRequest(LTC_STATE_s *ltc_state, LTC_REQUEST_s statereq) {
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
    bool retval = false;

    OS_EnterTaskCritical();
    retval = ltc_state->first_measurement_made;
    OS_ExitTaskCritical();

    return retval;
}

/**
 * @brief   sets the measurement initialization status.
 */
static void LTC_SetFirstMeasurementCycleFinished(LTC_STATE_s *ltc_state) {
    OS_EnterTaskCritical();
    ltc_state->first_measurement_made = true;
    OS_ExitTaskCritical();
}

extern void LTC_monitoringPinInit(void) {
    /* Set 3rd PE pins to enable daisy chains */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN10);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN11);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN12);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN13);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN14);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN15);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN16);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER3, PEX_PIN17);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN10);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN11);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN12);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN13);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN14);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN15);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN16);
    PEX_SetPin(PEX_PORT_EXPANDER3, PEX_PIN17);
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

TEST_LTC_DEFINE_GET(ltc_cmdWRCFG);
TEST_LTC_DEFINE_GET(ltc_cmdRDCFG);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVA_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVB_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVC_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVD_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVE_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVF_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVG_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVH_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdRDCVI_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_cmdADCV_normal_Fuelcell);
TEST_LTC_DEFINE_GET(ltc_BC_cmdADOW_PUP_100ms_fuelcell);
TEST_LTC_DEFINE_GET(ltc_BC_cmdADOW_PDOWN_100ms_fuelcell);
#endif
