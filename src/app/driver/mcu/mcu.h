/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    mcu.h
 * @author  foxBMS Team
 * @date    2019-02-19 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  MCU
 *
 * @brief   Headers for the driver for the MCU module.
 *
 */

#ifndef FOXBMS__MCU_H_
#define FOXBMS__MCU_H_

/*========== Includes =======================================================*/
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * @brief   largest pin number that exists in TMS570LC4357
 * @details Checked in the data sheet spnu563a: The largest pin number that is
 *          used is 31.
*/
#define MCU_LARGEST_PIN_NUMBER (31u)

/**
 * @brief   maximum number of channels measured by the ADC1
 * @details Checked in the data sheet spnu563a: ADC1 supports 32 channels
 */
#define MCU_ADC1_MAX_NR_CHANNELS (32u)

#ifndef UNITY_UNIT_TEST
/** Address of Free Running Counter 0 (FRC0) */
#define MCU_RTI_CNT0_FRC0_REG (*((volatile uint32_t *)0xFFFFFC10u))
#else
extern volatile uint32_t MCU_RTI_CNT0_FRC0_REG;
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Wait blocking a certain time in microseconds.
 * @details This function uses the Free Running Counter 0 (FRC0) to generate a
 *          delay in microseconds. It does not modify anything related to the
 *          counter. This counter is already used by FreeRTOS.
 * @param   delay_us   time in microseconds to wait
 */
extern void MCU_Delay_us(uint32_t delay_us);

/**
 * @brief   Get the current value of the Free Running Counter 0 (FRC0)
 * @details This function returns the current value of the Free Running
 *          Counter 0 (FRC0). This counter is already used by FreeRTOS and this
 *          function does nothing more than accessing this value.
 * @returns the current counter value of FRC0
 */
extern uint32_t MCU_GetFreeRunningCount(void);

/**
 * @brief   Convert the counter value FRC0 to a time in microseconds
 * @details This function applies a scaling factor to the FRC0 counter value,
 *          which can be retrieved with #MCU_GetFreeRunningCount() so that this
 *          value represents microseconds. Typically this used with counter
 *          differences in order to get timing estimates on microsecond-level.
 * @param[in]   count   counter value of the FRC0
 * @returns     equivalent time in microseconds
 */
extern uint32_t MCU_ConvertFrcDifferenceToTimespan_us(uint32_t count);

/**
 * @brief   Checks if a timeout in microseconds has elapsed
 * @details First the function #MCU_GetFreeRunningCount() must be called
 *          to retrieve the current value of the free running counter.
 *          This function checks if the time given as parameter has elapsed
 *          since the counter was retrieved.
 * @param[in]   startCounter  counter value of the FRC0, used as starting point
 * @param[in]   timeout_us    time in microseconds to check against
 * @returns     true if timeout has elapsed, false otherwise
 */
extern bool MCU_IsTimeElapsed(uint32_t startCounter, uint32_t timeout_us);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__MCU_H_ */
