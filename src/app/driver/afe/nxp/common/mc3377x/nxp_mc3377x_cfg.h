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
 * @file    nxp_mc3377x_cfg.h
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  N77X
 *
 * @brief   Header for the configuration for the MC3377X analog front-end.
 * @details TODO
 */

#ifndef FOXBMS__NXP_MC3377X_CFG_H_
#define FOXBMS__NXP_MC3377X_CFG_H_

/*========== Includes =======================================================*/
#include "battery_system_cfg.h"

#if defined(FOXBMS_AFE_DRIVER_NXP_MC33775A)
#include "nxp_mc33775a_cfg.h"
#endif

#include "nxp_mc3377x_defs.h"
#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Number of bytes to be sent over I2C to write and read I2C mux register in order to choose channel */
#define N77X_I2C_NR_BYTES_FOR_MUX_WRITE (4u)

/** Number of bytes in I2C transaction after which the I2C bus should change to read*/
#define N77X_I2C_NR_BYTES_TO_SWITCH_TO_READ_FOR_UX_READ (2u)

/**
 * Index used for FreeRTOS notification sent when TX DMA interrupt comes
 */
#define N77X_NOTIFICATION_TX_INDEX (1u)
/**
 * Index used for FreeRTOS notification sent when RX DMA interrupt comes
 */
#define N77X_NOTIFICATION_RX_INDEX (2u)
/**
 * Default notification value, signals that no notification was received
 */
#define N77X_NO_NOTIFIED_VALUE (0x0u)
/**
 * Notification value sent when TX DMA interrupt comes
 */
#define N77X_TX_NOTIFIED_VALUE (0x50u)
/**
 * Notification value sent when RX DMA interrupt comes
 */
#define N77X_RX_NOTIFIED_VALUE (0x60u)
/**
 * Time in ms to wait for DMA TX notification, after this delay the transaction
 * is considered to have failed
 */
#define N77X_NOTIFICATION_TX_TIMEOUT_ms (1u)
/**
 * Time in ms to wait for DMA RX notification, after this delay the transaction
 * is considered to have failed
 */
#define N77X_NOTIFICATION_RX_TIMEOUT_ms (2u)

/**
 * N77X Tx message length in bytes.
 */
#define N77X_TX_MESSAGE_LENGTH (7u)

/**
 * N77X Rx message length in bytes.
 */
#define N77X_RX_MESSAGE_LENGTH (4u)

/*========== Extern Constant and Variable Declarations ======================*/

/** Multiplexer measurement sequence */
extern N77X_MUX_CH_CFG_s n77x_muxSequence[N77X_MUX_SEQUENCE_LENGTH];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC3377X_CFG_H_ */
