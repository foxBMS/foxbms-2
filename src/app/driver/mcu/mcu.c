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
 * @file    mcu.c
 * @author  foxBMS Team
 * @date    2019-02-19 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  MCU
 *
 * @brief   Driver for the MCU module.
 *
 */

/*========== Includes =======================================================*/
#include "mcu.h"

#include "HL_system.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** This is defined in port.c for FreeRTOS.
 * It is named portRTI_CNT0_CPUC0_REG
 * This must be changed to reflect the FreeRTOS configuration
 */
#define MCU_RTI_CNT0_CPUC0_REG (0x00000001u)

/** threshold in order to limit the time spent in wait to avoid livelock in wait */
#define MCU_US_WAIT_TIMEOUT (10000u)

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief   frequency of the FRC0 counter
 * @details refer to data sheet SPNU563A-March 2018 p. 585 formula 23
 */
static const uint32_t mcu_frcClock_Hz = (uint32_t)((RTI_FREQ)*1000000.0f) / ((MCU_RTI_CNT0_CPUC0_REG) + 1u);

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void MCU_Delay_us(uint32_t delay_us) {
    /* AXIVION Routine Generic-MissingParameterAssert: delay_us: parameter accepts whole range */
    /**
     * First the time of one FRC0 increment in microseconds is computed with
     * increment_time = (1/mcu_frcClock_Hz)*1e6
     * The 1e6 factor is to get the period in microseconds.
     *
     * The number of increments needed to reach 1 microsecond is then
     * computed with
     * nr_increments_for_1us = (1us/increment_time)
     *
     * To avoid floating point computation problems, the number of
     * increments needed to reach 1 microsecond is computed at once with:
     */
    const uint32_t rti_nrOfCounts_us = (uint32_t)(((float_t)mcu_frcClock_Hz) / 1e6f);

    /* Get current value of FRC0 counter */
    const uint32_t startValue = MCU_RTI_CNT0_FRC0_REG;

    uint32_t timeOut = 0;
    while (timeOut < MCU_US_WAIT_TIMEOUT) {
        /**
         * Poll FRC0 value until the value corresponding to the desired
         * waiting timing has been reached.
         * The timeout is to ensure the code does not get trapped here.
         */
        const uint32_t checkValue = MCU_RTI_CNT0_FRC0_REG;
        if ((checkValue - startValue) >= (delay_us * rti_nrOfCounts_us)) {
            break;
        }
        timeOut++;
    }
}

extern uint32_t MCU_GetFreeRunningCount(void) {
    return MCU_RTI_CNT0_FRC0_REG;
}

extern uint32_t MCU_ConvertFrcDifferenceToTimespan_us(uint32_t count) {
    /* AXIVION Routine Generic-MissingParameterAssert: count: parameter accepts whole range */
    const uint32_t rti_nrOfCounts_us = (uint32_t)(((float_t)mcu_frcClock_Hz) / 1e6f);
    return count / rti_nrOfCounts_us;
}

extern bool MCU_IsTimeElapsed(uint32_t startCounter, uint32_t timeout_us) {
    bool retVal                = false;
    uint32_t counter           = MCU_GetFreeRunningCount();
    uint32_t timeDifference_us = MCU_ConvertFrcDifferenceToTimespan_us(counter - startCounter);
    if (timeDifference_us < timeout_us) {
        retVal = false;
    } else {
        retVal = true;
    }
    return retVal;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
