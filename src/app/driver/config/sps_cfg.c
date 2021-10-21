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
 * @file    sps_cfg.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2021-10-01 (date of last update)
 * @ingroup DRIVERS_CONF
 * @prefix  SPS
 *
 * @brief   Configuration for the driver for the smart power switches
 *
 */

/*========== Includes =======================================================*/
#include "sps_cfg.h"

#include "pex_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
/** channel states */
SPS_CHANNEL_STATE_s sps_channelStatus[SPS_NR_OF_AVAILABLE_SPS_CHANNELS] = {
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_CONTACTOR, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
    {SPS_CHANNEL_OFF, SPS_CHANNEL_OFF, 0.0f, SPS_AFF_GENERAL_IO, SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA},
};

/** mapping of channel states to feedback pins
 *
 * * IDs are the same as in #sps_channelStatus (#SPS_CHANNEL_INDEX)
 * * The mapping is intended to be constant; if a feedback channel shall be
 *   used, this should also be configured in #sps_channelStatus
*/
const SPS_CHANNEL_FEEDBACK_MAPPING_s sps_kChannelFeedbackMapping[SPS_NR_OF_AVAILABLE_SPS_CHANNELS] = {
    {PEX_PORT_EXPANDER1, PEX_PIN00},
    {PEX_PORT_EXPANDER1, PEX_PIN01},
    {PEX_PORT_EXPANDER1, PEX_PIN02},
    {PEX_PORT_EXPANDER1, PEX_PIN03},
    {PEX_PORT_EXPANDER1, PEX_PIN04},
    {PEX_PORT_EXPANDER1, PEX_PIN05},
    {PEX_PORT_EXPANDER1, PEX_PIN06},
    {PEX_PORT_EXPANDER1, PEX_PIN07},
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
