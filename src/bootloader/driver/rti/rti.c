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
 * @file    rti.c
 * @author  foxBMS Team
 * @date    2019-02-19 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  RTI
 *
 * @brief   Driver for the RTI module.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "rti.h"

#include "HL_reg_rti.h"
#include "HL_rti.h"
#include "HL_system.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* Set Pre-scalar for RTI clock */
#define RTI_CNT0_CPUC0_REG (0x00000009u)

/** Maximum wait time in microseconds in order to limit the time spent in wait
 *  to avoid livelock in wait */
#define RTI_WAIT_TIMEOUT_us (10000u)

/** Ratio between 1 MHz and 1 Hz */
#define RTI_RATIO_MHz_Hz (1000000.0f)

/** Mask for checking the overflow flag of RTI counter 0, only the OVL0INT can
 *  be read */
#define RTI_MASK_OVERFLOW_FLAG_COUNTER_0 (0x20000u)

/** The bitshift number of the overflow flag of RTI counter 0 */
#define RTI_BITSHIFT_OVERFLOW_FLAG_COUNTER_0 (17u)

/** The bit value if the overflow flag of RTI counter 0 is set */
#define RTI_SET_OVERFLOW_FLAG_COUNTER_0 (1u)

/** Register value to reset all overflow flags of the RTI counters */
#define RTI_RESET_OVERFLOW_FLAG_REGISTER_VALUE (0x0007000Fu)

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief   frequency of the FRC0 counter
 * @details refer to data sheet docref: SPNU563A-March 2018 p. 585 formula 23
 */
static const uint32_t rti_frcClock_Hz = (uint32_t)((RTI_FREQ)*RTI_RATIO_MHz_Hz) / ((RTI_CNT0_CPUC0_REG) + 1u);

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern uint32_t RTI_GetFreeRunningCount(void) {
    return RTI_CNT0_FRC0_REG;
}

extern uint32_t RTI_ResetFreeRunningCount(void) {
    uint32_t retVal   = 0u;
    RTI_CNT0_FRC0_REG = 0x00000000u;
#ifndef UNITY_UNIT_TEST
    if (RTI_CNT0_FRC0_REG == 0x00000000u)
#endif
    {
        retVal = 1u;
    }

    return retVal;
}

extern uint32_t RTI_ConvertFrcDifferenceToTimespan_us(uint32_t count) {
    /* AXIVION Routine Generic-MissingParameterAssert: count: parameter accepts whole range */
    const uint32_t rti_nrOfCounts_us = (uint32_t)(((float_t)rti_frcClock_Hz) / RTI_RATIO_MHz_Hz);
    return count / rti_nrOfCounts_us;
}

extern bool RTI_IsTimeElapsed(uint32_t startCounter, uint32_t timeout_us) {
    /* AXIVION Routine Generic-MissingParameterAssert: startCounter: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: timeout_us: parameter accepts whole range */
    uint32_t counter           = RTI_GetFreeRunningCount();
    uint32_t timeDifference_us = 0u;
    bool isTimeElapsed         = false;
    bool gotoNext              = true;

    uint8_t isOverflow = (RTI_INT_FLAG_REG & RTI_MASK_OVERFLOW_FLAG_COUNTER_0) >> RTI_BITSHIFT_OVERFLOW_FLAG_COUNTER_0;

    /* Check if it is an overflow situation */
    if (isOverflow == RTI_SET_OVERFLOW_FLAG_COUNTER_0) {
        /* Clear all pending interrupts -> clear overflow flags */
        RTI_INT_FLAG_REG = RTI_RESET_OVERFLOW_FLAG_REGISTER_VALUE;
        gotoNext         = false;
        isTimeElapsed    = true;
    }

    /* If no overflow happened -> check whether the specified time has passed */
    if (gotoNext == true) {
        timeDifference_us = RTI_ConvertFrcDifferenceToTimespan_us(counter - startCounter);
        if (timeDifference_us > timeout_us) {
            isTimeElapsed = true;
        } else {
            isTimeElapsed = false;
        }
    }

    return isTimeElapsed;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
