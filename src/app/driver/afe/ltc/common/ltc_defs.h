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
 * @file    ltc_defs.h
 * @author  foxBMS Team
 * @date    2015-09-01 (date of creation)
 * @updated 2021-12-07 (date of last update)
 * @ingroup DRIVERS
 * @prefix  LTC
 *
 * @brief   Headers for the driver for the LTC monitoring chip.
 *
 */

#ifndef FOXBMS__LTC_DEFS_H_
#define FOXBMS__LTC_DEFS_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "diag_cfg.h"
#include "ltc_cfg.h"

#include "database.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/** error table for the LTC driver */
typedef struct {
    uint8_t PEC_valid[BS_NR_OF_STRINGS][LTC_N_LTC]; /*!<    */
    uint8_t mux0[BS_NR_OF_STRINGS][LTC_N_LTC];      /*!<    */
    uint8_t mux1[BS_NR_OF_STRINGS][LTC_N_LTC];      /*!<    */
    uint8_t mux2[BS_NR_OF_STRINGS][LTC_N_LTC];      /*!<    */
    uint8_t mux3[BS_NR_OF_STRINGS][LTC_N_LTC];      /*!<    */
} LTC_ERRORTABLE_s;

/** struct for storing information on the openwire detection */
typedef struct {
    int16_t openWirePup[BS_NR_OF_STRINGS][BS_NR_OF_BAT_CELLS];
    int16_t openWirePdown[BS_NR_OF_STRINGS][BS_NR_OF_BAT_CELLS];
    int32_t openWireDelta[BS_NR_OF_STRINGS][BS_NR_OF_BAT_CELLS];
} LTC_OPENWIRE_DETECTION_s;

/*========== Extern Constant and Variable Declarations ======================*/

/**
 * Voltage measurement mode
 * DCP0 means discharged not permitted during measurement
 * DCP1 means discharged permitted during measurement
 * Also DCP0 means balancing is interrupted when a cell is measured.
 */
typedef enum {
    LTC_ADCMODE_UNDEFINED,     /*!< ADC measurement mode is not defined        */
    LTC_ADCMODE_FAST_DCP0,     /*!< ADC measurement mode: Fast with DCP0       */
    LTC_ADCMODE_NORMAL_DCP0,   /*!< ADC measurement mode: Normal with DCP0     */
    LTC_ADCMODE_FILTERED_DCP0, /*!< ADC measurement mode: Filtered with DCP0   */
    LTC_ADCMODE_FAST_DCP1,     /*!< ADC measurement mode: Fast with DCP1       */
    LTC_ADCMODE_NORMAL_DCP1,   /*!< ADC measurement mode: Normal with DCP1     */
    LTC_ADCMODE_FILTERED_DCP1, /*!< ADC measurement mode: Filtered with DCP1   */
} LTC_ADCMODE_e;

/** Number of measured channels */
typedef enum {
    LTC_ADCMEAS_UNDEFINED,              /*!< not defined                            */
    LTC_ADCMEAS_ALLCHANNEL_CELLS,       /*!< all cell voltages are measured          */
    LTC_ADCMEAS_SINGLECHANNEL_TWOCELLS, /*!< only two cell voltages are measured */
    LTC_ADCMEAS_ALLCHANNEL_GPIOS,       /*!< all GPIO voltages are measured          */
    LTC_ADCMEAS_SINGLECHANNEL_GPIO1,    /*!< only a single ADC channel (GPIO1) is measured  */
    LTC_ADCMEAS_SINGLECHANNEL_GPIO2,    /*!< only a single ADC channel (GPIO2) is measured  */
    LTC_ADCMEAS_SINGLECHANNEL_GPIO3,    /*!< only a single ADC channel (GPIO3) is measured  */
    LTC_ADCMEAS_SINGLECHANNEL_GPIO4,    /*!< only a single ADC channel (GPIO4) is measured  */
    LTC_ADCMEAS_SINGLECHANNEL_GPIO5,    /*!< only a single ADC channel (GPIO5) is measured  */
    LTC_ADCMEAS_ALLCHANNEL_SC,          /*!< all ADC channels + sum of cells are measured   */
} LTC_ADCMEAS_CHAN_e;

/** States of the LTC state machine */
typedef enum {
    /* Init-Sequence */
    LTC_STATEMACH_UNINITIALIZED,  /*!<    */
    LTC_STATEMACH_INITIALIZATION, /*!<    */
    LTC_STATEMACH_REINIT,         /*!<    */
    LTC_STATEMACH_INITIALIZED,    /*!< LTC is initialized                             */
    /* Voltage Measurement Cycle */
    LTC_STATEMACH_IDLE,                    /*!<    */
    LTC_STATEMACH_STARTMEAS,               /*!<    */
    LTC_STATEMACH_READVOLTAGE,             /*!<    */
    LTC_STATEMACH_MUXMEASUREMENT,          /*!< Mux (Temperature and Balancing) Measurement    */
    LTC_STATEMACH_MUXMEASUREMENT_FINISHED, /*!<    */
    LTC_STATEMACH_BALANCECONTROL,          /*!<    */
    LTC_STATEMACH_ALLGPIOMEASUREMENT,      /*!<    */
    LTC_STATEMACH_READALLGPIO,             /*!<    */
    LTC_STATEMACH_READVOLTAGE_2CELLS,
    LTC_STATEMACH_STARTMEAS_2CELLS,
    LTC_STATEMACH_USER_IO_CONTROL,  /*!< Control of the user port expander              */
    LTC_STATEMACH_USER_IO_FEEDBACK, /*!< Control of the user port expander              */
    LTC_STATEMACH_EEPROM_READ,      /*!< Control of the external EEPROM                 */
    LTC_STATEMACH_EEPROM_WRITE,     /*!< Control of the external EEPROM                 */
    LTC_STATEMACH_TEMP_SENS_READ,   /*!< Control of the external temperature sensor     */
    LTC_STATEMACH_BALANCEFEEDBACK,
    LTC_STATEMACH_OPENWIRE_CHECK,
    LTC_STATEMACH_DEVICE_PARAMETER,
    LTC_STATEMACH_ADC_ACCURACY,
    LTC_STATEMACH_DIGITAL_FILTER,
    LTC_STATEMACH_VOLTMEAS_SUMOFCELLS,
    LTC_STATEMACH_EEPROM_READ_UID,     /*!< Control of the external EEPROM                 */
    LTC_STATEMACH_USER_IO_CONTROL_TI,  /*!< Control of the user port expander              */
    LTC_STATEMACH_USER_IO_FEEDBACK_TI, /*!< Control of the user port expander              */
    LTC_STATEMACH_STARTMEAS_CONTINUE,
    LTC_STATEMACH_MEASCYCLE_FINISHED,
    LTC_STATEMACH_UNDEFINED,            /*!< undefined state                                */
    LTC_STATEMACH_RESERVED1,            /*!< reserved state                                 */
    LTC_STATEMACH_ERROR_SPIFAILED,      /*!< Error-State: SPI error                         */
    LTC_STATEMACH_ERROR_PECFAILED,      /*!< Error-State: PEC error                         */
    LTC_STATEMACH_ERROR_MUXFAILED,      /*!< Error-State: multiplexer error                 */
    LTC_STATEMACH_ERROR_INITIALIZATION, /*!< Error-State: initialization error              */
} LTC_STATEMACH_e;

/** General substates */
typedef enum {
    LTC_ENTRY,           /*!< Substate entry state       */
    LTC_ERROR_ENTRY,     /*!< Substate entry error       */
    LTC_ERROR_PROCESSED, /*!< Substate error processed   */
} LTC_STATEMACH_SUB_e;

/** Substates for the uninitialized state */
typedef enum {
    LTC_ENTRY_UNINITIALIZED, /*!< Initialize-sequence */
} LTC_STATEMACH_UNINITIALIZED_SUB_e;

/** Substates for the initialization state */
typedef enum {
    /* Init-Sequence */
    LTC_INIT_STRING,                  /*!<    */
    LTC_ENTRY_INITIALIZATION,         /*!<    */
    LTC_START_INIT_INITIALIZATION,    /*!<    */
    LTC_RE_ENTRY_INITIALIZATION,      /*!<    */
    LTC_READ_INITIALIZATION_REGISTER, /*!<    */
    LTC_CHECK_INITIALIZATION,         /*!<    */
    LTC_EXIT_INITIALIZATION,          /*!<    */
} LTC_STATEMACH_INITIALIZATION_SUB_e;

/** Substates for the uninitialized state */
typedef enum {
    LTC_ENTRY_INITIALIZED, /*!<    */
} LTC_STATEMACH_INITIALIZED_SUB_e;

/** Substates for the read voltage state */
typedef enum {
    /* Init-Sequence */
    LTC_READ_VOLTAGE_REGISTER_A_RDCVA_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_B_RDCVB_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_C_RDCVC_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_D_RDCVD_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_E_RDCVE_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_F_RDCVF_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_G_RDCVG_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_H_RDCVH_READVOLTAGE, /*!<    */
    LTC_READ_VOLTAGE_REGISTER_I_RDCVI_READVOLTAGE, /*!<    */
    LTC_EXIT_READVOLTAGE,                          /*!<    */
    LTC_READ_AUXILIARY_REGISTER_A_RDAUXA,          /*!<    */
    LTC_READ_AUXILIARY_REGISTER_B_RDAUXB,          /*!<    */
    LTC_READ_AUXILIARY_REGISTER_C_RDAUXC,          /*!<    */
    LTC_READ_AUXILIARY_REGISTER_D_RDAUXD,          /*!<    */
    LTC_EXIT_READAUXILIARY_ALLGPIOS,               /*!<    */
} LTC_STATEMACH_READVOLTAGE_SUB_e;

/** Substates for the cell voltage + SC measurement state */
typedef enum {
    LTC_VOLTMEAS_SC_TRIGGER_CONVERSION,
    LTC_VOLTMEAS_SC_READ_SC,
    LTC_EXIT_VOLTMEAS_SC,
} LTC_STATEMACH_READVOLTAGE_SC_SUB_e;

/** Substates for the balance control state */
typedef enum {
    /* Init-Sequence */
    LTC_CONFIG_BALANCECONTROL,           /*!<    */
    LTC_CONFIG2_BALANCECONTROL,          /*!<    */
    LTC_CONFIG2_BALANCECONTROL_END,      /*!<    */
    LTC_REQUEST_FEEDBACK_BALANCECONTROL, /*!<    */
    LTC_READ_FEEDBACK_BALANCECONTROL,    /*!<    */
    LTC_SAVE_FEEDBACK_BALANCECONTROL,    /*!<    */
    LTC_EXIT_BALANCECONTROL,             /*!<    */
    LTC_STATEMACH_STARTMUXMEASUREMENT,
    LTC_STATEMACH_MUXCONFIGURATION_INIT, /*!<    */
    LTC_STATEMACH_MUXMEASUREMENT_CONFIG, /*!< Configuration of the multiplexers              */
    LTC_STATEMACH_READMUXMEASUREMENT,    /*!<    */
    LTC_STATEMACH_STOREMUXMEASUREMENT,   /*!<    */
} LTC_STATEMACH_BALANCECONTROL_SUB;

/** Substates for open-wire check */
typedef enum {
    LTC_REQUEST_PULLUP_CURRENT_OPENWIRE_CHECK,   /*!<    */
    LTC_READ_VOLTAGES_PULLUP_OPENWIRE_CHECK,     /*!<    */
    LTC_REQUEST_PULLDOWN_CURRENT_OPENWIRE_CHECK, /*!<    */
    LTC_READ_VOLTAGES_PULLDOWN_OPENWIRE_CHECK,   /*!<    */
    LTC_PERFORM_OPENWIRE_CHECK,
} LTC_STATEMACH_OPENWIRECHECK_SUB;

/** Substates for diagnosis state */
typedef enum {
    LTC_REQUEST_DEVICE_PARAMETER, /*!<    */
    LTC_DEVICE_PARAMETER_READ_STATUS_REGISTER_A,
    LTC_DEVICE_PARAMETER_READ_STATUS_REGISTER_B,
    LTC_REQUEST_REDUNDANCY_CHECK,
    LTC_REDUNDANCY_READ_STATUS_REGISTER_A,
    LTC_REDUNDANCY_READ_STATUS_REGISTER_B,
    LTC_EXIT_DEVICE_PARAMETER,
} LTC_STATEMACH_DIAGNOSIS_SUB_e;

/** Substates for ADC accuracy check */
typedef enum {
    LTC_REQUEST_ADC1_VERIFICATION,
    LTC_READ_ADC1_VERIFICATION_VALUE,
    LTC_SAVE_ADC1_VERIFICATION_VALUE,
    LTC_REQUEST_ADC2_VERIFICATION,
    LTC_READ_ADC2_VERIFICATION_VALUE,
    LTC_EXIT_ADC_ACCURACY_CHECK,
} LTC_STATEMACH_ADC_ACCURACY_CHECK_SUB_e;

/** Substates for digital filter check */
typedef enum {
    LTC_REQUEST_CLEAR_CELLVOLTAGES,
    LTC_REQUEST_CLEAR_AUX_REGISTER,
    LTC_REQUEST_CLEAR_STATUS_REGISTER,
    LTC_REQUEST_DIGITAL_FILTER_CHECK_CELLVOLTAGES,
    LTC_DIGITAL_FILTER_SELF_TEST_1_CELLVOLTAGES,
    LTC_DIGITAL_FILTER_SELF_TEST_1_AUX,
    LTC_DIGITAL_FILTER_READ_STATUS_REGISTER_A,
    LTC_DIGITAL_FILTER_READ_STATUS_REGISTER_B,
    LTC_DIGITAL_FILTER_SELF_TEST_1_STATUS,
    LTC_DIGITAL_FILTER_SELF_TEST_2_CELLVOLTAGES,
    LTC_DIGITAL_FILTER_SELF_TEST_2_AUX,
    LTC_DIGITAL_FILTER_READ_STATUS_REGISTER_A_2,
    LTC_DIGITAL_FILTER_READ_STATUS_REGISTER_B_2,
    LTC_DIGITAL_FILTER_SELF_TEST_2_STATUS,
    LTC_EXIT_DIGITAL_FILTER_CHECK,
} LTC_STATEMACH_DIGITAL_FILTER_SUB_e;

/** Substates for the user IO control state */
typedef enum {
    LTC_USER_IO_SET_OUTPUT_REGISTER,                 /*!<    */
    LTC_USER_IO_READ_INPUT_REGISTER,                 /*!<    */
    LTC_USER_IO_SEND_CLOCK_STCOMM,                   /*!<    */
    LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM, /*!<    */
    LTC_USER_IO_SAVE_DATA,                           /*!<    */
    LTC_USER_IO_FINISHED,                            /*!<    */
} LTC_STATEMACH_USER_IO_CONTROL_e;

/** Substates for the user IO control state, TI port expander */
typedef enum {
    LTC_USER_IO_SET_DIRECTION_REGISTER_TI,
    LTC_USER_IO_SET_OUTPUT_REGISTER_TI,                        /*!<    */
    LTC_USER_IO_READ_INPUT_REGISTER_TI_FIRST,                  /*!<    */
    LTC_USER_IO_READ_INPUT_REGISTER_TI_SECOND,                 /*!<    */
    LTC_USER_IO_SEND_CLOCK_STCOMM_TI,                          /*!<    */
    LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI,        /*!<    */
    LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_SECOND, /*!<    */
    LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_THIRD,  /*!<    */
    LTC_USER_IO_READ_I2C_TRANSMISSION_RESULT_RDCOMM_TI_FOURTH, /*!<    */
    LTC_USER_IO_SAVE_DATA_TI,                                  /*!<    */
    LTC_USER_IO_FINISHED_TI,                                   /*!<    */
} LTC_STATEMACH_USER_IO_CONTROL_TI_e;

/** Substates for the EEPROM control state */
typedef enum {
    LTC_EEPROM_SET_READ_ADDRESS,                    /*!<    */
    LTC_EEPROM_READ_DATA1,                          /*!<    */
    LTC_EEPROM_READ_DATA2,                          /*!<    */
    LTC_EEPROM_WRITE_DATA1,                         /*!<    */
    LTC_EEPROM_WRITE_DATA2,                         /*!<    */
    LTC_EEPROM_SEND_CLOCK_STCOMM1,                  /*!<    */
    LTC_EEPROM_SEND_CLOCK_STCOMM2,                  /*!<    */
    LTC_EEPROM_SEND_CLOCK_STCOMM3,                  /*!<    */
    LTC_EEPROM_SEND_CLOCK_STCOMM4,                  /*!<    */
    LTC_EEPROM_READ_I2C_TRANSMISSION_RESULT_RDCOMM, /*!<    */
    LTC_EEPROM_SAVE_READ,                           /*!<    */
    LTC_EEPROM_FINISHED,                            /*!<    */
} LTC_STATEMACH_EEPROM_READ_UID_e;

/** Substates for the temperature sensor control state */
typedef enum {
    LTC_TEMP_SENS_SEND_DATA1,                          /*!<    */
    LTC_TEMP_SENS_READ_DATA1,                          /*!<    */
    LTC_TEMP_SENS_READ_DATA3,                          /*!<    */
    LTC_TEMP_SENS_SEND_CLOCK_STCOMM1,                  /*!<    */
    LTC_TEMP_SENS_SEND_CLOCK_STCOMM2,                  /*!<    */
    LTC_TEMP_SENS_READ_I2C_TRANSMISSION_RESULT_RDCOMM, /*!<    */
    LTC_TEMP_SENS_SAVE_TEMP,                           /*!<    */
    LTC_TEMP_SENS_FINISHED,                            /*!<    */
} LTC_STATEMACH_TEMP_SENS_READ_e;

/** Substates for the multiplexer measurement configuration state */
typedef enum {
    /* Init-Sequence */
    LTC_SET_MUX_CHANNEL_WRCOMM_MUXMEASUREMENT_CONFIG, /*!<    */
    LTC_SEND_CLOCK_STCOMM_MUXMEASUREMENT_CONFIG,
    LTC_SEND_CLOCK_STCOMM_DIRECTION_CONFIG,                        /*!<    */
    LTC_SEND_CLOCK_STCOMM_WRITE_IO,                                /*!<    */
    LTC_READ_I2C_TRANSMISSION_RESULT_RDCOMM_MUXMEASUREMENT_CONFIG, /*!<    */
    LTC_READ_I2C_TRANSMISSION_CHECK_MUXMEASUREMENT_CONFIG,         /*!<    */
    LTC_START_GPIO_MEASUREMENT_MUXMEASUREMENT_CONFIG,              /*!<    */
} LTC_STATEMACH_MUXMEASUREMENT_CONFIG_SUB_e;

/** Substates for the all-GPIO multiplexer measurement state */
typedef enum {
    /* Init-Sequence */
    LTC_READ_AUXILIARY_REGISTER_A_RAUXA_READALLGPIO, /*!<    */
    LTC_READ_AUXILIARY_REGISTER_B_RAUXB_READALLGPIO, /*!<    */
    LTC_READ_AUXILIARY_REGISTER_C_RAUXC_READALLGPIO, /*!<    */
    LTC_READ_AUXILIARY_REGISTER_D_RAUXD_READALLGPIO, /*!<    */
    LTC_EXIT_READALLGPIO,                            /*!<    */
} LTC_STATEMACH_READALLGPIO_SUB_e;

/** Substates for the single GPIO multiplexer measurement state */
typedef enum {
    /* Init-Sequence */
    LTC_READ_AUXILIARY_REGISTER_A_RAUXA_MUXMEASUREMENT, /*!<    */
    LTC_SAVE_MUX_MEASUREMENT_MUXMEASUREMENT,            /*!<    */
} LTC_STATEMACH_MUXMEASUREMENT_SUB_e;

/** State requests for the LTC statemachine */
typedef enum {
    LTC_STATE_INIT_REQUEST,             /*!<    */
    LTC_STATE_USER_IO_WRITE_REQUEST,    /*!<    */
    LTC_STATE_USER_IO_READ_REQUEST,     /*!<    */
    LTC_STATE_USER_IO_REQUEST,          /*!<    */
    LTC_STATE_USER_IO_WRITE_REQUEST_TI, /*!<    */
    LTC_STATE_USER_IO_READ_REQUEST_TI,  /*!<    */
    LTC_STATE_EEPROM_READ_REQUEST,      /*!<    */
    LTC_STATE_EEPROM_WRITE_REQUEST,     /*!<    */
    LTC_STATE_EEPROM_READ_UID_REQUEST,  /*!<    */
    LTC_STATE_TEMP_SENS_READ_REQUEST,   /*!<    */
    LTC_STATE_BALANCEFEEDBACK_REQUEST,
    LTC_STATE_REINIT_REQUEST,                    /*!<    */
    LTC_STATE_IDLE_REQUEST,                      /*!<    */
    LTC_STATE_VOLTAGEMEASUREMENT_REQUEST,        /*!<    */
    LTC_STATE_VOLTAGEMEASUREMENT_2CELLS_REQUEST, /*!<    */
    LTC_STATE_VOLTAGEMEASUREMENT_SC_REQUEST,     /*!<    */
    LTC_STATE_READVOLTAGE_REQUEST,               /*!<    */
    LTC_STATE_READVOLTAGE_2CELLS_REQUEST,
    LTC_STATE_MUXMEASUREMENT_REQUEST, /*!<    */
    LTC_STATE_BALANCECONTROL_REQUEST, /*!<    */
    LTC_STATEMACH_BALANCEFEEDBACK_REQUEST,
    LTC_STATE_ALLGPIOMEASUREMENT_REQUEST, /*!<    */
    LTC_STATE_OPENWIRE_CHECK_REQUEST,
    LTC_STATEMACH_DEVICE_PARAMETER_REQUEST,
    LTC_STATEMACH_ADC_ACCURACY_REQUEST,
    LTC_STATEMACH_DIGITAL_FILTER_REQUEST,
    LTC_STATE_NO_REQUEST, /*!<    */
} LTC_STATE_REQUEST_e;

/**
 * Possible return values when state requests are made to the LTC statemachine
 */
typedef enum {
    LTC_OK,                  /*!< LTC --> ok                             */
    LTC_BUSY_OK,             /*!< LTC under load --> ok                  */
    LTC_REQUEST_PENDING,     /*!< requested to be executed               */
    LTC_ILLEGAL_REQUEST,     /*!< Request can not be executed            */
    LTC_SPI_ERROR,           /*!< Error state: Source: SPI               */
    LTC_PEC_ERROR,           /*!< Error state: Source: PEC               */
    LTC_MUX_ERROR,           /*!< Error state: Source: MUX               */
    LTC_INIT_ERROR,          /*!< Error state: Source: Initialization    */
    LTC_OK_FROM_ERROR,       /*!< Return from error --> ok               */
    LTC_ERROR,               /*!< General error state                    */
    LTC_ALREADY_INITIALIZED, /*!< Initialization of LTC already finished */
    LTC_ILLEGAL_TASK_TYPE,   /*!< Illegal                                */
} LTC_RETURN_TYPE_e;

/**
 * Variable to define if the LTC should measure are restart the initialization sequence
 */
typedef enum {
    LTC_HAS_TO_MEASURE,        /*!< measurement state of the LTC       */
    LTC_HAS_TO_REINIT,         /*!< re-initialization state of the LTC */
    LTC_HAS_TO_MEASURE_2CELLS, /*!< measurement state of the LTC    */
} LTC_TASK_TYPE_e;

/** configuration of the mux channels */
typedef struct {
    uint8_t muxID; /*!< multiplexer ID 0 - 3       */
    uint8_t muxCh; /*!< multiplexer channel 0 - 7   */
} LTC_MUX_CH_CFG_s;

/** struct that holds the multiplexer sequence */
typedef struct {
    uint8_t nr_of_steps;      /*!< number of steps in the multiplexer sequence   */
    LTC_MUX_CH_CFG_s *seqptr; /*!< pointer to the multiplexer sequence   */
} LTC_MUX_SEQUENCE_s;

/** This struct contains pointer to used data buffers */
typedef struct {
    SPI_INTERFACE_CONFIG_s *pSpiInterface;
    uint16_t *txBuffer;
    uint16_t *rxBuffer;
    uint32_t frameLength;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltage;
    DATA_BLOCK_CELL_TEMPERATURE_s *cellTemperature;
    DATA_BLOCK_BALANCING_FEEDBACK_s *balancingFeedback;
    DATA_BLOCK_BALANCING_CONTROL_s *balancingControl;
    DATA_BLOCK_SLAVE_CONTROL_s *slaveControl;
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *allGpioVoltages;
    DATA_BLOCK_OPEN_WIRE_s *openWire;
    uint16_t *usedCellIndex;
    LTC_OPENWIRE_DETECTION_s *openWireDetection;
    LTC_ERRORTABLE_s *errorTable;

    DATA_BLOCK_USER_MUX_s *user_mux;
    uint16_t *GPIOVoltages;   /* LTC2_NUMBER_OF_GPIOS * NR_OF_LTCs */
    uint16_t *valid_GPIOPECs; /* NR_OF_LTCs */

} LTC_DATAPTR_s;

/** This struct contains error counter and pointer to used error buffers */
typedef struct {
    uint32_t *errPECCnt; /* array length: Number of used LTCs */
    uint32_t *errSPICnt; /* array length: Number of used LTCs */
    uint8_t *ltcStatus;  /* array length: Number of used LTCs */
    uint8_t errPECRetryCnt;
    uint8_t errSPIRetryCnt;
    uint8_t errOccurred;
    uint32_t nrOfConsecutiveErrors;
} LTC_ERROR_s;

/**
 * This struct contains the measurement configuration for the LTC
 * Measurement is deactivated with value = LTC_STATE_NO_REQUEST
 */
typedef struct {
    LTC_STATE_REQUEST_e measVoltage;             /* activated = LTC_STATE_VOLTAGEMEASUREMENT_REQUEST */
    LTC_STATE_REQUEST_e measVoltage2Cells;       /* activated = LTC_STATE_VOLTAGEMEASUREMENT_2CELLS_REQUEST */
    LTC_STATE_REQUEST_e measVoltageSumofCells;   /* activated = LTC_STATE_VOLTAGEMEASUREMENT_SC_REQUEST */
    LTC_STATE_REQUEST_e measMux;                 /* activated = LTC_STATE_MUXMEASUREMENT_REQUEST */
    LTC_STATE_REQUEST_e balancing;               /* activated = LTC_STATE_BALANCECONTROL_REQUEST */
    LTC_STATE_REQUEST_e balancing_feedback;      /* activated = LTC_STATE_BALANCEFEEDBACK_REQUEST */
    LTC_STATE_REQUEST_e measAllGPIO;             /* activated = LTC_STATE_ALLGPIOMEASUREMENT_REQUEST */
    LTC_STATE_REQUEST_e userIO;                  /* activated = LTC_STATE_USER_IO_REQUEST */
    LTC_STATE_REQUEST_e readEEPROM;              /* activated = LTC_STATE_EEPROM_READ_UID_REQUEST */
    LTC_STATE_REQUEST_e measTemperature;         /* activated = LTC_STATE_TEMP_SENS_READ_REQUEST */
    LTC_STATE_REQUEST_e openWireCheck;           /* activated = LTC_STATE_OPENWIRE_CHECK_REQUEST */
    LTC_STATE_REQUEST_e deviceParameterCheck;    /* activated = LTC_STATEMACH_DEVICE_PARAMETER_REQUEST */
    LTC_STATE_REQUEST_e accuracyADCverification; /* activated = LTC_STATEMACH_ADC_ACCURACY_REQUEST */
    LTC_STATE_REQUEST_e digitalFilterCheck;      /* activated = LTC_STATEMACH_DIGITAL_FILTER_REQUEST */
    uint8_t taskCycleCnt;                        /* holds the current state machine index */
    uint8_t numberActiveOfStates;                /* number of active states */
    uint8_t activeStates[12]; /* array holds the different substates that are executed one after another */
                              /* maximum number of states : 12 */
} LTC_CONFIG_s;

/**
 *
 */
typedef enum {
    LTC_NOT_REUSED,
    LTC_REUSE_READVOLT_FOR_ADOW_PUP,
    LTC_REUSE_READVOLT_FOR_ADOW_PDOWN,
} LTC_REUSE_MODE_e;

/**
 * TI port expander IO direction (input or output).
 * Here the value in the enum is needed because it is written in
 * a configuration register.
 */
typedef enum {
    LTC_PORT_EXPANDER_TI_OUTPUT = 0x0,  /* specific value needed */
    LTC_PORT_EXPANDER_TI_INPUT  = 0xFF, /* specific value needed */
} LTC_PORT_EXPANDER_TI_DIRECTION_e;

/** This structure contains a state request and the addressed string. */
typedef struct {
    LTC_STATE_REQUEST_e request; /*!<    */
    uint8_t string;              /*!<    */
} LTC_REQUEST_s;

/**
 * This structure contains all the variables relevant for the LTC state machine.
 * The user can get the current state of the LTC state machine with this variable
 */
typedef struct {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms */
    LTC_TASK_TYPE_e taskMode;   /*!< current task of the state machine */
    LTC_REQUEST_s statereq;     /*!< current state request made to the state machine */
    LTC_STATEMACH_e state;      /*!< state of Driver State Machine */
    uint8_t substate;           /*!< current substate of the state machine */
    LTC_STATEMACH_e laststate;  /*!< previous state of the state machine */
    uint8_t lastsubstate;       /*!< previous substate of the state machine */
    LTC_ADCMODE_e adcMode;      /*!< current LTC ADCmeasurement mode (fast, normal or filtered) */
    LTC_ADCMODE_e voltMeasMode; /*!< current LTC ADCmeasurement mode (fast, normal or filtered) */
    LTC_ADCMODE_e gpioMeasMode; /*!< current LTC ADCmeasurement mode (fast, normal or filtered) */
    LTC_ADCMODE_e adcModereq;   /*!< requested LTC ADCmeasurement mode (fast, normal or filtered) */
    LTC_ADCMEAS_CHAN_e
        adcMeasCh; /*!< current number of channels measured for GPIOS (one at a time for multiplexers or all five GPIOs) */
    LTC_ADCMEAS_CHAN_e
        adcMeasChreq; /*!< requested number of channels measured for GPIOS (one at a time for multiplexers or all five GPIOs) */
    uint8_t
        numberOfMeasuredMux; /*!< number of multiplexer channels measured by the LTC chip before a voltage measurement is made */
    uint32_t ErrPECCounter; /*!< counts the number of times there was A PEC (CRC) error during communication with LTC */
    uint8_t
        ErrRetryCounter; /*!< counts how many times the drivers retried to communicate with LTC in case of a communication error */
    uint32_t ErrRequestCounter; /*!< counts the number of illegal requests to the LTC state machine */
    uint8_t triggerentry;       /*!< counter for re-entrance protection (function running flag) */
    uint32_t
        commandDataTransferTime; /*!< time needed for sending an instruction to the LTC, followed by data transfer from the LTC */
    uint32_t commandTransferTime; /*!< time needed for sending an instruction to the LTC */
    uint32_t
        gpioClocksTransferTime;  /*!< time needed for sending 72 clock signal to the LTC, used for I2C communication */
    uint32_t VoltageSampleTime;  /*!< time stamp at which the cell voltage were measured */
    uint32_t muxSampleTime;      /*!< time stamp at which a multiplexer input was measured */
    uint8_t instanceID;          /*!< number to distinguish between different ltc states, starting with 0,1,2,3....8 */
    uint8_t nrBatcellsPerModule; /*!< number of cells per module */
    uint8_t busSize;             /*!< number of connected LTCs to parallel bus network */
    LTC_ERROR_s errStatus;       /*!< contains pointer to local error buffer and error indicators */
    uint8_t *ltcIDs;             /*!< array with LTC IDs */
    uint8_t cntDeviceRD;         /*!< current Index of array ltcIDs to determine device ID */
    uint32_t ctrlCallCnt;        /*!< counts the LTC2_CTRL calls */
    uint8_t taskCycleCnt;        /*!< counts the current task cycle */
    LTC_REUSE_MODE_e
        reusageMeasurementMode; /*!< flag that indicates if currently any state is reused i.e. cell voltage measurement */
    LTC_CONFIG_s ltcConfig;     /*!< struct that holds the measurement configuration of the ltc network */
    bool first_measurement_made; /*!< flag that indicates if the first measurement cycle was completed */
    STD_RETURN_TYPE_e
        ltc_muxcycle_finished; /*!< flag that indicates if the measurement sequence of the multiplexers is finished  */
    STD_RETURN_TYPE_e check_spi_flag;       /*!< indicates if interrupt flag or timer must be considered */
    STD_RETURN_TYPE_e balance_control_done; /*!< indicates if balance control was done */
    uint8_t resendCommandCounter;           /*!< counter if commandy should be send multiple times e.g. ADOW command */
    bool transmit_ongoing;                  /*!< SPI transmissioncurrently ongoing */
    STD_RETURN_TYPE_e dummyByte_ongoing;    /*!< SPI dummy byte is currently transmitted */
    SPI_INTERFACE_CONFIG_s *spiSeqPtr;      /*!< pointer to the SPI sequence to be measured */
    SPI_INTERFACE_CONFIG_s *spiSeqEndPtr;   /*!< pointer to the end of the SPI sequence */
    uint8_t spiNumberInterfaces;            /*!< number of SPI channels that have to be measured */
    uint8_t currentString;                  /*!< string currently being addressed */
    uint8_t requestedString;                /*!< string addressed by the current state request */
    DIAG_ID_e spiDiagErrorEntry;            /*!< diagnosis entry for SPI related events */
    DIAG_ID_e pecDiagErrorEntry;            /*!< diagnosis entry for PEC related events */
    DIAG_ID_e muxDiagErrorEntry;            /*!< diagnosis entry for multiplexer related events */
    DIAG_ID_e voltMeasDiagErrorEntry;       /*!< diagnosis entry for voltage measurement related events */
    DIAG_ID_e tempMeasDiagErrorEntry;       /*!< diagnosis entry for temperature measurement related events */
    LTC_DATAPTR_s ltcData;                  /*!< contains pointers to the local data buffer */
    LTC_MUX_CH_CFG_s *muxmeas_seqptr
        [BS_NR_OF_STRINGS]; /*!< pointer to the multiplexer sequence to be measured (contains a list of elements [multiplexer id, multiplexer channels]) (1,-1)...(3,-1),(0,1),...(0,7) */
    LTC_MUX_CH_CFG_s *muxmeas_seqendptr
        [BS_NR_OF_STRINGS]; /*!< point to the end of the multiplexer sequence; pointer to ending point of sequence */
    uint8_t muxmeas_nr_end
        [BS_NR_OF_STRINGS]; /*!< number of multiplexer channels that have to be measured; end number of sequence, where measurement is finished*/
    uint8_t configuration[6]; /*!< holds the configuration of the ltc (configuration register) */

} LTC_STATE_s;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__LTC_DEFS_H_ */
