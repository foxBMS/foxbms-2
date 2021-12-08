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
 * @file    mcu.c
 * @author  foxBMS Team
 * @date    2019-02-19 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MCU
 *
 * @brief   Driver for the MCU module.
 *
 */

/*========== Includes =======================================================*/
#include "mcu.h"

#include "HL_system.h"

/*========== Macros and Definitions =========================================*/

/** This is defined in port.c for FreeRTOS.
 * It is named portRTI_CNT0_CPUC0_REG
 * This must be changed to reflect the FreeRTOS configuration
 */
#define MCU_RTI_CNT0_CPUC0_REG (0x00000001U)

/** threshold in order to limit the time spent in wait to avoid livelock in wait */
#define MCU_US_WAIT_TIMEOUT (10000U)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void MCU_delay_us(uint32_t delay_us) {
    uint32_t timeOut           = 0;
    uint32_t startValue        = 0;
    uint32_t rti_clock         = 0;
    uint32_t rti_nrOfCounts_us = 0;

    /**
     * This is the frequency of the FRC0 counter
     * data sheet SPNU563A-March 2018 p. 585 formula 23
     */
    rti_clock = (uint32_t)((AVCLK1_FREQ)*1000000.0f) / ((MCU_RTI_CNT0_CPUC0_REG) + 1u);

    /**
     * First the time of one FRC0 increment in microseconds is computed with
     * increment_time = (1/rti_clock)*1e6
     * The 1e6 factor is to get the period in microseconds.
     *
     * The number of increments needed to reach 1 microsecond is then
     * computed with
     * nr_increments_for_1us = (1us/increment_time)
     *
     * To avoid floating point computation problems, the number of
     * increments needed to reach 1 microsecond is computed at once with:
     */
    rti_nrOfCounts_us = (uint32_t)(((float)rti_clock) / 1e6f);

    /* Get current value of FRC0 counter */
    startValue = (uint32_t)MCU_RTI_CNT0_FRC0_REG;

    while (timeOut < MCU_US_WAIT_TIMEOUT) {
        /**
         * Poll FRC0 value until the value corresponding to the desired
         * waiting timing has been reached.
         * The timeout is to ensure the code does not get trapped here.
         */
        if (((uint32_t)MCU_RTI_CNT0_FRC0_REG - startValue) >= (delay_us * rti_nrOfCounts_us)) {
            break;
        }
        timeOut++;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
