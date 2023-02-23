/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    nxp_mc33775a_cfg.h
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  N775
 *
 * @brief   Header for the configuration for the MC33775A analog front-end.
 *
 */

#ifndef FOXBMS__NXP_MC33775A_CFG_H_
#define FOXBMS__NXP_MC33775A_CFG_H_

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"

#include "nxp_afe_dma.h"
#include "nxp_mc33775a_defs.h"
#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
#define N775_MAXIMUM_NUMBER_OF_SUPPORTED_CELL_MEASUREMENTS (14u)

#if BS_NR_OF_CELL_BLOCKS_PER_MODULE > N775_MAXIMUM_NUMBER_OF_SUPPORTED_CELL_MEASUREMENTS
#error "Number of cell blocks per module cannot be higher than maximum number of supported cells per IC"
#endif

/** Number of bytes to be sent over I2C to write and read I2C mux register in order to choose channel */
#define N775_I2C_NR_BYTES_FOR_MUX_WRITE (4u)

/** Number of bytes in I2C transaction after which the I2C bus should change to read*/
#define N775_I2C_NR_BYTES_TO_SWITCH_TO_READ_FOR_UX_READ (2u)

/**
 * Index used for FreeRTOS notification sent when TX DMA interrupt comes
 */
#define N775_NOTIFICATION_TX_INDEX (1u)
/**
 * Index used for FreeRTOS notification sent when RX DMA interrupt comes
 */
#define N775_NOTIFICATION_RX_INDEX (2u)
/**
 * Default notification value, signals that no notification was received
 */
#define N775_NO_NOTIFIED_VALUE (0x0u)
/**
 * Notification value sent when TX DMA interrupt comes
 */
#define N775_TX_NOTIFIED_VALUE (0x50u)
/**
 * Notification value sent when RX DMA interrupt comes
 */
#define N775_RX_NOTIFIED_VALUE (0x60u)
/**
 * Time in ms to wait for DMA TX notification, after this delay the transaction
 * is considered to have failed
 */
#define N775_NOTIFICATION_TX_TIMEOUT_ms (1u)
/**
 * Time in ms to wait for DMA RX notification, after this delay the transaction
 * is considered to have failed
 */
#define N775_NOTIFICATION_RX_TIMEOUT_ms (2u)

/**
 * Number of used N775-ICs
 */
#define N775_N_N775 (BS_NR_OF_MODULES_PER_STRING)

/**
 * Number of N775-ICs per battery module
 */
#define N775_NUMBER_OF_N775_PER_MODULE (1u)

/**
 * 775A Tx message length in bytes.
 */
#define N775_TX_MESSAGE_LENGTH (7u)

/**
 * 775A Rx message length in bytes.
 */
#define N775_RX_MESSAGE_LENGTH (4u)

/**
 * Timeout in milliseconds added to the transmission time for interrupt-based
 * SPI transmission.
 */
#define N775_TRANSMISSION_TIMEOUT (10u)

/**
 * N775 statemachine short time definition in ms
 */
#define N775_STATEMACH_SHORTTIME (1u)

/**
 * N775 Maximum time from wakeup event to possible communication in ms
 * 2.4ms in data sheet
 */
#define N775_TWAKECOM_MS (3u)

/**
 * N775 Time between two consecutive wake-up messages in ms
 * 4ms in data sheet
 */
#define N775_TDWAKE_MS (4u)

/**
 * N775 time for daisy-chain to wake-up in ms
 */
#define N775_TIME_DAISYCHAIN_WAKEUP_MS (N775_N_N775 * (N775_TWAKECOM_MS + N775_TDWAKE_MS))

/**
 * N775 time between two wake up commands in ms
 */
#define N775_TIME_BETWEEN_WAKEUP_MS (4u)

/**
 * N775 time after enumeraiton in ms
 */
#define N775_TIME_AFTER_ENUMERATION_MS (1u)

/**
 * Maximum number of re-tries in case of SPI error during the communication with daisy chain
 * before going into error state
 */
#define N775_TRANSMIT_SPIERRLIMIT (3u)

/**
 * Max number of Bytes to be received from daisy-chain
 * This limit is not due to the hardware.
 * It is only to set the length of the receive buffer for DMA.
 */
#define N775_MAX_N_BYTES_FOR_DATA_RECEPTION (N775_TX_MESSAGE_LENGTH * N775_MAX_NUMBER_OF_VOLTAGES)

/**
 * Transmit functions
 */
#define N775_SendData(txbuf, rxbuf, length) SPI_TransmitReceiveDataDma(&spi_nxp775InterfaceTx[0], txbuf, rxbuf, length)

/*========== Extern Constant and Variable Declarations ======================*/

/**
 * Multiplexer measurement sequence
 */
extern N775_MUX_CH_CFG_s n775_muxSequence[N775_MUX_SEQUENCE_LENGTH];

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
extern int16_t N775_ConvertVoltagesToTemperatures(uint16_t adcVoltage_mV);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC33775A_CFG_H_ */
