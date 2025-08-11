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
 * @file    nxp_mc33775a_balancing.c
 * @author  foxBMS Team
 * @date    2025-02-03 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Balancing functions of the MC33775A analog front-end driver.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc3377x_balancing.h"
/* clang-format off */
#include "nxp_mc3377x-ll.h"
/* clang-format on */
#include "nxp_mc3377x_reg_def.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void N77x_BalanceControl(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    N77x_BalanceSetup(pState);

    DATA_READ_DATA(pState->n77xData.balancingControl);

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress   = m + 1u;
        uint16_t balancingState = 0u;
        for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
            if (pState->n77xData.balancingControl->activateBalancing[pState->currentString][m][cb] == true) {
                balancingState |= 1u << cb;
            }
        }
        /* All channels active --> 14 bits set to 1 --> 0x3FFF */
        FAS_ASSERT(balancingState <= 0x3FFFu);
        /* Enable channels, one written to a channels means balancing active */
        N77x_CommunicationWrite(deviceAddress, MC3377X_BAL_CH_CFG_OFFSET, balancingState, pState->pSpiTxSequence);
    }
}

extern void N77x_BalanceSetup(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Set global timeout counter to max value */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_BAL_GLOB_TO_TMR_OFFSET, N77X_GLOBAL_BALANCING_TIMER, pState->pSpiTxSequence);

    /* Disable pre-balancing timer by setting it to zero */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_BAL_PRE_TMR_OFFSET, N77X_PRE_BALANCING_TIMER, pState->pSpiTxSequence);

    /* Set PWM value for all channels to 100%, set balancing timer for all channels to maximum value */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_TMR_CH_ALL_OFFSET,
        (MC3377X_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL << MC3377X_BAL_TMR_CH_ALL_PWM_POS) |
            (N77X_ALL_CHANNEL_BALANCING_TIMER << MC3377X_BAL_TMR_CH_ALL_BALTIME_POS),
        pState->pSpiTxSequence);

    /* Select timer based balancing and enable balancing */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_BAL_GLOB_CFG_OFFSET,
        (MC3377X_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL << MC3377X_BAL_GLOB_CFG_BALEN_POS) |
            (MC3377X_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL << MC3377X_BAL_GLOB_CFG_TMRBALEN_POS),
        pState->pSpiTxSequence);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
