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
 * @file    ltc_6813-1_cfg.h
 * @author  foxBMS Team
 * @date    2015-02-18 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  LTC
 *
 * @brief   Header for the configuration for the LTC 6804-1 6811-1, 6812-1,
 *          and 6813-1 monitoring IC.
 * @details TODO
 */

#ifndef FOXBMS__LTC_6813_1_CFG_H_
#define FOXBMS__LTC_6813_1_CFG_H_

/*========== Includes =======================================================*/
/* clang-format off */
#include "ltc_cfg.h"
/* clang-format on */

#include "ltc_defs.h"
#include "battery_system_cfg.h"

#include "ltc_afe_dma.h"
#include "spi.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/**
 * If set to 1 LTC driver is configured to use foxBMS slave boards version 1.x
 * If set to 2 LTC driver is configured to use foxBMS slave boards version 2.x
 */
#define SLAVE_BOARD_VERSION (2)

#if SLAVE_BOARD_VERSION == 1
#elif SLAVE_BOARD_VERSION == 2
#else
#error Please select the slave board version you want to use. Configuration file: \src\module\config\ltc_cfg.h
#endif

/**
 * @def     LTC_6813_MAX_SUPPORTED_CELLS
 * @brief   Defines the maximal number of supported cells per module
 */
#define LTC_6813_MAX_SUPPORTED_CELLS (18u)

#if BS_NR_OF_CELL_BLOCKS_PER_MODULE > LTC_6813_MAX_SUPPORTED_CELLS
#error "Number of cell blocks per module cannot be higher than maximum number of cells per module"
#endif

/**
 * If set to 0 LTC driver is configured to use PCA8574 port expander
 * If set to 1 LTC driver is configured to use TCA6408A port expander
 */
#define LTC_PORTEXPANDER_VERSION (1u)

/** Address of TI port expander (0 or 1) */
#define LTC_PORTEXPANDER_ADR_TI (0u)

/** Controls if PEC should be discarded (true) or not (false) */
#define LTC_DISCARD_PEC (false)

/* set to true or false  */
#define LTC_GOTO_MUX_CHECK (true)

/* set to true or false */
#define LTC_DISCARD_MUX_CHECK (false)

/** Number of multiplexer used per LTC-IC */
#define LTC_N_MUX_PER_LTC (3u)

/** Number of channels per multiplexer */
#define LTC_N_MUX_CHANNELS_PER_MUX (8u)

/** Number of multiplexer measurements per LTC cycle */
#define LTC_NUMBER_OF_MUX_MEASUREMENTS_PER_CYCLE (8u)

/** Number of multiplexed channels per LTC-IC */
#define LTC_N_MUX_CHANNELS_PER_LTC (LTC_N_MUX_PER_LTC * LTC_N_MUX_CHANNELS_PER_MUX)

/** Number of LTC-ICs per battery module */
#define LTC_NUMBER_OF_LTC_PER_MODULE (1u)

/** Number of LTC cell voltages to be read per register */
#define LTC_NUMBER_OF_CELL_VOLTAGES_PER_REGISTER (3u)

/** Open-wire detection threshold */
#define LTC_ADOW_THRESHOLD (-400)

/**
 * Measurement mode for voltages, possible values:
 * - LTC_ADCMODE_NORMAL_DCP0
 * - LTC_ADCMODE_FILTERED_DCP0
 * - LTC_ADCMODE_FAST_DCP0
 */
#define LTC_VOLTAGE_MEASUREMENT_MODE (LTC_ADCMODE_NORMAL_DCP0)

/**
 * Measurement mode for GPIOs, possible values:
 * - LTC_ADCMODE_NORMAL_DCP0
 * - LTC_ADCMODE_FILTERED_DCP0
 * - LTC_ADCMODE_FAST_DCP0
 */
#define LTC_GPIO_MEASUREMENT_MODE (LTC_ADCMODE_NORMAL_DCP0)

/**
 * Measurement mode for Open-wire check, possible values:
 * - LTC_ADCMODE_NORMAL_DCP0
 * - LTC_ADCMODE_FILTERED_DCP0
 */
#define LTC_OW_MEASUREMENT_MODE (LTC_ADCMODE_NORMAL_DCP0)

/**
 * Timeout in milliseconds added to the transmission time for interrupt-based
 * SPI transmission.
 */
#define LTC_TRANSMISSION_TIMEOUT (10)

/**
 * SPI1 is used for communication with LTC
 * @{
 */
#define LTC_SPI_HANDLE    (&spi_devices[0])
#define LTC_SPI_INSTANCE  (*LTC_SPI_HANDLE.Instance)
#define LTC_SPI_PRESCALER (*LTC_SPI_HANDLE.Init.BaudRatePrescaler)
/**@}*/

/** start definition of LTC timings; Twake (see LTC data sheet) */
#define LTC_TWAKE_US (300)
/** start definition of LTC timings; Tready (see LTC data sheet) */
#define LTC_TREADY_US (10)
/** start definition of LTC timings; Tidle (see LTC data sheet) */
#define LTC_TIDLE_US (6700)

/** LTC SPI wakeup time */
#define LTC_SPI_WAKEUP_WAIT_TIME_US (30u)

/** LTC state machine short time definition in ms */
#define LTC_STATEMACH_SHORTTIME (1)

/**
 * time for the first initialization of the daisy chain
 * see LTC6804 data sheet page 41
 */
#define LTC_STATEMACH_DAISY_CHAIN_FIRST_INITIALIZATION_TIME ((LTC_TWAKE_US * LTC_N_LTC) / 1000)
/**
 * time for the second initialization of the daisy chain
 * see LTC6804 data sheet page 41
 */
#define LTC_STATEMACH_DAISY_CHAIN_SECOND_INITIALIZATION_TIME ((LTC_TREADY_US * LTC_N_LTC) / 1000)

/*
 * Timings of Voltage Cell and GPIO measurement for all cells or all GPIO
 */

/**
 * ~1.1ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Fast Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_ALL_CELLS_FAST_TCYCLE (2)

/**
 * ~2.3ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Normal Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_ALL_CELLS_NORMAL_TCYCLE (3)

/**
 * ~201ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Filtered Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_ALL_CELLS_FILTERED_TCYCLE (202)

/**
 * ~203us Measurement+Calibration Cycle Time When Starting from the REFUP State in Fast Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_TWO_CELLS_FAST_TCYCLE (1)

/**
 * ~407us Measurement+Calibration Cycle Time When Starting from the REFUP State in Normal Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_TWO_CELLS_NORMAL_TCYCLE (1)

/**
 * ~34ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Filtered Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_TWO_CELLS_FILTERED_TCYCLE (35)

/*
 * Timings of Voltage Cell and GPIO measurement for a pair of cells or a single GPIO
 */

/**
 * ~1.8ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Fast Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_ALL_GPIOS_FAST_TCYCLE (2)

/**
 * ~3.9ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Normal Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_ALL_GPIOS_NORMAL_TCYCLE (4)

/**
 * ~335ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Filtered Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_ALL_GPIOS_FILTERED_TCYCLE (336)

/**
 *  ~380us Measurement+Calibration Cycle Time When Starting from the REFUP State in Fast Mode
 *  unit: ms
 */
#define LTC_STATEMACH_MEAS_SINGLE_GPIO_FAST_TCYCLE (1)

/**
 * ~788us Measurement+Calibration Cycle Time When Starting from the REFUP State in Normal Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_SINGLE_GPIO_NORMAL_TCYCLE (1)

/**
 * ~67.1ms Measurement+Calibration Cycle Time When Starting from the REFUP State in Filtered Mode
 * unit: ms
 */
#define LTC_STATEMACH_MEAS_SINGLE_GPIO_FILTERED_TCYCLE (68)

/** LTC state machine sequence error timing in ms */
#define LTC_STATEMACH_SEQ_ERR_TIME (5)
/** LTC state machine CRC-transmission error timing in ms */
#define LTC_STATEMACH_PEC_ERR_TIME (1)

/**
 * Maximum number of re-tries in case of CRC error during the communication with daisy chain
 * before going into error state
 */
#define LTC_TRANSMIT_PEC_ERR_LIMIT (10)

/** If set to 1, check if multiplexers acknowledged transmission */
#define LTC_READCOM (0)

/**
 * Number of required ADOW commands because of external C-Pin capacitance and
 * the respective duration to perform an open wire check for 14 modules with
 * 12 cells each. During this time no cell voltages and temperatures are measured!
 * +----------------+--------------+---------------+----------+----------+
 * | External C pin | Normal  mode | Filtered mode | Duration | Duration |
 * | capacitance    |              |               |  normal  | filtered |
 * | ---------------+--------------+---------------+----------+----------+
 * |   <= 10nF      |      2       |        2      |    32ms  |   828ms  |
 * |     100nF      |     10       |        2      |   112ms  |   828ms  |
 * |       1uF      |    100       |        2      |  1012ms  |   828ms  |
 * |       C        | 1.5+(C/10nF) |        2      |          |          |
 * +----------------+--------------+---------------+----------+----------+
 */
#define LTC_NUMBER_REQ_ADOW_COMMANDS (2)

/**
 * Transmit functions
 * @{
 */
#define LTC_TRANSMIT_WAKE_UP(spi_ltcInterface) SPI_TransmitDummyByte(spi_ltcInterface, LTC_SPI_WAKEUP_WAIT_TIME_US)
#define LTC_TRANSMIT_I2C_COMMAND(spi_ltcInterface, txbuf)                 \
    SPI_TransmitDummyByte(spi_ltcInterface, LTC_SPI_WAKEUP_WAIT_TIME_US); \
    SPI_TransmitData(spi_ltcInterface, txbuf, 4 + 9)
#define LTC_TRANSMIT_COMMAND(spi_ltcInterface, command)                   \
    SPI_TransmitDummyByte(spi_ltcInterface, LTC_SPI_WAKEUP_WAIT_TIME_US); \
    SPI_TransmitData(spi_ltcInterface, command, 4)
#define LTC_TRANSMIT_RECEIVE_DATA(spi_ltcInterface, txbuf, rxbuf, length) \
    SPI_TransmitDummyByte(spi_ltcInterface, LTC_SPI_WAKEUP_WAIT_TIME_US); \
    SPI_TransmitReceiveDataDma(spi_ltcInterface, txbuf, rxbuf, length)
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/

/**
 * Definition of the multiplexer measurement sequence
 */
extern LTC_MUX_SEQUENCE_s ltc_mux_seq;

/**
 * On the foxBMS slave board there are 6 multiplexer inputs dedicated to temperature
 * sensors by default.
 * Lookup table between temperature sensors and battery cells
 */
extern const uint8_t ltc_muxSensorTemperature_cfg[BS_NR_OF_TEMP_SENSORS_PER_MODULE];

/** Lookup table to indicate which voltage inputs are used */
extern const uint8_t ltc_voltage_input_used[LTC_6813_MAX_SUPPORTED_CELLS];

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   converts a raw voltage from multiplexer to a temperature value in
 *          deci &deg;C.
 * @details The temperatures are read from NTC elements via voltage dividers.
 *          This function implements the look-up table between voltage and
 *          temperature, taking into account the NTC characteristics and the
 *          voltage divider.
 * @param   adcVoltage_mV   voltage read from the multiplexer in mV
 * @return  temperature value in deci &deg;C
 */
extern int16_t LTC_ConvertMuxVoltagesToTemperatures(uint16_t adcVoltage_mV);

/**
 * @brief   Get connected cell voltage input from an cell block index.
 * @details This function returns the AFE cell voltage input from cell block
 *          index.
 * @param   indexCellBlock   cell block index
 * @return  AFE voltage input where this cell block is connected to
 */
extern uint8_t LTC_GetVoltageInputIndexFromCellBlockIndex(uint8_t indexCellBlock);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__LTC_6813_1_CFG_H_ */
