/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    adi_ades183x.h
 * @author  foxBMS Team
 * @date    2015-09-01 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Declarations for the driver of the ADI ADES18x family of
 *          analog front-ends.
 * @details Declares the high-level functions for the ADI ADES18x family driver
 *          The #ADI_ActivateInterfaceBoard function is specific to the
 *          hardware that communicates with the daisy-chain, i.e., for foxBMS 2
 *          it defines the setup of the port expander.
 */

#ifndef FOXBMS__ADI_ADES183X_H_
#define FOXBMS__ADI_ADES183X_H_

/*========== Includes =======================================================*/
/* clang-format off */
#include "adi_ades183x_cfg.h"
/* clang-format on */

#include "adi_ades183x_defs.h"
#include "afe.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* Value notified to the AFE task in the DMA callback called when
   the SPI transaction for the communication with the ADI AFE is
   finished */
#define ADI_DMA_SPI_FINISHED_NOTIFICATION_VALUE (0x50u)

/*========== Extern Constant and Variable Declarations ======================*/

/** Contains the internal state of the adi driver. */
extern ADI_STATE_s adi_stateBase;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Sets the pins to activate the interface board.
 */
extern void ADI_ActivateInterfaceBoard(void);

/**
 * @brief   Makes a request to the ADI driver.
 * @param   request request to be made with string addressed
 * @return  STD_OK if request queue was empty, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e ADI_MakeRequest(AFE_REQUEST_e request);

/**
 * @brief   Implements the actual measurement sequence for the ADI driver.
 * @details This function contains the sequence of events
 * @param   pAdiState state of the adi driver
 */
extern void ADI_MeasurementCycle(ADI_STATE_s *pAdiState);

/**
 * @brief   Gets the measurement initialization status.
 * @param   pAdiState state of the adi driver
 * @return  true if a first measurement cycle was made, false otherwise
 */
extern bool ADI_IsFirstMeasurementCycleFinished(ADI_STATE_s *pAdiState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_ADI_AccessToDatabase(ADI_STATE_s *pAdiState);
extern void TEST_ADI_BalanceControl(ADI_STATE_s *pAdiState);
extern STD_RETURN_TYPE_e TEST_ADI_GetRequest(AFE_REQUEST_e *request);
extern bool TEST_ADI_ProcessMeasurementNotStartedState(ADI_STATE_s *pAdiState, AFE_REQUEST_e *request);
extern void TEST_ADI_RunCurrentStringMeasurement(ADI_STATE_s *pAdiState);
extern void TEST_ADI_SetFirstMeasurementCycleFinished(ADI_STATE_s *pAdiState);
#endif

#endif /* FOXBMS__ADI_ADES183X_H_ */
