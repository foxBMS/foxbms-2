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
 * @file    state_estimation.h
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup APPLICATION
 * @prefix  SE
 *
 * @brief   Header for state-estimation module responsible for the estimation
 *          of state-of-charge (SOC), state-of-energy (SOE) and state-of-health
 *          (SOH). Functions as a wrapper for the individual state-estimation
 *          algorithms.
 * @details TODO
 */

#ifndef FOXBMS__STATE_ESTIMATION_H_
#define FOXBMS__STATE_ESTIMATION_H_

/*========== Includes =======================================================*/

#include "database.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Wrapper for algorithm specific SOC initialization
 * @param[in]   ccPresent    true if current sensor present, false otherwise
 * @param[in]   stringNumber string addressed
 */
extern void SE_InitializeSoc(bool ccPresent, uint8_t stringNumber);

/**
 * @brief   Wrapper for algorithm specific SOE initialization
 * @param[in]   ec_present   true if current sensor present, false otherwise
 * @param[in]   stringNumber string addressed
 */
extern void SE_InitializeSoe(bool ec_present, uint8_t stringNumber);

/**
 * @brief   Wrapper for algorithm specific SOH initialization
 * @param[in]   stringNumber    string addressed
 */
extern void SE_InitializeSoh(uint8_t stringNumber);

/**
 * @brief   Main function to perform state estimations
 */
extern void SE_RunStateEstimations(void);

/** \defgroup state-estimation-api State Estimation API
 *  @details    This API is implemented by the state estimation functions and
 *              called by the state estimation module.
 * @{
 * INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE se-state-estimation-api-start-include */
/**
 * @brief   initializes startup SOC-related values like lookup from nonvolatile
 *          ram at startup
 * @param[out]  pSocValues   pointer to SOC database entry
 * @param[in]   ccPresent    true if current sensor present, false otherwise
 * @param[in]   stringNumber string addressed
 */
extern void SE_InitializeStateOfCharge(DATA_BLOCK_SOC_s *pSocValues, bool ccPresent, uint8_t stringNumber);

/**
 * @brief   periodically called algorithm to calculate state-of-charge (SOC)
 * @param[out] pSocValues pointer to SOC values
 */
extern void SE_CalculateStateOfCharge(DATA_BLOCK_SOC_s *pSocValues);

/**
 * @brief   look-up table for SOC initialization
 * @param[in]   voltage_mV    voltage in mV of battery cell
 * @return  returns SOC value in percentage from 0.0% to 100.0%
 */
extern float_t SE_GetStateOfChargeFromVoltage(int16_t voltage_mV);

/**
 * @brief   initializes startup state-of-energy (SOE) related values
 * @param[out]  pSoeValues     pointer to SOE database entry
 * @param[in]   ec_present     true if current sensor EC message received, false otherwise
 * @param[in]   stringNumber   string addressed
 */
extern void SE_InitializeStateOfEnergy(DATA_BLOCK_SOE_s *pSoeValues, bool ec_present, uint8_t stringNumber);

/**
 * @brief   periodically called algorithm to calculate state-of-energy (SOE)
 * @param[out] pSoeValues pointer to SOE database entry
 */
extern void SE_CalculateStateOfEnergy(DATA_BLOCK_SOE_s *pSoeValues);

/**
 * @brief   initializes startup state-of-health related values
 * @param[out]  pSohValues     pointer to SOH database entry
 * @param[in]   stringNumber   string addressed
 */
extern void SE_InitializeStateOfHealth(DATA_BLOCK_SOH_s *pSohValues, uint8_t stringNumber);

/**
 * @brief   calculates state-of-health (SOH)
 * @param[out] pSohValues pointer to SOH database entry
 */
extern void SE_CalculateStateOfHealth(DATA_BLOCK_SOH_s *pSohValues);

/** INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE se-state-estimation-api-stop-include
 * @}
 */

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__STATE_ESTIMATION_H_ */
