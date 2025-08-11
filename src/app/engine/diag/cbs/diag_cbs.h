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
 * @file    diag_cbs.h
 * @author  foxBMS Team
 * @date    2021-02-17 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup ENGINE
 * @prefix  DIAG
 *
 * @brief   Diagnosis driver header
 * @details TODO
 */

#ifndef FOXBMS__DIAG_CBS_H_
#define FOXBMS__DIAG_CBS_H_

/*========== Includes =======================================================*/

#include "diag_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief dummy callback function of diagnosis events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          data
 */
extern void DIAG_DummyCallback(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for overvoltage events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where overvoltage event occurred
 */
extern void DIAG_ErrorOvervoltage(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for undervoltage events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where undervoltage event occurred
 */
extern void DIAG_ErrorUndervoltage(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);
/**
 * @brief diagnosis callback function for overtemperature charge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where overtemperature event occurred
 */
extern void DIAG_ErrorOvertemperatureCharge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for overtemperature discharge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where overtemperature event occurred
 */
extern void DIAG_ErrorOvertemperatureDischarge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for undertemperature charge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where undertemperature event occurred
 */
extern void DIAG_ErrorUndertemperatureCharge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for undertemperature discharge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where undertemperature event occurred
 */
extern void DIAG_ErrorUndertemperatureDischarge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for overcurrent charge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where overcurrent event occurred
 */
extern void DIAG_ErrorOvercurrentCharge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for overcurrent discharge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where overcurrent event occurred
 */
extern void DIAG_ErrorOvercurrentDischarge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for overcurrent charge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where overcurrent event occurred
 */
extern void DIAG_ErrorCurrentOnOpenString(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for current measurement related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where event occurred
 */
extern void DIAG_ErrorCurrentMeasurement(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for current sensor related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where event occurred
 */
extern void DIAG_ErrorHighVoltageMeasurement(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/* -----------------  Functions for system related events ------------------ */

/**
 * @brief Callback function for system monitoring related diagnosis events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          data
 */
extern void DIAG_ErrorSystemMonitoring(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief Callback function for interlock related diagnosis events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim  shim to the database entries
 * @param[in] data          data
 */
extern void DIAG_ErrorInterlock(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for CAN related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data              data
 */
extern void DIAG_ErrorCanTiming(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for CAN Rx related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          data
 */
extern void DIAG_ErrorCanRxQueueFull(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for CAN Tx related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          data
 */

extern void DIAG_ErrorCanTxQueueFull(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for AFE module related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where AFE event occurred
 */
extern void DIAG_ErrorAfeDriver(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for AFE related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where event occurred
 */
extern void DIAG_ErrorAfe(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for current sensor related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where current sensor event occurred
 */
extern void DIAG_ErrorCurrentSensor(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for SBC related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          data
 */
extern void DIAG_Sbc(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for plausibility check related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where current sensor event occurred
 */
extern void DIAG_ErrorPlausibility(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for string contactor feedback related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where contactor feedback event occurred
 */
extern void DIAG_StringContactorFeedback(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for precharge contactor feedback related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where precharge feedback event occurred
 */
extern void DIAG_PrechargeContactorFeedback(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for plausibility check related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         #DIAG_EVENT_e
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where precharge feedback event occurred
 */
extern void DIAG_PlausibilityCheck(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for deep discharge events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where deep discharge event occurred
 */
extern void DIAG_ErrorDeepDischarge(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for current sensor related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  TODO
 */
extern void DIAG_ErrorPowerMeasurement(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for the insulation measurement
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  TODO
 */
extern void DIAG_Insulation(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for I2C port expander related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          not relevant
 */
extern void DIAG_I2c(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for FRAM related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          not relevant
 */
extern void DIAG_FramError(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for RTC related events
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          not relevant
 */
extern void DIAG_Rtc(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for ALERT flag
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          not relevant
 */
extern void DIAG_AlertFlag(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for precharge abort reasons
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] stringNumber  stringNumber where precharging is performed
 */
extern void DIAG_PrechargeProcess(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t stringNumber);

/**
 * @brief diagnosis callback function for aerosol alert
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          not relevant
 */
extern void DIAG_AerosolAlert(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/**
 * @brief diagnosis callback function for clamp 30C
 * @param[in] diagId        ID of diag entry
 * @param[in] event         OK, NOK or RESET
 * @param[in] kpkDiagShim   shim to the database entries
 * @param[in] data          not relevant
 */
extern void DIAG_SupplyVoltageClamp30c(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__DIAG_CBS_H_ */
