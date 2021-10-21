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
 * @file    sps.h
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2021-10-01 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SPS
 *
 * @brief   Headers for the driver for the smart power switches.
 *
 */

#ifndef FOXBMS__SPS_H_
#define FOXBMS__SPS_H_

/*========== Includes =======================================================*/
#include "contactor_cfg.h"
#include "sps_cfg.h"

#include "sps_types.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Initialize IOs for the SPS driver.
 * @details This function sets the necessary register (e.g., configure pins as
 *          input or output).
 */
extern void SPS_Initialize(void);

/**
 * @brief   Control function for the CONT driver state machine.
 * @details This function contains the sequence of events in the SPS state
 *          machine. It must be called time-triggered, every 10ms. It exits
 *          without effect, if the function call is a reentrance.
 */
extern void SPS_Ctrl(void);

/**
 * @brief   Request state of a contactor
 * @details This function checks if the requested channel is affiliated to contactors and
 *          then passes the arguments on to #SPS_RequestChannelState().
 * @param[in]   channelIndex    number of the channel that should be accessed; This has to be
 *                              a SPS channel and it has to be affiliated with #SPS_AFF_CONTACTOR
 * @param[in]   channelFunction requested functional state of the channel
 */
extern void SPS_RequestContactorState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction);

/**
 * @brief   Request state of a general IO
 * @details This function checks if the requested channel is affiliated to general IO and
 *          then passes the arguments on to #SPS_RequestChannelState().
 * @param[in]   channelIndex    number of the channel that should be accessed; This has to be
 *                              a SPS channel and it has to be affiliated with #SPS_AFF_GENERAL_IO
 * @param[in]   channelFunction requested functional state of the channel
 */
extern void SPS_RequestGeneralIOState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction);

/**
 * @brief   Get feedback value
 * @details This function compares the measured current in the contactor
 *          channel with a fixed threshold and assumes the contactor feedback
 *          to be closed when this threshold is passed on the channel.
 * @param[in]   channelIndex    index of the channel (contactor) that should be accessed
 * @returns     state of the channel's feedback
 */
extern CONT_ELECTRICAL_STATE_TYPE_e SPS_GetChannelCurrentFeedback(const SPS_CHANNEL_INDEX channelIndex);

/**
 * @brief   Get the feedback state of a channel
 * @details Retrieves the feedback state of a sps channel by looking up the
 *          appropriate channel in #sps_kChannelFeedbackMapping and retrieving
 *          the value through the PEX API.
 * @param[in]   channelIndex    number of the SPS channel that shall be checked
 * @param[in]   normallyOpen    true if the feedback is normally open, false if not
 * @return  state of the channel as reported by PEX
 */
extern CONT_ELECTRICAL_STATE_TYPE_e SPS_GetChannelPexFeedback(const SPS_CHANNEL_INDEX channelIndex, bool normallyOpen);

/**
 * @brief   Returns the channel affiliation
 * @param[in]   channelIndex    index of the channel in question
 * @returns     affiliation of the channel
 */
extern SPS_CHANNEL_AFFILIATION_e SPS_GetChannelAffiliation(SPS_CHANNEL_INDEX channelIndex);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern void TEST_SPS_RequestChannelState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction);
extern SPS_STATE_e TEST_SPS_GetSpsState(void);
extern void TEST_SPS_SetSpsState(const SPS_STATE_e newState);
extern uint8_t TEST_SPS_GetSpsTimer(void);
extern void TEST_SPS_SetSpsTimer(const uint8_t newTimer);
#endif /* UNITY_UNIT_TEST */

#endif /* FOXBMS__SPS_H_ */
