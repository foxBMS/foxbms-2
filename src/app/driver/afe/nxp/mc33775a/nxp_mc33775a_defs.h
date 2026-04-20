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
 * @file    nxp_mc33775a_defs.h
 * @author  foxBMS Team
 * @date    2020-09-01 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Definitions for the driver for the MC33775A analog front-end.
 * @details TODO
 */

#ifndef FOXBMS__NXP_MC33775A_DEFS_H_
#define FOXBMS__NXP_MC33775A_DEFS_H_

/*========== Includes =======================================================*/

#include "database.h"
#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/* TODO: Determine which defines are version-specific*/
/* Use of mux for temperature measurement */
#define N77X_USE_MUX_FOR_TEMP (SLV_USE_MUX_FOR_TEMP)
/* Read mux state after setting it */
#define N77X_CHECK_MUX_STATE (true)
/* Check supply current (part of MC33775A DIAG) */
#define N77X_CHECK_SUPPLY_CURRENT (false)
/* Broadcast address for the N775 slaves */
#define N77X_BROADCAST_ADDRESS (63u)

/** N775 Maximum wakeup to communication time in ms. Datasheet value: maximum 2.4 ms */
#define N77X_T_WAKE_COM_MS (3u)
/** N775 switching time from active to deep sleep mode. Datasheet value: maximum 0.3 ms */
#define N77X_T_SW_ACT_DEEP_SLEEP_MS (1u)
/** Waiting time between entering in cyclic mode and start of conversation.
 *  Datasheet value: minimum 5.0 ms */
#define N77X_T_WAIT_CYC_SOC_MS (6u)

/* Timeout in 10ms to go into sleep, max 255 (corresponding to 2550ms) */
#define N77X_TIMEOUT_TO_SLEEP_10MS (200u)
/* Timeout enable, 0u in register = enabled */
#define N77X_TIMEOUT_ENABLED (0u)
/* Timeout enable, 0x5A in register = disabled */
#define N77X_TIMEOUT_DISABLED (0x5Au)
/* Timeout enable switch*/
#define N77X_TIMEOUT_SWITCH (N77X_TIMEOUT_ENABLED)

/* Default chain address */
#define N77X_DEFAULT_CHAIN_ADDRESS (1u)
/* Measurement capture time */
/* + 1 to take balancing pause time before measurement into account */
#define N77X_MEASUREMENT_CAPTURE_TIME_MS (5u + 1u)
/* Time to wait after measurement capture for measurements to be ready */
#define N77X_MEASUREMENT_READY_TIME_MS (1u)
/* Value stored in N775A register when not valid */
#define N77X_INVALID_REGISTER_VALUE (0x8000u)
/* Invalid value converted to mV */
#define N77X_INVALID_mV_VALUE (3867)
/* Number of GPIO pins on one multiplexer */
#define N77X_MUX_GPIOS_PER_MUX (8u)
/* Length of the mux measurement sequence */
#if BS_NR_OF_TEMP_SENSORS_PER_MODULE <= N77X_MUX_GPIOS_PER_MUX
#define N77X_MUX_SEQUENCE_LENGTH (8u)
#else
#define N77X_MUX_SEQUENCE_LENGTH (18u)
#endif
/* Mux disable value */
#define N77X_MUX_DISABLE_VALUE (0xFFu)
/* Time in 10us for pause of balancing before measurement start */
/* Wait time after capture must be increased if this value is too high */
#define N77X_BAL_PAUSE_ENABLE_10us (100u)
/**
 * Upper bits of ADG728 mux address byte
 *  Set to 0: bit2, bit1 (address), bit0 (R/W)
 */
#define N77X_ADG728_ADDRESS_UPPER_BITS (0x98u)
/* I2C R/W bit, read */
#define N77X_I2C_READ (1u)
/* I2C R/W bit, write */
#define N77X_I2C_WRITE (0u)
/* Dummy byte for I2C, replaced by read data */
#define N77X_I2C_DUMMY_BYTE (0x0u)
/* GPIO position (0 to 7) used for multiplexed temperature measurement */
/* For the 775 Slave board, MUX0 is on GPIO0 and MUX1 on GPIO1 */
#define N77X_MULTIPLEXER_TEMP_GPIO_POSITION  (0u)
#define N77X_MULTIPLEXER_TEMP_GPIO1_POSITION (2u)
/* Global balancing timer (0x0 - 0xFFFF) */
#define N77X_GLOBAL_BALANCING_TIMER (0xFFFFu)
/* Pre-balancing timer (0x0 - 0xFFFF) */
#define N77X_PRE_BALANCING_TIMER (0x0u)
/* All channel balancing timer (0x0 - 0xFFFF) */
#define N77X_ALL_CHANNEL_BALANCING_TIMER (0xFFFFu)
/* Global balancing timer value to reach to reset it to max */
#define N77X_GLOBAL_BALANCING_TIMER_VALUE_SET_TO_MAX_AGAIN (0x1000u)
/* Maximum number of tries to avoid endless loop when waiting for a flag to be ready */
#define N77X_FLAG_READY_TRIES (5u)

/* Timeout in ms when waiting for an I2C transaction over NXP slave */
#define N77X_I2C_FINISHED_TIMEOUT_ms (50u)

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC33775A_DEFS_H_ */
