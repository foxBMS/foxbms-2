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
 * @file    bal_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  BAL
 *
 * @brief   Header for the configuration for the driver for balancing
 * @details TODO
 */

#ifndef FOXBMS__BAL_CFG_H_
#define FOXBMS__BAL_CFG_H_

/*========== Includes =======================================================*/

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** BAL state machine short time definition in 100*ms */
#define BAL_STATEMACH_SHORTTIME_100ms (1u)

/** BAL state machine long time definition in 100*ms */

#define BAL_STATEMACH_LONGTIME_100ms (50u)

/** BAL state machine balancing time in 100*ms */
#define BAL_STATEMACH_BALANCING_TIME_100ms (10u)

/** default value for the BAL voltage threshold */
#define BAL_DEFAULT_THRESHOLD_mV (200)

/** maximum value that BAL voltage threshold may take */
#define BAL_MAXIMUM_THRESHOLD_mV (5000)

/** minimum value that BAL voltage threshold may take */
#define BAL_MINIMUM_THRESHOLD_mV (0)

/** BAL hysteresis for voltage threshold when balancing was finished in mV */
#define BAL_HYSTERESIS_mV (200)

/** BAL lower voltage limit in MV */
#define BAL_LOWER_VOLTAGE_LIMIT_mV (2000)

/** BAL upper temperature limit in deci &deg;C */
#define BAL_UPPER_TEMPERATURE_LIMIT_ddegC (700)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   set balancing threshold
 * @param[in]   threshold_mV    threshold in mV
 */
extern void BAL_SetBalancingThreshold(int32_t threshold_mV);

/**
 * @brief   get balancing threshold
 * @return  balancing threshold in mV
 */
extern int32_t BAL_GetBalancingThreshold_mV(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BAL_CFG_H_ */
