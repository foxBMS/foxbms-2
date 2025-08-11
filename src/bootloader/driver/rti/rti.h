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
 * @file    rti.h
 * @author  foxBMS Team
 * @date    2019-02-19 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  RTI
 *
 * @brief   Headers for the driver for the RTI module.
 * @details TODO
 *
 */

#ifndef FOXBMS__RTI_H_
#define FOXBMS__RTI_H_

/*========== Includes =======================================================*/
#include "HL_reg_rti.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** Address of Free Running Counter 0 (FRC0) */
#ifndef UNITY_UNIT_TEST
/** Free Running Counter 0 (FRC0) */
#define RTI_CNT0_FRC0_REG (rtiREG1->CNT[0].FRCx)
/** Register value that contains the overflow flag of the RTI counter 0 */
#define RTI_INT_FLAG_REG (rtiREG1->INTFLAG)
#else
extern uint32_t RTI_CNT0_FRC0_REG;
extern uint32_t RTI_INT_FLAG_REG;
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Reset the value of the Free Running Counter 0 (FRC0)
 * @returns 1 if the counter has been successfully reset
 */
extern uint32_t RTI_ResetFreeRunningCount(void);

/**
 * @brief   Return the current value of the Free Running Counter 0 (FRC0)
 * @returns current counter value of FRC0
 */
extern uint32_t RTI_GetFreeRunningCount(void);

/**
 * @brief   Convert the counter value FRC0 to a time in microseconds
 * @details This function applies a scaling factor to the FRC0 counter value,
 *          which can be retrieved with #RTI_GetFreeRunningCount() so that this
 *          value represents microseconds. Typically this used with counter
 *          differences in order to get timing estimates on microsecond-level.
 * @param   count counter value of the FRC0
 * @returns equivalent time in microseconds
 */
extern uint32_t RTI_ConvertFrcDifferenceToTimespan_us(uint32_t count);

/**
 * @brief   Check if a timeout in microseconds has elapsed
 * @details First the function #RTI_GetFreeRunningCount() must be called
 *          to retrieve the current value of the free running counter.
 *          This function checks if the time given as parameter has elapsed
 *          since the counter was retrieved.
 * @param   startCounter  counter value of the FRC0, used as starting point
 * @param   timeout_us    time in microseconds to check against
 * @returns true if timeout has elapsed, false otherwise
 */
extern bool RTI_IsTimeElapsed(uint32_t startCounter, uint32_t timeout_us);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__RTI_H_ */
