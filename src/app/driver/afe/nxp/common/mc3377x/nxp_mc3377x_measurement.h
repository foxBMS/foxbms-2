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
 * @file    nxp_mc3377x_measurement.h
 * @author  foxBMS Team
 * @date    2025-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Header file of nxp_mc3377x_measurement.c
 * @details TODO
 */

#ifndef FOXBMS__NXP_MC3377X_MEASUREMENT_H_
#define FOXBMS__NXP_MC3377X_MEASUREMENT_H_

/*========== Includes =======================================================*/
#include "nxp_mc3377x_cfg.h"

#include "nxp_mc3377x-ll.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   captures the measurement.
 * @details The MC3377X measures continuously.
 *          This function takes a snapshot on all slaves in the daisy-chain.
 * @param   pState state of the NXP MC3377X driver
 */
extern void N77x_CaptureMeasurement(N77X_STATE_s *pState);

/**
 * @brief   starts the measurement.
 * @details The MC3377X measures continuously.
 *          This function starts the measurement.
 * @param  pState  state of the NXP MC3377X driver
 */
extern void N77x_StartMeasurement(N77X_STATE_s *pState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_N77x_InitMeasurement(N77X_STATE_s *pState);

#if defined(FOXBMS_AFE_DRIVER_NXP_MC33775A)
extern void TEST_N77x_RetrieveVoltages(N77X_STATE_s *pState, uint8_t m, uint16_t *primaryRawValues, bool *gpio03Error);
extern void TEST_N77x_RetrieveTemperatures(N77X_STATE_s *pState, uint8_t m, bool *gpio03Error, bool *gpio47Error);
extern void TEST_N77x_RetrieveMeasurement(N77X_STATE_s *pState);
#endif
#endif

#endif /* FOXBMS__NXP_MC3377X_MEASUREMENT_H_ */
